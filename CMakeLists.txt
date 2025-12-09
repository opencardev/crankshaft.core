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

# Add custom CMake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Download Material Design Icons font and generate mappings
# - Automatically downloads MDI font v7.4.47 (1.3 MB)
# - Auto-generates MaterialDesignIcons.js with 7447+ icon mappings (requires Python3)
# - To force regeneration: cmake -DFORCE_GENERATE_MDI_MAPPINGS=ON ..
include(DownloadMaterialDesignIcons)
download_material_design_icons()

# Find Qt6 packages
find_package(Qt6 REQUIRED COMPONENTS Core Network WebSockets Gui Qml Quick)

# Enable testing
enable_testing()

# Add subdirectories
add_subdirectory(core)
add_subdirectory(ui)
add_subdirectory(tests)

# CPack configuration for DEB packages
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_CONTACT "OpenCarDev Team <opencardevuk@gmail.com>")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "OpenCarDev Team <opencardevuk@gmail.com>")
set(CPACK_DEBIAN_PACKAGE_SECTION "misc")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/opencardev/crankshaft")
set(CPACK_DEBIAN_FILE_NAME "DEB-DEFAULT")

# Enable component-based packaging
set(CPACK_DEB_COMPONENT_INSTALL ON)
set(CPACK_COMPONENTS_ALL core ui)

# Core package configuration
set(CPACK_COMPONENT_CORE_DISPLAY_NAME "Crankshaft Core")
set(CPACK_COMPONENT_CORE_DESCRIPTION "Crankshaft core backend service with event bus and WebSocket server")
set(CPACK_DEBIAN_CORE_PACKAGE_NAME "crankshaft-core")
set(CPACK_DEBIAN_CORE_PACKAGE_DEPENDS "libqt6core6 (>= 6.2), libqt6network6 (>= 6.2), libqt6websockets6 (>= 6.2)")
set(CPACK_DEBIAN_CORE_PACKAGE_CONTROL_EXTRA "${CMAKE_SOURCE_DIR}/packaging/core/postinst;${CMAKE_SOURCE_DIR}/packaging/core/prerm")

# UI package configuration
set(CPACK_COMPONENT_UI_DISPLAY_NAME "Crankshaft UI")
set(CPACK_COMPONENT_UI_DESCRIPTION "Crankshaft QML-based user interface")
set(CPACK_COMPONENT_UI_DEPENDS core)
set(CPACK_DEBIAN_UI_PACKAGE_NAME "crankshaft-ui")
set(CPACK_DEBIAN_UI_PACKAGE_DEPENDS "crankshaft-core (>= ${PROJECT_VERSION}), libqt6core6 (>= 6.2), libqt6gui6 (>= 6.2), libqt6qml6 (>= 6.2), libqt6quick6 (>= 6.2), libqt6websockets6 (>= 6.2), qml6-module-qtquick, qml6-module-qtquick-controls, qml6-module-qtquick-layouts")
set(CPACK_DEBIAN_UI_PACKAGE_CONTROL_EXTRA "${CMAKE_SOURCE_DIR}/packaging/ui/postinst;${CMAKE_SOURCE_DIR}/packaging/ui/prerm")

include(CPack)
