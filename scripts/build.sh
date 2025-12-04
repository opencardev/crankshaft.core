#!/bin/bash
# Project: Crankshaft
# This file is part of Crankshaft project.
# Copyright (C) 2025 OpenCarDev Team
#
#  Crankshaft is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 3 of the License, or
#  (at your option) any later version.
#
#  Crankshaft is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with Crankshaft. If not, see <http://www.gnu.org/licenses/>.

set -e
set -u

# Default values
BUILD_TYPE="Debug"
COMPONENT="all"
BUILD_DIR="build"
CREATE_PACKAGE=false

# Usage information
usage() {
    cat << EOF
Usage: $0 [OPTIONS]

Named parameters:
  --build-type TYPE      Build configuration (Debug|Release) [default: Debug]
  --component COMP       Component to build (all|core|ui|tests) [default: all]
  --package              Create DEB packages after building [default: false]
  --help                 Display this help message

Examples:
  $0                                      # Build all components in Debug mode
  $0 --build-type Release                 # Build all components in Release mode
  $0 --component ui --build-type Debug    # Build only UI in Debug mode
  $0 --build-type Release --package       # Build all in Release mode and create packages
  $0 --component core --package           # Build only core in Debug mode and create package
EOF
    exit 1
}

# Parse named arguments
while [[ $# -gt 0 ]]; do
    case "$1" in
        --build-type)
            if [[ $# -lt 2 ]]; then
                echo "Error: --build-type requires a value"
                usage
            fi
            BUILD_TYPE="$2"
            shift 2
            ;;
        --component)
            if [[ $# -lt 2 ]]; then
                echo "Error: --component requires a value"
                usage
            fi
            COMPONENT="$2"
            shift 2
            ;;
        --package)
            CREATE_PACKAGE=true
            shift
            ;;
        --help)
            usage
            ;;
        *)
            echo "Error: Unknown option '$1'"
            usage
            ;;
    esac
done

# Validate build type
if [[ "$BUILD_TYPE" != "Debug" && "$BUILD_TYPE" != "Release" ]]; then
    echo "Error: Invalid build type '$BUILD_TYPE'. Must be 'Debug' or 'Release'."
    usage
fi

# Validate component
if [[ "$COMPONENT" != "all" && "$COMPONENT" != "core" && "$COMPONENT" != "ui" && "$COMPONENT" != "tests" ]]; then
    echo "Error: Invalid component '$COMPONENT'. Must be 'all', 'core', 'ui', or 'tests'."
    usage
fi

# Determine build target
case "$COMPONENT" in
    core)
        BUILD_TARGET="crankshaft-core"
        ;;
    ui)
        BUILD_TARGET="crankshaft-ui"
        ;;
    tests)
        BUILD_TARGET="crankshaft-tests"
        ;;
    all)
        BUILD_TARGET=""
        ;;
esac

echo "Building Crankshaft MVP: Component=${COMPONENT}, Build Type=${BUILD_TYPE}"

# Create build directory
mkdir -p "${BUILD_DIR}"

# Configure
echo "Configuring CMake..."
cmake -S . -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE="${BUILD_TYPE}"

# Build
echo "Building..."
if [ -z "$BUILD_TARGET" ]; then
    cmake --build "${BUILD_DIR}" --config "${BUILD_TYPE}" -j"$(nproc)"
else
    cmake --build "${BUILD_DIR}" --config "${BUILD_TYPE}" --target "${BUILD_TARGET}" -j"$(nproc)"
fi

echo ""
echo "Build complete!"
case "$COMPONENT" in
    core)
        echo "Executable: ${BUILD_DIR}/core/crankshaft-core"
        ;;
    ui)
        echo "Executable: ${BUILD_DIR}/ui/crankshaft-ui"
        ;;
    tests)
        echo "Tests: ${BUILD_DIR}/tests/test_eventbus"
        echo "Tests: ${BUILD_DIR}/tests/test_websocket"
        ;;
    all)
        echo "Executables:"
        echo "  Core:  ${BUILD_DIR}/core/crankshaft-core"
        echo "  UI:    ${BUILD_DIR}/ui/crankshaft-ui"
        echo "  Tests: ${BUILD_DIR}/tests/test_eventbus"
        echo "  Tests: ${BUILD_DIR}/tests/test_websocket"
        ;;
esac

# Create packages if requested
if [ "$CREATE_PACKAGE" = true ]; then
    echo ""
    echo "Creating DEB packages..."
    cd "${BUILD_DIR}"
    cpack -G DEB
    cd ..
    
    echo ""
    echo "Packages created in ${BUILD_DIR}:"
    ls -lh "${BUILD_DIR}"/*.deb 2>/dev/null || echo "No packages found"
fi
