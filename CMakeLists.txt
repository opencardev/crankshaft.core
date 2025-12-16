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

# Allow version override via CMAKE_PROJECT_VERSION cache variable
if(NOT DEFINED CMAKE_PROJECT_VERSION)
    set(CMAKE_PROJECT_VERSION "0.1.0")
endif()

# Extract numeric version for project() (CMake VERSION only supports major.minor.patch.tweak)
# Keep full version for package naming
set(FULL_VERSION "${CMAKE_PROJECT_VERSION}")
string(REGEX REPLACE "^([0-9]+\\.[0-9]+\\.[0-9]+).*" "\\1" NUMERIC_VERSION "${CMAKE_PROJECT_VERSION}")

project(crankshaft-mvp VERSION ${NUMERIC_VERSION} LANGUAGES CXX)

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

# Use full version string for package naming (includes git suffix if present)
set(CPACK_PACKAGE_VERSION "${FULL_VERSION}")

set(CPACK_PACKAGE_CONTACT "OpenCarDev Team <opencardevuk@gmail.com>")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "OpenCarDev Team <opencardevuk@gmail.com>")
set(CPACK_DEBIAN_COMPRESSION_TYPE "xz")

# Enable component-based packaging for separate core and ui DEBs
set(CPACK_DEB_COMPONENT_INSTALL ON)
set(CPACK_COMPONENTS_ALL runtime development core ui)

# AASDK Runtime package metadata
set(CPACK_COMPONENT_RUNTIME_DISPLAY_NAME "AASDK Runtime Libraries")
set(CPACK_COMPONENT_RUNTIME_DESCRIPTION "Android Auto SDK runtime libraries and SSL certificates")
set(CPACK_DEBIAN_RUNTIME_PACKAGE_NAME "libaasdk0")
set(CPACK_DEBIAN_RUNTIME_PACKAGE_SECTION "libs")
# libssl3 was renamed to libssl3t64 in Debian deb13 (time64 transition)
# Prefer t64 on deb13, but allow fallback to non-t64 to ease CI
if(CPACK_DEBIAN_PACKAGE_RELEASE STREQUAL "deb13")
	set(CPACK_DEBIAN_RUNTIME_PACKAGE_DEPENDS "libssl3t64, libusb-1.0-0")
else()
	set(CPACK_DEBIAN_RUNTIME_PACKAGE_DEPENDS "libssl3, libusb-1.0-0")
endif()
set(CPACK_DEBIAN_RUNTIME_FILE_NAME DEB-DEFAULT)

# AASDK Development package metadata
set(CPACK_COMPONENT_DEVELOPMENT_DISPLAY_NAME "AASDK Development Files")
set(CPACK_COMPONENT_DEVELOPMENT_DESCRIPTION "Android Auto SDK headers and development files")
set(CPACK_COMPONENT_DEVELOPMENT_DEPENDS runtime)
set(CPACK_DEBIAN_DEVELOPMENT_PACKAGE_NAME "libaasdk-dev")
set(CPACK_DEBIAN_DEVELOPMENT_PACKAGE_SECTION "libdevel")
set(CPACK_DEBIAN_DEVELOPMENT_PACKAGE_DEPENDS "libaasdk0 (>= ${NUMERIC_VERSION}), libssl-dev, libprotobuf-dev, libusb-1.0-0-dev")
set(CPACK_DEBIAN_DEVELOPMENT_FILE_NAME DEB-DEFAULT)

# Resolve Qt6 dependency names per suite (t64 only exists on deb13)
# Prefer t64 core/gui on deb13, allow non-t64 as alternative for robustness
if(CPACK_DEBIAN_PACKAGE_RELEASE STREQUAL "deb13")
	set(QT6_CORE_PKG "libqt6core6t64")
	set(QT6_GUI_PKG  "libqt6gui6")
else()
	set(QT6_CORE_PKG "libqt6core6")
	set(QT6_GUI_PKG  "libqt6gui6")
endif()

set(QT6_NETWORK_PKG "libqt6network6")
set(QT6_DBUS_PKG    "libqt6dbus6")
set(QT6_QML_PKG     "libqt6qml6")
set(QT6_QUICK_PKG   "libqt6quick6")
set(QT6_WS_PKG      "libqt6websockets6")

# QML runtime module packages (common across suites)
# Platform plugins are now built-in to libqt6gui6/libqt6core6
set(QML_RUNTIME_PKGS "qml6-module-qtquick, qml6-module-qtquick-controls, qml6-module-qtquick-layouts, qml6-module-qtquick-window")

# Core package metadata
set(CPACK_COMPONENT_CORE_DISPLAY_NAME "Crankshaft Core")
set(CPACK_COMPONENT_CORE_DESCRIPTION "Crankshaft core service with WebSocket server and event bus")
set(CPACK_COMPONENT_CORE_DEPENDS runtime)
set(CPACK_DEBIAN_CORE_PACKAGE_NAME "crankshaft-core")
set(CPACK_DEBIAN_CORE_PACKAGE_DEPENDS "libaasdk0 (>= ${NUMERIC_VERSION}), ${QT6_CORE_PKG}, ${QT6_NETWORK_PKG}, ${QT6_WS_PKG}, ${QT6_DBUS_PKG}, libgstreamer1.0-0, gstreamer1.0-plugins-good, gstreamer1.0-plugins-bad, gstreamer1.0-plugins-ugly, gstreamer1.0-libav, libdbus-1-3, libusb-1.0-0, gstreamer1.0-libav, gstreamer1.0-tools")
set(CPACK_DEBIAN_CORE_PACKAGE_CONTROL_EXTRA "${CMAKE_SOURCE_DIR}/packaging/core/postinst;${CMAKE_SOURCE_DIR}/packaging/core/prerm;${CMAKE_SOURCE_DIR}/packaging/core/crankshaft-core.service")
set(CPACK_DEBIAN_CORE_FILE_NAME DEB-DEFAULT)

# UI package metadata
set(CPACK_COMPONENT_UI_DISPLAY_NAME "Crankshaft UI")
set(CPACK_COMPONENT_UI_DESCRIPTION "Crankshaft QML-based user interface")
set(CPACK_COMPONENT_UI_DEPENDS core)
set(CPACK_DEBIAN_UI_PACKAGE_NAME "crankshaft-ui")
set(CPACK_DEBIAN_UI_PACKAGE_DEPENDS "crankshaft-core (>= ${NUMERIC_VERSION}), ${QT6_CORE_PKG}, ${QT6_GUI_PKG}, ${QT6_QML_PKG}, ${QT6_QUICK_PKG}, ${QT6_WS_PKG}, ${QML_RUNTIME_PKGS}")
set(CPACK_DEBIAN_UI_PACKAGE_CONTROL_EXTRA "${CMAKE_SOURCE_DIR}/packaging/ui/postinst;${CMAKE_SOURCE_DIR}/packaging/ui/prerm;${CMAKE_SOURCE_DIR}/packaging/ui/crankshaft-ui.service")
set(CPACK_DEBIAN_UI_FILE_NAME DEB-DEFAULT)

# Source package configuration (.tar.gz)
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "crankshaft-mvp-${FULL_VERSION}")
set(CPACK_SOURCE_IGNORE_FILES
    "/build/"
    "CMakeCache.txt"
    "CMakeFiles/"
    "_CPack_Packages/"
    "~$"
    "/[.]git/"
    "/[.]vscode/"
    "/[.]github/"
    "/[.]idea/"
    "[.]gitignore"
    "[.]DS_Store"
    "/test_build/"
    "/lib/"
    "/bin/"
    "[.]so$"
    "[.]a$"
    "[.]deb$"
    "[.]swp$"
    "[.]swo$"
)

include(CPack)
