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
    
    // Mock data - replace with actual backend integration
    ListModel {
        id: networksModel
        ListElement { ssid: "HomeNetwork"; strength: 80; secure: true; connected: false }
        ListElement { ssid: "CoffeeShop"; strength: 60; secure: false; connected: false }
        ListElement { ssid: "Airport"; strength: 45; secure: true; connected: false }
        ListElement { ssid: "Office_5G"; strength: 85; secure: true; connected: true }
    }
    
    property bool scanning: false
    
    background: Rectangle {
        color: Theme.background
    }
    
    // Header
    Rectangle {
        id: header
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 60
        color: Theme.surface
        z: 10
        
        RowLayout {
            anchors.fill: parent
            anchors.margins: Theme.spacingMd
            
            AppButton {
                text: "⬅ " + Strings.buttonBack
                onClicked: {
                    if (stack) {
                        stack.pop()
                    }
                }
            }
            
            Text {
                text: "WiFi Networks"
                font.pixelSize: Theme.fontSizeHeading2
                font.bold: true
                color: Theme.textPrimary
                Layout.fillWidth: true
            }
            
            AppButton {
                text: scanning ? "Scanning..." : "🔄 Scan"
                enabled: !scanning
                onClicked: {
                    scanning = true
                    wsClient.publish("wifi/scan/request", {})
                    // Simulate scan completion
                    scanTimer.start()
                }
            }
        }
    }
    
    Timer {
        id: scanTimer
        interval: 2000
        onTriggered: scanning = false
    }
    
    // Network list
    ScrollView {
        anchors.top: header.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: Theme.spacingMd
        contentWidth: availableWidth
        clip: true
        
        ColumnLayout {
            width: parent.width
            spacing: Theme.spacingSm
            
            // Info text
            Text {
                text: "Tap a network to connect"
                font.pixelSize: Theme.fontSizeCaption
                color: Theme.textSecondary
                Layout.fillWidth: true
                Layout.bottomMargin: Theme.spacingSm
            }
            
            // Network list
            Repeater {
                model: networksModel
                
                delegate: Rectangle {
                    Layout.fillWidth: true
                    height: 76
                    color: Theme.surface
                    radius: Theme.radiusSm
                    border.width: model.connected ? 2 : 0
                    border.color: Theme.primary
                    
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            if (!model.connected) {
                                // Request connection
                                wsClient.publish("wifi/connect", {
                                    "ssid": model.ssid,
                                    "secure": model.secure
                                })
                                
                                // Show feedback
                                statusText.text = "Connecting to " + model.ssid + "..."
                                statusText.color = Theme.primary
                            }
                        }
                    }
                    
                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: Theme.spacingMd
                        spacing: Theme.spacingMd
                        
                        // Signal strength icon
                        Text {
                            text: model.strength >= 75 ? "📶" : 
                                  model.strength >= 50 ? "📶" : 
                                  model.strength >= 25 ? "📶" : "📶"
                            font.pixelSize: 24
                            opacity: model.strength >= 75 ? 1.0 : 
                                    model.strength >= 50 ? 0.75 : 
                                    model.strength >= 25 ? 0.5 : 0.25
                        }
                        
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4
                            
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: Theme.spacingSm
                                
                                Text {
                                    text: model.ssid
                                    font.pixelSize: Theme.fontSizeBody
                                    font.bold: model.connected
                                    color: Theme.textPrimary
                                    Layout.fillWidth: true
                                }
                                
                                Text {
                                    text: model.secure ? "🔒" : ""
                                    font.pixelSize: 16
                                    visible: model.secure
                                }
                                
                                Text {
                                    text: "✓ Connected"
                                    font.pixelSize: Theme.fontSizeCaption
                                    font.bold: true
                                    color: Theme.primary
                                    visible: model.connected
                                }
                            }
                            
                            Text {
                                text: "Signal: " + model.strength + "%  •  " + 
                                      (model.secure ? "Secured" : "Open") + 
                                      (model.connected ? "" : "  •  Tap to connect")
                                font.pixelSize: Theme.fontSizeCaption
                                color: Theme.textSecondary
                                Layout.fillWidth: true
                            }
                        }
                    }
                }
            }
            
            Item { Layout.fillHeight: true }
        }
    }
    
    // Status bar at bottom
    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: statusText.text !== "" ? 40 : 0
        color: Theme.surface
        visible: height > 0
        
        Behavior on height {
            NumberAnimation { duration: 200 }
        }
        
        Text {
            id: statusText
            anchors.centerIn: parent
            font.pixelSize: Theme.fontSizeCaption
            color: Theme.textSecondary
        }
        
        Timer {
            id: statusTimer
            interval: 3000
            onTriggered: statusText.text = ""
        }
    }
    
    // WebSocket message handlers
    Connections {
        target: wsClient
        
        function onEventReceived(topic, message) {
            if (topic === "wifi/scan/result") {
                // Update network list from backend
                // message should contain array of networks
                console.log("WiFi scan result:", JSON.stringify(message))
            } else if (topic === "wifi/connect/result") {
                if (message.success) {
                    statusText.text = "Connected to " + message.ssid
                    statusText.color = Theme.primary
                } else {
                    statusText.text = "Failed to connect: " + (message.error || "Unknown error")
                    statusText.color = Theme.error
                }
                statusTimer.start()
            }
        }
    }
}
