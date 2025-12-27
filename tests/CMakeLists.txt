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

# Fetch Catch2
include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.11.0 #v2.13.10
)
FetchContent_MakeAvailable(Catch2)

find_package(Qt6 REQUIRED COMPONENTS Core Network WebSockets Bluetooth Test)

# Some tests pull in core implementation files that depend on system libraries
# (GStreamer, libusb). Ensure we can find those and link them for the test
# executables so the test binaries are self-contained.
find_package(PkgConfig REQUIRED)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)
pkg_check_modules(GSTREAMER_APP REQUIRED gstreamer-app-1.0)
pkg_check_modules(GSTREAMER_VIDEO REQUIRED gstreamer-video-1.0)
pkg_check_modules(GSTREAMER_AUDIO REQUIRED gstreamer-audio-1.0)
pkg_check_modules(LIBUSB REQUIRED libusb-1.0)
pkg_check_modules(DBUS REQUIRED dbus-1)

# Test for EventBus
add_executable(test_eventbus 
  test_eventbus.cpp
  ../core/services/eventbus/EventBus.cpp
)

set_target_properties(test_eventbus PROPERTIES
  AUTOMOC ON
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
)

target_include_directories(test_eventbus PRIVATE
  ${CMAKE_SOURCE_DIR}/core
)

target_link_libraries(test_eventbus PRIVATE
  Catch2::Catch2WithMain
  Qt6::Core
  Qt6::Test
)

add_test(NAME EventBusTest COMMAND test_eventbus)

# Test for WebSocketServer
add_executable(test_websocket
  test_websocket.cpp
  ../core/services/eventbus/EventBus.cpp
  ../core/services/logging/Logger.cpp
  ../core/services/websocket/WebSocketServer.cpp
  ../core/services/service_manager/ServiceManager.cpp
  ../core/services/profile/ProfileManager.cpp
  ../core/hal/multimedia/MediaPipeline.cpp
  ../core/services/android_auto/AndroidAutoService.cpp
  ../core/hal/multimedia/AudioHAL.cpp
  ../core/hal/multimedia/VideoHAL.cpp
  ../core/services/android_auto/MockAndroidAutoService.cpp
  ../core/services/android_auto/RealAndroidAutoService.cpp
  ../core/services/android_auto/ProtocolHelpers.cpp
  ../core/hal/multimedia/GStreamerVideoDecoder.cpp
  ../core/hal/multimedia/IVideoDecoder.cpp
  ../core/hal/multimedia/IAudioMixer.cpp
  ../core/hal/multimedia/AudioMixer.cpp
)

set_target_properties(test_websocket PROPERTIES
  AUTOMOC ON
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
)

target_include_directories(test_websocket PRIVATE
  ${CMAKE_SOURCE_DIR}/core
  ${GSTREAMER_INCLUDE_DIRS}
  ${GSTREAMER_APP_INCLUDE_DIRS}
  ${GSTREAMER_VIDEO_INCLUDE_DIRS}
  ${GSTREAMER_AUDIO_INCLUDE_DIRS}
  ${LIBUSB_INCLUDE_DIRS}
  # Fallback include path for aap_protobuf generated headers when aasdk is built
  ${CMAKE_BINARY_DIR}/aasdk/protobuf
  # aasdk public headers (source and generated build include)
  ${CMAKE_SOURCE_DIR}/external/aasdk/include
  ${CMAKE_BINARY_DIR}/aasdk/include
)

target_link_libraries(test_websocket PRIVATE
  Catch2::Catch2WithMain
  Qt6::Core
  Qt6::Network
  Qt6::WebSockets
  Qt6::Bluetooth
  Qt6::Gui
  Qt6::Test
  ${GSTREAMER_LIBRARIES}
  ${GSTREAMER_APP_LIBRARIES}
  ${GSTREAMER_VIDEO_LIBRARIES}
  ${GSTREAMER_AUDIO_LIBRARIES}
  ${DBUS_LIBRARIES}
  ${LIBUSB_LIBRARIES}
  aasdk
)

add_test(NAME WebSocketTest COMMAND test_websocket)

