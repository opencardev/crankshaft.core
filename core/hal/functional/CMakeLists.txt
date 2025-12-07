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

# Functional HAL implementations using GStreamer, DBus, etc.

# Find GStreamer
find_package(GStreamer 1.0 REQUIRED)
find_package(GStreamerApp 1.0 REQUIRED)
find_package(GStreamerBase 1.0 REQUIRED)
find_package(GStreamerVideo 1.0 REQUIRED)
find_package(GStreamerAudio 1.0 REQUIRED)

# Find DBus for network/Bluetooth management
find_package(DBus1 REQUIRED)

# Audio implementation with GStreamer and ALSA
add_library(audio-impl STATIC
    AudioManagerImpl.cpp
    AudioDeviceImpl.cpp
)

target_link_libraries(audio-impl
    Qt6::Core
    Qt6::Network
    GStreamer::Core
    GStreamer::Audio
    GStreamer::App
)

target_include_directories(audio-impl PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ../multimedia
)

# Video implementation with GStreamer
add_library(video-impl STATIC
    VideoManagerImpl.cpp
    VideoDeviceImpl.cpp
    MediaPipelineImpl.cpp
)

target_link_libraries(video-impl
    Qt6::Core
    Qt6::Gui
    Qt6::OpenGL
    GStreamer::Core
    GStreamer::Video
    GStreamer::App
)

target_include_directories(video-impl PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ../multimedia
)

# WiFi implementation with NetworkManager (DBus)
add_library(wifi-impl STATIC
    WiFiManagerImpl.cpp
)

target_link_libraries(wifi-impl
    Qt6::Core
    Qt6::DBus
    DBus1::DBus1
)

target_include_directories(wifi-impl PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ../wireless
)

# Bluetooth implementation with BlueZ (DBus)
add_library(bluetooth-impl STATIC
    BluetoothManagerImpl.cpp
)

target_link_libraries(bluetooth-impl
    Qt6::Core
    Qt6::DBus
    DBus1::DBus1
)

target_include_directories(bluetooth-impl PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ../wireless
)

# Network service implementation
add_library(network-service STATIC
    NetworkServiceImpl.cpp
)

target_link_libraries(network-service
    Qt6::Core
    Qt6::Network
)

target_include_directories(network-service PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ../wireless
)

# Expose all implementation libraries
set(CRANKSHAFT_HAL_FUNCTIONAL_LIBS
    audio-impl
    video-impl
    wifi-impl
    bluetooth-impl
    network-service
    PARENT_SCOPE
)
