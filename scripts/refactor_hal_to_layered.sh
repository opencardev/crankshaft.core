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

echo "=========================================="
echo "HAL Layered Architecture Refactoring"
echo "=========================================="
echo ""

# Create new folder structure
echo "Creating folder structure..."
mkdir -p core/hal/transport
mkdir -p core/hal/functional
mkdir -p core/hal/mocks/transport
mkdir -p core/hal/mocks/functional

echo "Folder structure created."
echo ""
echo "New structure:"
echo "  core/hal/"
echo "    ├── transport/       - Transport layer (UART, USB, SPI, I2C, etc.)"
echo "    ├── functional/      - Functional devices (GPS, CAN, Camera, etc.)"
echo "    ├── mocks/"
echo "    │   ├── transport/   - Mock transport implementations"
echo "    │   └── functional/  - Mock functional devices"
echo ""

echo "=========================================="
echo "Refactoring complete!"
echo "=========================================="
echo ""
echo "Next steps:"
echo "  1. Create base Transport class"
echo "  2. Create transport implementations (UART, USB, SPI, I2C, Bluetooth, etc.)"
echo "  3. Refactor functional devices to use transports"
echo "  4. Create mock transports"
echo "  5. Update mock functional devices to use mock transports"
echo "  6. Update CMakeLists.txt"
echo "  7. Update ProfileManager to support transport configuration"
echo ""
