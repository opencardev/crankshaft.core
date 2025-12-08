/*
 * Project: Crankshaft
 * This file is part of Crankshaft project.
 * Copyright (C) 2025 OpenCarDev Team
 *
 *  Crankshaft is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Crankshaft is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Crankshaft. If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * EXAMPLE: Material Design Icons Usage
 * 
 * This file demonstrates how to use Material Design Icons throughout the application.
 * Browse all available icons at: https://pictogrammers.com/library/mdi/
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Crankshaft 1.0

Item {
    width: 1024
    height: 600
    
    ScrollView {
        anchors.fill: parent
        
        ColumnLayout {
            width: parent.width
            spacing: Theme.spacing * 2
            
            // Category: Navigation
            GroupBox {
                title: "Navigation Icons"
                Layout.fillWidth: true
                
                RowLayout {
                    Icon { name: "mdi-menu"; size: 32 }
                    Icon { name: "mdi-home"; size: 32 }
                    Icon { name: "mdi-arrow-left"; size: 32 }
                    Icon { name: "mdi-arrow-right"; size: 32 }
                    Icon { name: "mdi-arrow-up"; size: 32 }
                    Icon { name: "mdi-arrow-down"; size: 32 }
                    Icon { name: "mdi-chevron-left"; size: 32 }
                    Icon { name: "mdi-chevron-right"; size: 32 }
                    Icon { name: "mdi-navigation"; size: 32 }
                    Icon { name: "mdi-map-marker"; size: 32 }
                }
            }
            
            // Category: Media Controls
            GroupBox {
                title: "Media Controls"
                Layout.fillWidth: true
                
                RowLayout {
                    Icon { name: "mdi-play"; size: 32 }
                    Icon { name: "mdi-pause"; size: 32 }
                    Icon { name: "mdi-stop"; size: 32 }
                    Icon { name: "mdi-skip-next"; size: 32 }
                    Icon { name: "mdi-skip-previous"; size: 32 }
                    Icon { name: "mdi-fast-forward"; size: 32 }
                    Icon { name: "mdi-rewind"; size: 32 }
                    Icon { name: "mdi-volume-high"; size: 32 }
                    Icon { name: "mdi-volume-low"; size: 32 }
                    Icon { name: "mdi-volume-mute"; size: 32 }
                }
            }
            
            // Category: Connectivity
            GroupBox {
                title: "Connectivity & Network"
                Layout.fillWidth: true
                
                RowLayout {
                    Icon { name: "mdi-wifi"; size: 32 }
                    Icon { name: "mdi-wifi-off"; size: 32 }
                    Icon { name: "mdi-bluetooth"; size: 32 }
                    Icon { name: "mdi-bluetooth-connect"; size: 32 }
                    Icon { name: "mdi-usb"; size: 32 }
                    Icon { name: "mdi-ethernet"; size: 32 }
                    Icon { name: "mdi-network"; size: 32 }
                    Icon { name: "mdi-cellphone"; size: 32 }
                    Icon { name: "mdi-signal"; size: 32 }
                    Icon { name: "mdi-access-point"; size: 32 }
                }
            }
            
            // Category: Automotive
            GroupBox {
                title: "Automotive"
                Layout.fillWidth: true
                
                RowLayout {
                    Icon { name: "mdi-car"; size: 32 }
                    Icon { name: "mdi-car-electric"; size: 32 }
                    Icon { name: "mdi-car-connected"; size: 32 }
                    Icon { name: "mdi-car-battery"; size: 32 }
                    Icon { name: "mdi-car-door"; size: 32 }
                    Icon { name: "mdi-speedometer"; size: 32 }
                    Icon { name: "mdi-gas-station"; size: 32 }
                    Icon { name: "mdi-oil"; size: 32 }
                    Icon { name: "mdi-tire"; size: 32 }
                    Icon { name: "mdi-car-cruise-control"; size: 32 }
                }
            }
            
            // Category: Communication
            GroupBox {
                title: "Communication"
                Layout.fillWidth: true
                
                RowLayout {
                    Icon { name: "mdi-phone"; size: 32 }
                    Icon { name: "mdi-phone-incoming"; size: 32 }
                    Icon { name: "mdi-phone-outgoing"; size: 32 }
                    Icon { name: "mdi-phone-missed"; size: 32 }
                    Icon { name: "mdi-message"; size: 32 }
                    Icon { name: "mdi-email"; size: 32 }
                    Icon { name: "mdi-forum"; size: 32 }
                    Icon { name: "mdi-chat"; size: 32 }
                    Icon { name: "mdi-microphone"; size: 32 }
                    Icon { name: "mdi-microphone-off"; size: 32 }
                }
            }
            
            // Category: System & Settings
            GroupBox {
                title: "System & Settings"
                Layout.fillWidth: true
                
                RowLayout {
                    Icon { name: "mdi-cog"; size: 32 }
                    Icon { name: "mdi-cog-outline"; size: 32 }
                    Icon { name: "mdi-wrench"; size: 32 }
                    Icon { name: "mdi-tune"; size: 32 }
                    Icon { name: "mdi-information"; size: 32 }
                    Icon { name: "mdi-help-circle"; size: 32 }
                    Icon { name: "mdi-alert"; size: 32 }
                    Icon { name: "mdi-alert-circle"; size: 32 }
                    Icon { name: "mdi-check"; size: 32 }
                    Icon { name: "mdi-close"; size: 32 }
                }
            }
            
            // Category: User & Account
            GroupBox {
                title: "User & Account"
                Layout.fillWidth: true
                
                RowLayout {
                    Icon { name: "mdi-account"; size: 32 }
                    Icon { name: "mdi-account-circle"; size: 32 }
                    Icon { name: "mdi-account-cog"; size: 32 }
                    Icon { name: "mdi-account-multiple"; size: 32 }
                    Icon { name: "mdi-account-plus"; size: 32 }
                    Icon { name: "mdi-account-remove"; size: 32 }
                    Icon { name: "mdi-login"; size: 32 }
                    Icon { name: "mdi-logout"; size: 32 }
                    Icon { name: "mdi-lock"; size: 32 }
                    Icon { name: "mdi-lock-open"; size: 32 }
                }
            }
            
            // Category: Weather
            GroupBox {
                title: "Weather"
                Layout.fillWidth: true
                
                RowLayout {
                    Icon { name: "mdi-weather-sunny"; size: 32 }
                    Icon { name: "mdi-weather-cloudy"; size: 32 }
                    Icon { name: "mdi-weather-rainy"; size: 32 }
                    Icon { name: "mdi-weather-snowy"; size: 32 }
                    Icon { name: "mdi-weather-lightning"; size: 32 }
                    Icon { name: "mdi-weather-night"; size: 32 }
                    Icon { name: "mdi-weather-fog"; size: 32 }
                    Icon { name: "mdi-weather-windy"; size: 32 }
                    Icon { name: "mdi-thermometer"; size: 32 }
                    Icon { name: "mdi-water"; size: 32 }
                }
            }
            
            // Category: File & Folder
            GroupBox {
                title: "File & Folder"
                Layout.fillWidth: true
                
                RowLayout {
                    Icon { name: "mdi-folder"; size: 32 }
                    Icon { name: "mdi-folder-open"; size: 32 }
                    Icon { name: "mdi-file"; size: 32 }
                    Icon { name: "mdi-file-document"; size: 32 }
                    Icon { name: "mdi-file-music"; size: 32 }
                    Icon { name: "mdi-file-video"; size: 32 }
                    Icon { name: "mdi-file-image"; size: 32 }
                    Icon { name: "mdi-download"; size: 32 }
                    Icon { name: "mdi-upload"; size: 32 }
                    Icon { name: "mdi-cloud"; size: 32 }
                }
            }
            
            // Size Variations
            GroupBox {
                title: "Size Variations"
                Layout.fillWidth: true
                
                RowLayout {
                    spacing: Theme.spacing * 2
                    
                    Icon { name: "mdi-star"; size: 16; color: Theme.accent }
                    Icon { name: "mdi-star"; size: 24; color: Theme.accent }
                    Icon { name: "mdi-star"; size: 32; color: Theme.accent }
                    Icon { name: "mdi-star"; size: 48; color: Theme.accent }
                    Icon { name: "mdi-star"; size: 64; color: Theme.accent }
                }
            }
            
            // Color Variations
            GroupBox {
                title: "Color Variations"
                Layout.fillWidth: true
                
                RowLayout {
                    spacing: Theme.spacing * 2
                    
                    Icon { name: "mdi-heart"; size: 32; color: "#FF0000" }
                    Icon { name: "mdi-heart"; size: 32; color: "#FF5722" }
                    Icon { name: "mdi-heart"; size: 32; color: "#FFC107" }
                    Icon { name: "mdi-heart"; size: 32; color: "#4CAF50" }
                    Icon { name: "mdi-heart"; size: 32; color: "#2196F3" }
                    Icon { name: "mdi-heart"; size: 32; color: "#9C27B0" }
                    Icon { name: "mdi-heart"; size: 32; color: Theme.textPrimary }
                }
            }
        }
    }
}
