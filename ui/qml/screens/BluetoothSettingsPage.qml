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
        id: devicesModel
        ListElement { deviceId: "car_phone"; name: "Car Phone"; connected: true; paired: true; type: "phone" }
        ListElement { deviceId: "headphones"; name: "Headphones"; connected: false; paired: true; type: "audio" }
        ListElement { deviceId: "smartwatch"; name: "Smartwatch"; connected: false; paired: true; type: "watch" }
        ListElement { deviceId: "speaker_nearby"; name: "Nearby Speaker"; connected: false; paired: false; type: "audio" }
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
                text: "Bluetooth Devices"
                font.pixelSize: Theme.fontSizeHeading2
                font.bold: true
                color: Theme.textPrimary
                Layout.fillWidth: true
            }
            
            AppButton {
                text: scanning ? "Scanning..." : "🔍 Scan"
                enabled: !scanning
                onClicked: {
                    scanning = true
                    wsClient.publish("bluetooth/scan/request", {})
                    // Simulate scan completion
                    scanTimer.start()
                }
            }
        }
    }
    
    Timer {
        id: scanTimer
        interval: 3000
        onTriggered: scanning = false
    }
    
    // Device list
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
            spacing: Theme.spacingMd
            
            // Info text
            Text {
                text: "Tap a device to pair or connect"
                font.pixelSize: Theme.fontSizeCaption
                color: Theme.textSecondary
                Layout.fillWidth: true
                Layout.bottomMargin: Theme.spacingSm
            }
            
            // Paired devices section
            Text {
                text: "Paired Devices"
                font.pixelSize: Theme.fontSizeBody
                font.bold: true
                color: Theme.textPrimary
                Layout.fillWidth: true
                visible: hasPairedDevices()
            }
            
            Repeater {
                model: devicesModel
                
                delegate: Rectangle {
                    Layout.fillWidth: true
                    height: 76
                    color: Theme.surface
                    radius: Theme.radiusSm
                    border.width: model.connected ? 2 : 0
                    border.color: Theme.primary
                    visible: model.paired
                    
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            if (model.paired && !model.connected) {
                                // Connect to paired device
                                wsClient.publish("bluetooth/connect", {
                                    "id": model.deviceId,
                                    "name": model.name
                                })
                                statusText.text = "Connecting to " + model.name + "..."
                                statusText.color = Theme.primary
                            } else if (model.connected) {
                                // Disconnect
                                wsClient.publish("bluetooth/disconnect", {
                                    "id": model.deviceId
                                })
                                statusText.text = "Disconnecting from " + model.name + "..."
                                statusText.color = Theme.textSecondary
                            }
                        }
                    }
                    
                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: Theme.spacingMd
                        spacing: Theme.spacingMd
                        
                        // Device type icon
                        Text {
                            text: model.type === "phone" ? "📱" : 
                                  model.type === "audio" ? "🎧" : 
                                  model.type === "watch" ? "⌚" : "📡"
                            font.pixelSize: 32
                        }
                        
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4
                            
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: Theme.spacingSm
                                
                                Text {
                                    text: model.name
                                    font.pixelSize: Theme.fontSizeBody
                                    font.bold: model.connected
                                    color: Theme.textPrimary
                                    Layout.fillWidth: true
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
                                text: model.connected ? "Tap to disconnect" : 
                                      model.paired ? "Tap to connect" : 
                                      "Tap to pair"
                                font.pixelSize: Theme.fontSizeCaption
                                color: Theme.textSecondary
                                Layout.fillWidth: true
                            }
                        }
                    }
                }
            }
            
            // Available devices section
            Text {
                text: "Available Devices"
                font.pixelSize: Theme.fontSizeBody
                font.bold: true
                color: Theme.textPrimary
                Layout.fillWidth: true
                Layout.topMargin: Theme.spacingMd
                visible: hasUnpairedDevices()
            }
            
            Repeater {
                model: devicesModel
                
                delegate: Rectangle {
                    Layout.fillWidth: true
                    height: 76
                    color: Theme.surface
                    radius: Theme.radiusSm
                    visible: !model.paired
                    
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            // Pair new device
                            wsClient.publish("bluetooth/pair", {
                                "id": model.deviceId,
                                "name": model.name
                            })
                            statusText.text = "Pairing with " + model.name + "..."
                            statusText.color = Theme.primary
                        }
                    }
                    
                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: Theme.spacingMd
                        spacing: Theme.spacingMd
                        
                        // Device type icon
                        Text {
                            text: model.type === "phone" ? "📱" : 
                                  model.type === "audio" ? "🎧" : 
                                  model.type === "watch" ? "⌚" : "📡"
                            font.pixelSize: 32
                            opacity: 0.6
                        }
                        
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4
                            
                            Text {
                                text: model.name
                                font.pixelSize: Theme.fontSizeBody
                                color: Theme.textPrimary
                                Layout.fillWidth: true
                            }
                            
                            Text {
                                text: "Not paired  •  Tap to pair"
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
    
    // Helper functions
    function hasPairedDevices() {
        for (var i = 0; i < devicesModel.count; i++) {
            if (devicesModel.get(i).paired) return true
        }
        return false
    }
    
    function hasUnpairedDevices() {
        for (var i = 0; i < devicesModel.count; i++) {
            if (!devicesModel.get(i).paired) return true
        }
        return false
    }
    
    // WebSocket message handlers
    Connections {
        target: wsClient
        
        function onEventReceived(topic, message) {
            if (topic === "bluetooth/scan/result") {
                // Update device list from backend
                console.log("Bluetooth scan result:", JSON.stringify(message))
            } else if (topic === "bluetooth/pair/result") {
                if (message.success) {
                    statusText.text = "Paired with " + message.name
                    statusText.color = Theme.primary
                } else {
                    statusText.text = "Failed to pair: " + (message.error || "Unknown error")
                    statusText.color = Theme.error
                }
                statusTimer.start()
            } else if (topic === "bluetooth/connect/result") {
                if (message.success) {
                    statusText.text = "Connected to " + message.name
                    statusText.color = Theme.primary
                } else {
                    statusText.text = "Failed to connect: " + (message.error || "Unknown error")
                    statusText.color = Theme.error
                }
                statusTimer.start()
            } else if (topic === "bluetooth/disconnect/result") {
                if (message.success) {
                    statusText.text = "Disconnected from " + message.name
                    statusText.color = Theme.textSecondary
                } else {
                    statusText.text = "Failed to disconnect: " + (message.error || "Unknown error")
                    statusText.color = Theme.error
                }
                statusTimer.start()
            }
        }
    }
}
