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

# Read CPACK_DEBIAN_PACKAGE_RELEASE from environment variable if set
if(DEFINED ENV{CPACK_DEBIAN_RELEASE})
    set(CPACK_DEBIAN_PACKAGE_RELEASE "$ENV{CPACK_DEBIAN_RELEASE}")
else()
    set(CPACK_DEBIAN_PACKAGE_RELEASE "deb13")
endif()

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

# Detect architecture for packaging (similar to aasdk)
if(NOT TARGET_ARCH)
	if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
		set(TARGET_ARCH "amd64")
	elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
		set(TARGET_ARCH "arm64")
	elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "armv7l")
		set(TARGET_ARCH "armhf")
	elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "i386|i686")
		set(TARGET_ARCH "i386")
	else()
			set(TARGET_ARCH "unknown")
	endif()
	message(STATUS "Detected architecture: ${TARGET_ARCH}")
endif()
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "${TARGET_ARCH}")

# CPack generator configuration
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")

# Detect Debian release and set CPACK_DEBIAN_PACKAGE_RELEASE accordingly
if(NOT DEFINED CPACK_DEBIAN_PACKAGE_RELEASE OR CPACK_DEBIAN_PACKAGE_RELEASE STREQUAL "deb13")
	# Detect distribution if lsb_release available
	find_program(LSB_RELEASE_CMD lsb_release)
	if(LSB_RELEASE_CMD)
		execute_process(COMMAND ${LSB_RELEASE_CMD} -cs OUTPUT_VARIABLE DISTRO_CODENAME OUTPUT_STRIP_TRAILING_WHITESPACE)
		string(TOLOWER "${DISTRO_CODENAME}" DISTRO_CODENAME_LOWER)
		if(DISTRO_CODENAME_LOWER STREQUAL "jammy")
			set(CPACK_DEBIAN_PACKAGE_RELEASE "ubuntu22.04")
		elseif(DISTRO_CODENAME_LOWER STREQUAL "noble")
			set(CPACK_DEBIAN_PACKAGE_RELEASE "ubuntu24.04")
		elseif(DISTRO_CODENAME_LOWER STREQUAL "bookworm")
			set(CPACK_DEBIAN_PACKAGE_RELEASE "deb12")
		elseif(DISTRO_CODENAME_LOWER STREQUAL "trixie")
			set(CPACK_DEBIAN_PACKAGE_RELEASE "deb13")
		elseif(DISTRO_CODENAME_LOWER STREQUAL "bullseye")
			set(CPACK_DEBIAN_PACKAGE_RELEASE "deb11")
		else()
			set(CPACK_DEBIAN_PACKAGE_RELEASE "deb13")  # Default fallback
		endif()
	else()
		set(CPACK_DEBIAN_PACKAGE_RELEASE "deb13")  # Default fallback if lsb_release unavailable
	endif()
	message(STATUS "Detected Debian release: ${CPACK_DEBIAN_PACKAGE_RELEASE}")
endif()
set(CPACK_PACKAGE_CONTACT "OpenCarDev Team <opencardevuk@gmail.com>")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "OpenCarDev Team <opencardevuk@gmail.com>")
set(CPACK_DEBIAN_COMPRESSION_TYPE "xz")

# Enable component-based packaging for separate core and ui DEBs
set(CPACK_DEB_COMPONENT_INSTALL ON)
set(CPACK_COMPONENTS_ALL core ui)

# Core package metadata
set(CPACK_COMPONENT_CORE_DISPLAY_NAME "Crankshaft Core")
set(CPACK_COMPONENT_CORE_DESCRIPTION "Crankshaft core backend service with event bus and WebSocket server")
set(CPACK_DEBIAN_CORE_PACKAGE_NAME "crankshaft-core")
set(CPACK_DEBIAN_CORE_PACKAGE_CONTROL_EXTRA "${CMAKE_SOURCE_DIR}/packaging/core/postinst;${CMAKE_SOURCE_DIR}/packaging/core/prerm")
set(CPACK_DEBIAN_CORE_FILE_NAME DEB-DEFAULT)

# UI package metadata
set(CPACK_COMPONENT_UI_DISPLAY_NAME "Crankshaft UI")
set(CPACK_COMPONENT_UI_DESCRIPTION "Crankshaft QML-based user interface")
set(CPACK_COMPONENT_UI_DEPENDS core)
set(CPACK_DEBIAN_UI_PACKAGE_NAME "crankshaft-ui")
set(CPACK_DEBIAN_UI_PACKAGE_DEPENDS "crankshaft-core (>= ${PROJECT_VERSION}), libqt6core6 (>= 6.2), libqt6gui6 (>= 6.2), libqt6qml6 (>= 6.2), libqt6quick6 (>= 6.2), libqt6websockets6 (>= 6.2), qml6-module-qtquick, qml6-module-qtquick-controls, qml6-module-qtquick-layouts")
set(CPACK_DEBIAN_UI_PACKAGE_CONTROL_EXTRA "${CMAKE_SOURCE_DIR}/packaging/ui/postinst;${CMAKE_SOURCE_DIR}/packaging/ui/prerm")
set(CPACK_DEBIAN_UI_FILE_NAME DEB-DEFAULT)

# Source package configuration (.tar.gz)
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "crankshaft-mvp-${PROJECT_VERSION}")
set(CPACK_SOURCE_IGNORE_FILES
    "/build/"
    "/\\\\."
    "/.git/"
    "/.vscode/"
    "CMakeCache.txt"
    "CMakeFiles/"
    "_CPack_Packages/"
    "*.deb"
    "*.swp"
    "*.swo"
    "*~"
    ".DS_Store"
)

include(CPack)
