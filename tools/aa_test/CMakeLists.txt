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
project(aa_test VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find dependencies
find_package(Boost REQUIRED COMPONENTS system)
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)

# Find libusb and protobuf using pkg-config
pkg_check_modules(LIBUSB REQUIRED libusb-1.0)
pkg_check_modules(PROTOBUF REQUIRED protobuf)

# Find or build aasdk
set(AASDK_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/external/aasdk/include" CACHE PATH "AASDK include directory")
set(AASDK_LIB_DIR "${CMAKE_SOURCE_DIR}/build/core/aasdk/lib" CACHE PATH "AASDK library directory")

# Executable
add_executable(aa_test
    main.cpp
)

target_include_directories(aa_test PRIVATE
    ${AASDK_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
    ${LIBUSB_INCLUDE_DIRS}
    ${PROTOBUF_INCLUDE_DIRS}
)

target_link_libraries(aa_test PRIVATE
    ${AASDK_LIB_DIR}/libaasdk.so
    ${Boost_LIBRARIES}
    Threads::Threads
    ${LIBUSB_LIBRARIES}
    ${PROTOBUF_LIBRARIES}
)

# Install
install(TARGETS aa_test
    RUNTIME DESTINATION bin
)
