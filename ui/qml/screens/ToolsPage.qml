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

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"
import Crankshaft 1.0

Page {
    id: root
    
    property var stack: null
    
    background: Rectangle {
        color: Theme.background
    }
    
    header: Rectangle {
        width: parent.width
        height: 80
        color: Theme.surface
        
        RowLayout {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 16
            
            AppButton {
                text: "←"
                implicitWidth: 76
                implicitHeight: 76
                onClicked: {
                    if (stack) {
                        stack.pop()
                    }
                }
            }
            
            Text {
                text: Strings.cardToolsTitle
                font.pixelSize: 32
                font.bold: true
                color: Theme.textPrimary
                Layout.fillWidth: true
            }
        }
    }
    
    // 4x4 Grid of tool cards
    GridView {
        id: toolsGrid
        anchors.fill: parent
        anchors.margins: 16
        cellWidth: width / 4
        cellHeight: height / 4
        clip: true
        
        model: ListModel {
            ListElement {
                toolTitle: "Device Profiles"
                toolDescription: "Manage device configurations"
                toolIcon: "mdi-account-cog"
                toolPage: "profiles"
            }
            ListElement {
                toolTitle: "Diagnostics"
                toolDescription: "System diagnostics"
                toolIcon: "mdi-stethoscope"
                toolPage: ""
            }
            ListElement {
                toolTitle: "Logs"
                toolDescription: "View system logs"
                toolIcon: "mdi-text-box"
                toolPage: ""
            }
            ListElement {
                toolTitle: "Network"
                toolDescription: "Network settings"
                toolIcon: "mdi-network"
                toolPage: ""
            }
            ListElement {
                toolTitle: "Display"
                toolDescription: "Display calibration"
                toolIcon: "mdi-monitor"
                toolPage: ""
            }
            ListElement {
                toolTitle: "Audio"
                toolDescription: "Audio settings"
                toolIcon: "mdi-volume-high"
                toolPage: ""
            }
            ListElement {
                toolTitle: "USB"
                toolDescription: "USB device info"
                toolIcon: "mdi-usb"
                toolPage: ""
            }
            ListElement {
                toolTitle: "Bluetooth"
                toolDescription: "Bluetooth devices"
                toolIcon: "mdi-bluetooth"
                toolPage: ""
            }
            ListElement {
                toolTitle: "Performance"
                toolDescription: "System performance"
                toolIcon: "mdi-speedometer"
                toolPage: ""
            }
            ListElement {
                toolTitle: "Storage"
                toolDescription: "Storage management"
                toolIcon: "mdi-harddisk"
                toolPage: ""
            }
            ListElement {
                toolTitle: "Updates"
                toolDescription: "System updates"
                toolIcon: "mdi-update"
                toolPage: ""
            }
            ListElement {
                toolTitle: "About"
                toolDescription: "System information"
                toolIcon: "mdi-information"
                toolPage: ""
            }
        }
        
        delegate: Item {
            width: toolsGrid.cellWidth
            height: toolsGrid.cellHeight
            
            Card {
                anchors.fill: parent
                anchors.margins: 6
                title: model.toolTitle
                description: model.toolDescription
                icon: model.toolIcon
                
                onClicked: {
                    if (model.toolPage === "profiles") {
                        if (stack) {
                            stack.push(Qt.resolvedUrl("ProfilesPage.qml"), { stack: stack })
                        }
                    } else {
                        console.log("Tool clicked:", model.toolTitle)
                        // TODO: Navigate to respective tool pages when implemented
                    }
                }
            }
        }
    }
}
