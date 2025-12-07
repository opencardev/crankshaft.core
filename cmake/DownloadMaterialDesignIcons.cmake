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

# Download and set up Material Design Icons font
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
    
    # Set global variable for use in parent scope
    set(MDI_FONT_FILE "${MDI_FONT_FILE}" PARENT_SCOPE)
endfunction()
