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
import "Components"

Page {
    id: root
    
    background: Rectangle {
        color: Theme.background
    }
    
    header: Rectangle {
        width: parent.width
        height: 80
        color: Theme.surface
        
        RowLayout {
            anchors.fill: parent
            anchors.margins: Theme.spacingMd
            
            AppButton {
                text: qsTr("← Back")
                onClicked: stackView.pop()
            }
            
            Text {
                text: qsTr("Settings")
                font.pixelSize: Theme.fontSizeHeading2
                font.bold: true
                color: Theme.textPrimary
                Layout.fillWidth: true
            }
        }
    }
    
    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth
        
        ColumnLayout {
            width: parent.width
            spacing: Theme.spacingMd
            
            // Theme section
            Rectangle {
                Layout.fillWidth: true
                Layout.margins: Theme.spacingMd
                height: themeColumn.height + Theme.spacingLg * 2
                color: Theme.surface
                radius: Theme.radiusMd
                
                ColumnLayout {
                    id: themeColumn
                    anchors.centerIn: parent
                    width: parent.width - Theme.spacingLg * 2
                    spacing: Theme.spacingMd
                    
                    Text {
                        text: qsTr("Appearance")
                        font.pixelSize: Theme.fontSizeHeading3
                        font.bold: true
                        color: Theme.textPrimary
                    }
                    
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: Theme.spacingMd
                        
                        Text {
                            text: qsTr("Dark Mode")
                            font.pixelSize: Theme.fontSizeBody
                            color: Theme.textPrimary
                            Layout.fillWidth: true
                        }
                        
                        Switch {
                            checked: Theme.isDark
                            onToggled: {
                                Theme.toggleTheme()
                                wsClient.publish("ui/theme/changed", {
                                    "mode": Theme.isDark ? "dark" : "light"
                                })
                            }
                        }
                    }
                }
            }
            
            // Language section
            Rectangle {
                Layout.fillWidth: true
                Layout.margins: Theme.spacingMd
                height: langColumn.height + Theme.spacingLg * 2
                color: Theme.surface
                radius: Theme.radiusMd
                
                ColumnLayout {
                    id: langColumn
                    anchors.centerIn: parent
                    width: parent.width - Theme.spacingLg * 2
                    spacing: Theme.spacingMd
                    
                    Text {
                        text: qsTr("Language")
                        font.pixelSize: Theme.fontSizeHeading3
                        font.bold: true
                        color: Theme.textPrimary
                    }
                    
                    ComboBox {
                        Layout.fillWidth: true
                        model: ["English (GB)", "Deutsch (DE)"]
                        
                        onActivated: (index) => {
                            let lang = index === 0 ? "en-GB" : "de-DE"
                            wsClient.publish("ui/language/changed", {
                                "language": lang
                            })
                        }
                    }
                }
            }
            
            // Connection info
            Rectangle {
                Layout.fillWidth: true
                Layout.margins: Theme.spacingMd
                height: connColumn.height + Theme.spacingLg * 2
                color: Theme.surface
                radius: Theme.radiusMd
                
                ColumnLayout {
                    id: connColumn
                    anchors.centerIn: parent
                    width: parent.width - Theme.spacingLg * 2
                    spacing: Theme.spacingMd
                    
                    Text {
                        text: qsTr("Connection")
                        font.pixelSize: Theme.fontSizeHeading3
                        font.bold: true
                        color: Theme.textPrimary
                    }
                    
                    RowLayout {
                        Layout.fillWidth: true
                        
                        Text {
                            text: qsTr("Status:")
                            font.pixelSize: Theme.fontSizeBody
                            color: Theme.textSecondary
                        }
                        
                        Text {
                            text: wsClient.connected ? qsTr("Connected") : qsTr("Disconnected")
                            font.pixelSize: Theme.fontSizeBody
                            color: wsClient.connected ? Theme.success : Theme.error
                            font.bold: true
                        }
                    }
                }
            }
        }
    }
}
