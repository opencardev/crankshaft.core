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

# Usage information
usage() {
    echo "Usage: $0 [BUILD_TYPE] [COMPONENT]"
    echo ""
    echo "Arguments:"
    echo "  BUILD_TYPE   Build configuration (Debug|Release) [default: Debug]"
    echo "  COMPONENT    Component to build (all|core|ui|tests) [default: all]"
    echo ""
    echo "Examples:"
    echo "  $0                    # Build all components in Debug mode"
    echo "  $0 Release            # Build all components in Release mode"
    echo "  $0 Debug ui           # Build only UI in Debug mode"
    echo "  $0 Release core       # Build only core in Release mode"
    exit 1
}

# Parse arguments
BUILD_TYPE=${1:-Debug}
COMPONENT=${2:-all}
BUILD_DIR="build"

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
mkdir -p ${BUILD_DIR}

# Configure
echo "Configuring CMake..."
cmake -S . -B ${BUILD_DIR} -DCMAKE_BUILD_TYPE=${BUILD_TYPE}

# Build
echo "Building..."
if [ -z "$BUILD_TARGET" ]; then
    cmake --build ${BUILD_DIR} --config ${BUILD_TYPE} -j$(nproc)
else
    cmake --build ${BUILD_DIR} --config ${BUILD_TYPE} --target ${BUILD_TARGET} -j$(nproc)
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
        echo "Tests: ${BUILD_DIR}/tests/crankshaft-tests"
        ;;
    all)
        echo "Executables:"
        echo "  Core:  ${BUILD_DIR}/core/crankshaft-core"
        echo "  UI:    ${BUILD_DIR}/ui/crankshaft-ui"
        echo "  Tests: ${BUILD_DIR}/tests/crankshaft-tests"
        ;;
esac
