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

# Download and set up Material Design Icons font and generate mappings
function(download_material_design_icons)
    set(MDI_VERSION "7.4.47")
    # Try multiple sources for the font file
    set(MDI_FONT_URLs
        "https://cdn.jsdelivr.net/npm/@mdi/font@${MDI_VERSION}/fonts/materialdesignicons-webfont.ttf"
        "https://github.com/Templarian/MaterialDesign/releases/download/v${MDI_VERSION}/MaterialDesignIcons-Webfont.ttf"
        "https://raw.githubusercontent.com/Templarian/MaterialDesign-Font/master/fonts/materialdesignicons-webfont.ttf"
    )
    set(MDI_FONT_DIR "${CMAKE_BINARY_DIR}/ui/fonts")
    set(MDI_FONT_FILE "${MDI_FONT_DIR}/materialdesignicons-webfont.ttf")
    set(MDI_MAPPINGS_FILE "${CMAKE_SOURCE_DIR}/ui/qml/components/MaterialDesignIcons.js")
    
    # Create fonts directory
    file(MAKE_DIRECTORY "${MDI_FONT_DIR}")
    
    # Download font if not already present or if file is empty
    if(NOT EXISTS "${MDI_FONT_FILE}" OR (EXISTS "${MDI_FONT_FILE}" AND NOT FILE_SIZE_GREATER_THAN_ZERO))
        message(STATUS "Downloading Material Design Icons font v${MDI_VERSION}...")
        
        set(DOWNLOAD_SUCCESS FALSE)
        foreach(URL ${MDI_FONT_URLs})
            if(NOT DOWNLOAD_SUCCESS)
                message(STATUS "Trying: ${URL}")
                file(DOWNLOAD
                    "${URL}"
                    "${MDI_FONT_FILE}"
                    SHOW_PROGRESS
                    TIMEOUT 30
                    STATUS DOWNLOAD_STATUS
                )
                
                list(GET DOWNLOAD_STATUS 0 DOWNLOAD_CODE)
                list(GET DOWNLOAD_STATUS 1 DOWNLOAD_MESSAGE)
                
                if(DOWNLOAD_CODE EQUAL 0)
                    file(SIZE "${MDI_FONT_FILE}" FILE_SIZE)
                    if(FILE_SIZE GREATER 100000)
                        message(STATUS "Material Design Icons font downloaded successfully (${FILE_SIZE} bytes)")
                        set(DOWNLOAD_SUCCESS TRUE)
                    else()
                        message(STATUS "Downloaded file too small (${FILE_SIZE} bytes), trying next mirror...")
                        file(REMOVE "${MDI_FONT_FILE}")
                    endif()
                else()
                    message(STATUS "Download failed: ${DOWNLOAD_MESSAGE}, trying next mirror...")
                    file(REMOVE "${MDI_FONT_FILE}")
                endif()
            endif()
        endforeach()
        
        if(NOT DOWNLOAD_SUCCESS)
            message(WARNING "Failed to download Material Design Icons font from all sources. Icons may not display correctly.")
        endif()
    else()
        file(SIZE "${MDI_FONT_FILE}" FILE_SIZE)
        if(FILE_SIZE GREATER 0)
            message(STATUS "Material Design Icons font already present (${FILE_SIZE} bytes)")
        endif()
    endif()
    
    # Generate icon mappings if Python is available
    find_package(Python3 COMPONENTS Interpreter)
    if(Python3_FOUND)
        set(GENERATE_SCRIPT "${CMAKE_SOURCE_DIR}/scripts/generate_mdi_mappings.py")
        
        # Check if mappings file needs to be generated or updated
        set(SHOULD_GENERATE FALSE)
        
        if(NOT EXISTS "${MDI_MAPPINGS_FILE}")
            message(STATUS "MaterialDesignIcons.js not found, will generate")
            set(SHOULD_GENERATE TRUE)
        else()
            # Check if FORCE_GENERATE_MDI_MAPPINGS is set
            if(FORCE_GENERATE_MDI_MAPPINGS)
                message(STATUS "FORCE_GENERATE_MDI_MAPPINGS is set, will regenerate")
                set(SHOULD_GENERATE TRUE)
            endif()
        endif()
        
        if(SHOULD_GENERATE)
            message(STATUS "Generating Material Design Icons mappings...")
            execute_process(
                COMMAND ${Python3_EXECUTABLE} "${GENERATE_SCRIPT}"
                WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
                RESULT_VARIABLE GENERATE_RESULT
                OUTPUT_VARIABLE GENERATE_OUTPUT
                ERROR_VARIABLE GENERATE_ERROR
            )
            
            if(GENERATE_RESULT EQUAL 0)
                message(STATUS "Material Design Icons mappings generated successfully")
                if(GENERATE_OUTPUT)
                    message(STATUS "${GENERATE_OUTPUT}")
                endif()
            else()
                message(WARNING "Failed to generate Material Design Icons mappings: ${GENERATE_ERROR}")
            endif()
        else()
            message(STATUS "Material Design Icons mappings already up to date")
        endif()
    else()
        message(STATUS "Python3 not found, skipping icon mappings generation")
        if(NOT EXISTS "${MDI_MAPPINGS_FILE}")
            message(WARNING "MaterialDesignIcons.js not found and Python3 not available. Icons may not work correctly.")
        endif()
    endif()
    
    # Set global variable for use in parent scope
    set(MDI_FONT_FILE "${MDI_FONT_FILE}" PARENT_SCOPE)
endfunction()
