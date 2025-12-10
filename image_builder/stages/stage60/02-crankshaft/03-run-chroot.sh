#!/bin/bash -e
#  * Project: OpenAuto
#  * This file is part of openauto project.
#  * Copyright (C) 2025 OpenCarDev Team
#  *
#  *  openauto is free software: you can redistribute it and/or modify
#  *  it under the terms of the GNU General Public License as published by
#  *  the Free Software Foundation; either version 3 of the License, or
#  *  (at your option) any later version.
#  *
#  *  openauto is distributed in the hope that it will be useful,
#  *  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  *  GNU General Public License for more details.
#  *
#  *  You should have received a copy of the GNU General Public License
#  *  along with openauto. If not, see <http://www.gnu.org/licenses/>.

# Generate OpenCarDev metadata inside the image

mkdir -p /etc/opencardev

# Get installed versions (or n/a)
get_ver() {
  local pkg="$1"
  dpkg-query -W -f='${Version}' "$pkg" 2>/dev/null || echo "n/a"
}

LIBAASDK_VER=$(get_ver libaasdk)
OPENAUTO_VER=$(get_ver openauto)

# Write versions file
{
  echo "libaasdk=${LIBAASDK_VER}"
  echo "openauto=${OPENAUTO_VER}"
} > /etc/opencardev/versions.txt

# Also place a copy in the boot partition for end-users
mkdir -p /boot/opencardev
cp /etc/opencardev/versions.txt /boot/opencardev/versions.txt || true

# Emit a unique marker to logs so CI can parse if needed
echo "OPENCARDEV_VERSIONS libaasdk=${LIBAASDK_VER} openauto=${OPENAUTO_VER}"

# Produce a simple SBOM: installed packages list (package, version, architecture)
# This is a lightweight inventory suitable as a BOM attachment.
dpkg-query -W -f='${Package}\t${Version}\t${Architecture}\n' \
  | sort > /etc/opencardev/sbom-packages.txt

# Also note the timestamp
date -u +'%Y-%m-%dT%H:%M:%SZ' > /etc/opencardev/sbom-generated-at.txt
