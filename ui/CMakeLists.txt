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

project(crankshaft-ui)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick WebSockets LinguistTools)

# Set Qt policies to suppress warnings
qt_policy(SET QTP0001 NEW)  # Use :/qt/qml/ prefix
qt_policy(SET QTP0004 NEW)  # Require qmldir files

# Create executable first
qt_add_executable(crankshaft-ui
  main.cpp
  Theme.h
  WebSocketClient.h
  WebSocketClient.cpp
)

# Enable AUTOMOC for metatype generation
set_target_properties(crankshaft-ui PROPERTIES
  AUTOMOC ON
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/ui
)

qt_add_qml_module(crankshaft-ui
  URI Crankshaft
  VERSION 1.0
  NO_PLUGIN_OPTIONAL
  NO_IMPORT_SCAN
  QML_FILES
    qml/Main.qml
    qml/HomeScreen.qml
    qml/SettingsScreen.qml
    qml/Components/AppButton.qml
    qml/Components/Card.qml
    qml/Components/Icon.qml
  RESOURCES
    assets/icons/mdi.svg
)

target_include_directories(crankshaft-ui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(crankshaft-ui PRIVATE
  Qt6::Core
  Qt6::Gui
  Qt6::Qml
  Qt6::Quick
  Qt6::WebSockets
)
