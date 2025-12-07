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

# Android Auto service with AASDK

# Add AASDK as external project
if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/aasdk/CMakeLists.txt")
    add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/aasdk aasdk-build)
endif()

# Android Auto service library
add_library(android-auto-service STATIC
    AndroidAutoService.cpp
)

set_target_properties(android-auto-service PROPERTIES
    AUTOMOC ON
)

target_link_libraries(android-auto-service
    Qt6::Core
    Qt6::Network
)

# Link AASDK if available
if(TARGET aasdk)
    target_link_libraries(android-auto-service
        aasdk
    )
    target_compile_definitions(android-auto-service PRIVATE
        AASDK_AVAILABLE=1
    )
endif()

# Try to find aasdk via pkg-config as fallback
find_package(PkgConfig)
if(PkgConfig_FOUND)
    pkg_check_modules(AASDK IMPORTED_TARGET libaasdk)
    if(AASDK_FOUND)
        target_link_libraries(android-auto-service PRIVATE
            PkgConfig::AASDK
        )
        target_compile_definitions(android-auto-service PRIVATE
            AASDK_AVAILABLE=1
        )
    endif()
endif()

target_include_directories(android-auto-service PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Expose the service library
set(CRANKSHAFT_ANDROID_AUTO_SERVICE
    android-auto-service
    PARENT_SCOPE
)
