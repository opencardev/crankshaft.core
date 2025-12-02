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

find_package(Qt6 REQUIRED COMPONENTS Core Network WebSockets Test)

# Test for EventBus
add_executable(test_eventbus
  test_eventbus.cpp
  ../core/EventBus.cpp
)

set_target_properties(test_eventbus PROPERTIES
  AUTOMOC ON
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
)

target_include_directories(test_eventbus PRIVATE
  ${CMAKE_SOURCE_DIR}/core
)

target_link_libraries(test_eventbus PRIVATE
  Catch2::Catch2
  Qt6::Core
  Qt6::Test
)

add_test(NAME EventBusTest COMMAND test_eventbus)

# Test for WebSocketServer
add_executable(test_websocket
  test_websocket.cpp
  ../core/EventBus.cpp
  ../core/Logger.cpp
  ../core/WebSocketServer.cpp
)

set_target_properties(test_websocket PROPERTIES
  AUTOMOC ON
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
)

target_include_directories(test_websocket PRIVATE
  ${CMAKE_SOURCE_DIR}/core
)

target_link_libraries(test_websocket PRIVATE
  Catch2::Catch2
  Qt6::Core
  Qt6::Network
  Qt6::WebSockets
  Qt6::Test
)

add_test(NAME WebSocketTest COMMAND test_websocket)
