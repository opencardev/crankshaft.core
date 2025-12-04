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
project(crankshaft-mvp VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable QML language server configuration
set(QT_QML_GENERATE_QMLLS_INI ON CACHE BOOL "Generate .qmllint.ini for QML language server" FORCE)

# Find Qt6 packages
find_package(Qt6 REQUIRED COMPONENTS Core Network WebSockets Gui Qml Quick)

# Enable testing
enable_testing()

# Add subdirectories
add_subdirectory(core)
add_subdirectory(ui)
add_subdirectory(tests)

# CPack configuration for DEB package
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "crankshaft-mvp")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Crankshaft Automotive Infotainment System")
set(CPACK_PACKAGE_CONTACT "OpenCarDev Team")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "OpenCarDev Team")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt6core6, libqt6network6, libqt6websockets6, libqt6qml6, libqt6quick6")

include(CPack)
