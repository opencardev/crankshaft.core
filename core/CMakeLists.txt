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

project(crankshaft-core)

find_package(Qt6 REQUIRED COMPONENTS Core Network WebSockets Gui)

# Find GStreamer for multimedia
find_package(PkgConfig REQUIRED)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)
pkg_check_modules(GSTREAMER_APP REQUIRED gstreamer-app-1.0)
pkg_check_modules(GSTREAMER_VIDEO REQUIRED gstreamer-video-1.0)
pkg_check_modules(GSTREAMER_AUDIO REQUIRED gstreamer-audio-1.0)

# Find DBus for wireless/bluetooth
pkg_check_modules(DBUS REQUIRED dbus-1)

# Find libusb for Android Auto
pkg_check_modules(LIBUSB REQUIRED libusb-1.0)

# Find OpenSSL for AASDK
find_package(OpenSSL REQUIRED)

# Find Protobuf for AASDK
find_package(Protobuf REQUIRED)

# Add AASDK submodule
if(EXISTS ${CMAKE_SOURCE_DIR}/external/aasdk/CMakeLists.txt)
  add_subdirectory(${CMAKE_SOURCE_DIR}/external/aasdk aasdk)
else()
  message(WARNING "AASDK submodule not found. Run: git submodule update --init --recursive")
endif()

set(SOURCES
  main.cpp
  
  # Core Services
  services/eventbus/EventBus.cpp
  services/websocket/WebSocketServer.cpp
  services/config/ConfigService.cpp
  services/logging/Logger.cpp
  services/profile/ProfileManager.cpp
  services/android_auto/AndroidAutoService.cpp
  
  # Transport Layer
  hal/transport/Transport.cpp
  hal/transport/UARTTransport.cpp
  
  # Functional Device Layer
  hal/functional/FunctionalDevice.cpp
  hal/functional/GPSDevice.cpp
  hal/functional/CANDevice.cpp
  
  # Mock Transports (temporarily disabled - needs header files)
  # hal/mocks/transport/MockTransport.cpp
  
  # Multimedia HAL (temporarily disabled - needs header files)
  # hal/multimedia/AudioHAL.cpp
  # hal/multimedia/VideoHAL.cpp
  # hal/multimedia/MediaPipeline.cpp
  
  # Wireless HAL (temporarily disabled - needs header files)
  # hal/wireless/WiFiHAL.cpp
  # hal/wireless/BluetoothHAL.cpp
  # hal/wireless/NetworkService.cpp
)

# Headers are automatically discovered by AUTOMOC
add_executable(crankshaft-core ${SOURCES})

target_include_directories(crankshaft-core PRIVATE 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${GSTREAMER_INCLUDE_DIRS}
  ${GSTREAMER_APP_INCLUDE_DIRS}
  ${GSTREAMER_VIDEO_INCLUDE_DIRS}
  ${GSTREAMER_AUDIO_INCLUDE_DIRS}
  ${DBUS_INCLUDE_DIRS}
  ${LIBUSB_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/external/aasdk/include
)

set_target_properties(crankshaft-core PROPERTIES
  AUTOMOC ON
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/core
)

target_link_libraries(crankshaft-core PRIVATE
  Qt6::Core
  Qt6::Network
  Qt6::WebSockets
  Qt6::Gui
  ${GSTREAMER_LIBRARIES}
  ${GSTREAMER_APP_LIBRARIES}
  ${GSTREAMER_VIDEO_LIBRARIES}
  ${GSTREAMER_AUDIO_LIBRARIES}
  ${DBUS_LIBRARIES}
  ${LIBUSB_LIBRARIES}
  OpenSSL::SSL
  OpenSSL::Crypto
  protobuf::libprotobuf
  aasdk
)

# Install core executable
install(TARGETS crankshaft-core
  RUNTIME DESTINATION bin
  COMPONENT core
)

# Install core configuration
install(FILES ${CMAKE_SOURCE_DIR}/config/crankshaft.json
  DESTINATION /etc/crankshaft
  COMPONENT core
)
