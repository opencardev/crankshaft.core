#!/bin/bash -eu
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

echo "=== DEBUG BUILD-DOCKER.SH START ==="
echo "Working directory: ${DIR}"
echo "Script arguments: $*"

BUILD_OPTS="$*"

DOCKER="docker"

echo "=== Testing Docker connectivity ==="
if ! ${DOCKER} ps >/dev/null 2>&1; then
	echo "Docker not accessible without sudo, trying with sudo..."
	DOCKER="sudo docker"
fi
if ! ${DOCKER} ps >/dev/null; then
	echo "error connecting to docker:"
	${DOCKER} ps
	exit 1
fi
echo "Docker command: ${DOCKER}"
${DOCKER} --version

echo "=== Host System Information ==="
echo "Architecture: $(uname -m)"
echo "Kernel: $(uname -r)"
echo "OS: $(cat /etc/os-release | grep PRETTY_NAME)"

echo "=== Testing binfmt_misc on host ==="
echo "binfmt_misc module status:"
lsmod | grep binfmt_misc || echo "binfmt_misc module not loaded"

echo "binfmt_misc filesystem:"
mount | grep binfmt_misc || echo "binfmt_misc not mounted"

echo "binfmt_misc directory:"
ls -la /proc/sys/fs/binfmt_misc/ 2>/dev/null | head -10 || echo "binfmt_misc not accessible"

echo "QEMU static binaries:"
ls -la /usr/bin/qemu-*-static 2>/dev/null || echo "No QEMU static binaries found"

echo "Testing ARM emulation on host:"
/usr/bin/qemu-aarch64-static --version 2>/dev/null || echo "qemu-aarch64-static failed"

echo "Testing ARM Docker container on host:"
${DOCKER} run --rm --platform linux/arm64 busybox:latest echo "ARM64 test successful" || echo "ARM64 Docker test failed"

CONFIG_FILE=""
if [ -f "${DIR}/config" ]; then
	CONFIG_FILE="${DIR}/config"
fi

while getopts "c:" flag
do
	case "${flag}" in
		c)
			CONFIG_FILE="${OPTARG}"
			;;
		*)
			;;
	esac
done

# Ensure that the configuration file is an absolute path
if test -x /usr/bin/realpath; then
	CONFIG_FILE=$(realpath -s "$CONFIG_FILE")
fi

echo "=== Configuration File Processing ==="
echo "Config file path: ${CONFIG_FILE}"

# Ensure that the confguration file is present
if test -z "${CONFIG_FILE}"; then
	echo "Configuration file need to be present in '${DIR}/config' or path passed as parameter"
	exit 1
else
	echo "Loading configuration from: ${CONFIG_FILE}"
	echo "Config file contents:"
	cat "${CONFIG_FILE}"
	echo "--- End of config ---"
	# shellcheck disable=SC1090
	source "${CONFIG_FILE}"
fi

CONTAINER_NAME=${CONTAINER_NAME:-pigen_work}
CONTINUE=${CONTINUE:-0}
PRESERVE_CONTAINER=${PRESERVE_CONTAINER:-0}

echo "=== Container Settings ==="
echo "Container name: ${CONTAINER_NAME}"
echo "Continue mode: ${CONTINUE}"
echo "Preserve container: ${PRESERVE_CONTAINER}"

if [ -z "${IMG_NAME}" ]; then
	echo "IMG_NAME not set in 'config'" 1>&2
	echo 1>&2
exit 1
fi

echo "Image name: ${IMG_NAME}"

# Ensure the Git Hash is recorded before entering the docker container
GIT_HASH=${GIT_HASH:-"$(git rev-parse HEAD)"}
GIT_BRANCH=${GIT_BRANCH:-"$(git branch  --no-color  | grep -E '^\*')"}

echo "Git hash: ${GIT_HASH}"
echo "Git branch: ${GIT_BRANCH}"

echo "=== Container Status Check ==="
CONTAINER_EXISTS=$(${DOCKER} ps -a --filter name="${CONTAINER_NAME}" -q)
CONTAINER_RUNNING=$(${DOCKER} ps --filter name="${CONTAINER_NAME}" -q)

echo "Existing container ID: ${CONTAINER_EXISTS:-none}"
echo "Running container ID: ${CONTAINER_RUNNING:-none}"

if [ "${CONTAINER_RUNNING}" != "" ]; then
	echo "The build is already running in container ${CONTAINER_NAME}. Aborting."
	exit 1
fi
if [ "${CONTAINER_EXISTS}" != "" ] && [ "${CONTINUE}" != "1" ]; then
	echo "Container ${CONTAINER_NAME} already exists and you did not specify CONTINUE=1. Aborting."
	echo "You can delete the existing container like this:"
	echo "  ${DOCKER} rm -v ${CONTAINER_NAME}"
	exit 1
fi

# Modify original build-options to allow config file to be mounted in the docker container
BUILD_OPTS="$(echo "${BUILD_OPTS:-}" | sed -E 's@\-c\s?([^ ]+)@-c /config@')"
echo "Modified build options: ${BUILD_OPTS}"

echo "=== Building Docker Image ==="
echo "Building pi-gen image from: ${DIR}"
${DOCKER} build -t pi-gen "${DIR}"

echo "=== Starting Container Build ==="
if [ "${CONTAINER_EXISTS}" != "" ]; then
	echo "Using existing container approach..."
	trap 'echo "got CTRL+C... please wait 5s" && ${DOCKER} stop -t 5 ${CONTAINER_NAME}_cont' SIGINT SIGTERM
	
	echo "Running container with enhanced binfmt testing..."
	time ${DOCKER} run --rm --privileged \
		--volume "${CONFIG_FILE}":/config:ro \
		--mount type=bind,source=/proc/sys/fs/binfmt_misc,target=/host-binfmt \
		-e "GIT_HASH=${GIT_HASH}" \
		-e "GIT_BRANCH=${GIT_BRANCH}" \
		--volumes-from="${CONTAINER_NAME}" --name "${CONTAINER_NAME}_cont" \
		pi-gen \
		bash -e -o pipefail -c "
		echo '=== INSIDE CONTAINER DEBUG START ==='
		echo 'Container architecture: \$(uname -m)'
		echo 'Container OS: \$(cat /etc/os-release | grep PRETTY_NAME)'
		
		echo '=== Testing binfmt_misc inside container ==='
		echo 'binfmt_misc module status:'
		lsmod | grep binfmt_misc || echo 'binfmt_misc module not loaded in container'
		
		echo 'Host binfmt_misc (mounted at /host-binfmt):'
		ls -la /host-binfmt/ 2>/dev/null | head -10 || echo 'host-binfmt not accessible'
		
		echo 'Attempting to bind mount host binfmt to container location:'
		mkdir -p /proc/sys/fs/binfmt_misc
		mount --bind /host-binfmt /proc/sys/fs/binfmt_misc || echo 'Bind mount failed'
		
		echo 'binfmt_misc directory in container after bind mount:'
		ls -la /proc/sys/fs/binfmt_misc/ 2>/dev/null | head -10 || echo 'binfmt_misc still not accessible in container'
		
		echo 'QEMU binaries in container:'
		ls -la /usr/bin/qemu-*-static 2>/dev/null | head -5 || echo 'No QEMU static binaries in container'
		
		echo 'Testing qemu-aarch64-static in container:'
		/usr/bin/qemu-aarch64-static --version 2>/dev/null || echo 'qemu-aarch64-static failed in container'
		
		echo '=== Running dpkg-reconfigure qemu-user-static ==='
		dpkg-reconfigure qemu-user-static
		
		echo '=== Post-reconfigure binfmt test ==='
		ls -la /proc/sys/fs/binfmt_misc/ 2>/dev/null | head -10 || echo 'binfmt_misc still not accessible after reconfigure'
		
		echo '=== Starting actual build ==='
		cd /pi-gen
		./build.sh ${BUILD_OPTS}
		
		echo '=== Copying build logs ==='
		rsync -av work/*/build.log deploy/
		" &
	wait "$!"
else
	echo "Using new container approach..."
	trap 'echo "got CTRL+C... please wait 5s" && ${DOCKER} stop -t 5 ${CONTAINER_NAME}' SIGINT SIGTERM
	
	echo "Running new container with enhanced binfmt testing..."
	time ${DOCKER} run --name "${CONTAINER_NAME}" --privileged \
		--volume "${CONFIG_FILE}":/config:ro \
		--mount type=bind,source=/proc/sys/fs/binfmt_misc,target=/host-binfmt \
		-e "GIT_HASH=${GIT_HASH}" \
		-e "GIT_BRANCH=${GIT_BRANCH}" \
		pi-gen \
		bash -e -o pipefail -c "
		echo '=== INSIDE CONTAINER DEBUG START ==='
		echo 'Container architecture: \$(uname -m)'
		echo 'Container OS: \$(cat /etc/os-release | grep PRETTY_NAME)'
		
		echo '=== Testing binfmt_misc inside container ==='
		echo 'binfmt_misc module status:'
		lsmod | grep binfmt_misc || echo 'binfmt_misc module not loaded in container'
		
		echo 'Host binfmt_misc (mounted at /host-binfmt):'
		ls -la /host-binfmt/ 2>/dev/null | head -10 || echo 'host-binfmt not accessible'
		
		echo 'Attempting to bind mount host binfmt to container location:'
		mkdir -p /proc/sys/fs/binfmt_misc
		mount --bind /host-binfmt /proc/sys/fs/binfmt_misc || echo 'Bind mount failed'
		
		echo 'binfmt_misc directory in container after bind mount:'
		ls -la /proc/sys/fs/binfmt_misc/ 2>/dev/null | head -10 || echo 'binfmt_misc still not accessible in container'
		
		echo 'QEMU binaries in container:'
		ls -la /usr/bin/qemu-*-static 2>/dev/null | head -5 || echo 'No QEMU static binaries in container'
		
		echo 'Testing qemu-aarch64-static in container:'
		/usr/bin/qemu-aarch64-static --version 2>/dev/null || echo 'qemu-aarch64-static failed in container'
		
		echo '=== Running dpkg-reconfigure qemu-user-static ==='
		dpkg-reconfigure qemu-user-static
		
		echo '=== Post-reconfigure binfmt test ==='
		ls -la /proc/sys/fs/binfmt_misc/ 2>/dev/null | head -10 || echo 'binfmt_misc still not accessible after reconfigure'
		
		echo '=== Starting actual build ==='
		cd /pi-gen
		./build.sh ${BUILD_OPTS}
		
		echo '=== Copying build logs ==='
		rsync -av work/*/build.log deploy/
		" &
	wait "$!"
fi

echo "=== Copying Results ==="
echo "copying results from deploy/"
${DOCKER} cp "${CONTAINER_NAME}":/pi-gen/deploy .
ls -lah deploy

echo "=== Cleanup ==="
# cleanup
if [ "${PRESERVE_CONTAINER}" != "1" ]; then
	echo "Removing container ${CONTAINER_NAME}..."
	${DOCKER} rm -v "${CONTAINER_NAME}"
else
	echo "Preserving container ${CONTAINER_NAME}"
fi

echo "=== BUILD COMPLETED ==="
echo "Done! Your image(s) should be in deploy/"