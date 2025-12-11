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

cmake_minimum_required(VERSION 3.16)

# Multimedia HAL module
add_library(crankshaft-hal-multimedia INTERFACE)

target_include_directories(crankshaft-hal-multimedia INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/multimedia
)

# Wireless HAL module
add_library(crankshaft-hal-wireless INTERFACE)

target_include_directories(crankshaft-hal-wireless INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/wireless
)

# Functional HAL implementations (mocks/GStreamer/DBus)
add_subdirectory(functional)

# Transport HAL module
add_subdirectory(transport)

# Mocks for testing
add_subdirectory(mocks)
