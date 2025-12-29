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

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"

echo "=== Building AA Test Tool ==="
echo "Script dir: ${SCRIPT_DIR}"
echo "Build dir: ${BUILD_DIR}"
echo "Repo root: ${REPO_ROOT}"

# Ensure AASDK is built first
if [ ! -f "${REPO_ROOT}/build/core/aasdk/lib/libaasdk.so" ]; then
    echo "ERROR: AASDK library not found at ${REPO_ROOT}/build/core/aasdk/lib/libaasdk.so"
    echo "Please build the main project first:"
    echo "  cd ${REPO_ROOT}"
    echo "  cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug"
    echo "  cmake --build build"
    exit 1
fi

# Configure
echo "Configuring..."
cmake -S "${SCRIPT_DIR}" -B "${BUILD_DIR}" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DAASDK_INCLUDE_DIR="${REPO_ROOT}/external/aasdk/include" \
    -DAASDK_LIB_DIR="${REPO_ROOT}/build/core/aasdk/lib"

# Build
echo "Building..."
cmake --build "${BUILD_DIR}" -j

echo ""
echo "=== Build Complete ==="
echo "Executable: ${BUILD_DIR}/aa_test"
echo ""
echo "Run with:"
echo "  ${BUILD_DIR}/aa_test"
echo "  AASDK_VERBOSE_USB=1 ${BUILD_DIR}/aa_test"
echo "  sudo AASDK_VERBOSE_USB=1 ${BUILD_DIR}/aa_test"
