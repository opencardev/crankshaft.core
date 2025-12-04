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

find_package(Qt6 REQUIRED COMPONENTS Core Network WebSockets)

set(SOURCES
  main.cpp
  EventBus.cpp
  WebSocketServer.cpp
  ConfigService.cpp
  Logger.cpp
)

set(HEADERS
  EventBus.h
  WebSocketServer.h
  ConfigService.h
  Logger.h
)

add_executable(crankshaft-core ${SOURCES} ${HEADERS})

target_include_directories(crankshaft-core PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(crankshaft-core PROPERTIES
  AUTOMOC ON
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/core
)

target_link_libraries(crankshaft-core PRIVATE
  Qt6::Core
  Qt6::Network
  Qt6::WebSockets
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
