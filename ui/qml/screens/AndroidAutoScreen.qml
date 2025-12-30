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

Rectangle {
    id: androidAutoScreen
    color: '#000000'

    property alias videoSurface: videoSurface
    property alias connectionStatus: connectionStatus
    property var stack: null
    
    property string statusText: qsTr('Android Auto - Disconnected')
    property color statusColor: '#F44336'
    property bool isConnected: false
    
    // Test timer to verify property binding works
    Timer {
        interval: 5000
        running: true
        repeat: true
        onTriggered: {
            console.log("[AndroidAutoScreen] STATUS TEST - statusText:", androidAutoScreen.statusText, "statusColor:", androidAutoScreen.statusColor)
        }
    }
    
    Component.onCompleted: {
        console.log("[AndroidAutoScreen] Component initialized")
        console.log("[AndroidAutoScreen] wsClient connected?", wsClient.connected)
        console.log("[AndroidAutoScreen] Subscribing to android-auto/status/#")
        wsClient.subscribe("android-auto/status/#")
    }
    
    // Also try subscribing when connection is established
    Connections {
        target: wsClient
        function onConnectedChanged() {
            if (wsClient.connected) {
                console.log("[AndroidAutoScreen] WebSocket connected! Re-subscribing...")
                wsClient.subscribe("android-auto/status/#")
            }
        }
    }
    
    Connections {
        target: wsClient
        
        function onEventReceived(topic, payload) {
            console.log("[AndroidAutoScreen] Received event on topic:", topic, "payload:", payload)
            if (topic === "android-auto/status/state-changed") {
                let stateName = payload.stateName || "UNKNOWN"
                console.log("[AndroidAutoScreen] State changed to:", stateName)
                androidAutoScreen.statusText = "Android Auto - " + stateName
                
                if (stateName === "CONNECTED") {
                    androidAutoScreen.statusColor = '#4CAF50'  // Green
                    androidAutoScreen.isConnected = true
                } else if (stateName === "DISCONNECTED") {
                    androidAutoScreen.statusColor = '#F44336'  // Red
                    androidAutoScreen.isConnected = false
                } else {
                    androidAutoScreen.statusColor = '#FF9800'  // Orange for other states
                    androidAutoScreen.isConnected = false
                }
            } else if (topic === "android-auto/status/connected") {
                let device = payload.device
                console.log("[AndroidAutoScreen] Connected, device:", device)
                androidAutoScreen.statusText = "Android Auto - CONNECTED (" + device.model + ")"
                androidAutoScreen.statusColor = '#4CAF50'  // Green
                androidAutoScreen.isConnected = true
            } else if (topic === "android-auto/status/disconnected") {
                console.log("[AndroidAutoScreen] Disconnected")
                androidAutoScreen.statusText = "Android Auto - Disconnected"
                androidAutoScreen.statusColor = '#F44336'  // Red
                androidAutoScreen.isConnected = false
            } else if (topic === "android-auto/status/error") {
                console.log("[AndroidAutoScreen] Error:", payload.error)
                androidAutoScreen.statusText = "Android Auto - Error: " + payload.error
                androidAutoScreen.statusColor = '#F44336'  // Red
                androidAutoScreen.isConnected = false
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Connection status bar
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 60
            color: '#1a1a1a'
            border.color: '#333333'
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 12

                Rectangle {
                    id: statusIndicator
                    Layout.preferredWidth: 12
                    Layout.preferredHeight: 12
                    radius: 6
                    color: AndroidAutoStatus.statusColor
                    
                    Behavior on color {
                        ColorAnimation { duration: 300 }
                    }
                }

                Text {
                    id: connectionStatus
                    color: '#FFFFFF'
                    font.pixelSize: 14
                    font.family: 'Roboto'
                    text: AndroidAutoStatus.statusText
                }

                Item { Layout.fillWidth: true }

                Text {
                    color: '#BBBBBB'
                    font.pixelSize: 12
                    font.family: 'Roboto'
                    text: qsTr('Connect Android device via USB')
                }
            }
        }

        // Video rendering surface
        Rectangle {
            id: videoSurface
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: '#000000'

            MouseArea {
                id: touchArea
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton

                onPressed: (mouse) => {
                    wsClient.publish("androidauto/touch", {
                        "x": mouse.x / width,
                        "y": mouse.y / height,
                        "action": "down"
                    })
                }

                onReleased: (mouse) => {
                    wsClient.publish("androidauto/touch", {
                        "x": mouse.x / width,
                        "y": mouse.y / height,
                        "action": "up"
                    })
                }

                onPositionChanged: (mouse) => {
                    if (pressed) {
                        wsClient.publish("androidauto/touch", {
                            "x": mouse.x / width,
                            "y": mouse.y / height,
                            "action": "move"
                        })
                    }
                }
            }

            BusyIndicator {
                anchors.centerIn: parent
                running: true
            }

            Text {
                anchors.centerIn: parent
                color: '#FFFFFF'
                font.pixelSize: 18
                font.bold: true
                font.family: 'Roboto'
                text: qsTr('Android Auto Projection\nConnect your Android device via USB')
                horizontalAlignment: Text.AlignHCenter
            }
        }

        // Control buttons
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 50
            color: '#1a1a1a'
            border.color: '#333333'
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 8

                Button {
                    id: backButton
                    Layout.preferredWidth: 40
                    Layout.preferredHeight: 40
                    text: '⬅'
                    font.pixelSize: 16
                    onClicked: wsClient.publish("androidauto/key", { "key": "BACK" })
                }

                Button {
                    id: homeButton
                    Layout.preferredWidth: 40
                    Layout.preferredHeight: 40
                    text: '⌂'
                    font.pixelSize: 16
                    onClicked: wsClient.publish("androidauto/key", { "key": "HOME" })
                }

                Item { Layout.fillWidth: true }

                Button {
                    id: disconnectButton
                    Layout.preferredWidth: 40
                    Layout.preferredHeight: 40
                    text: '✕'
                    font.pixelSize: 16
                    onClicked: wsClient.publish("androidauto/disconnect", {})
                }

                Button {
                    id: exitButton
                    Layout.preferredWidth: 60
                    Layout.preferredHeight: 40
                    text: 'Exit'
                    font.pixelSize: 14
                    onClicked: {
                        if (stack) {
                            stack.pop()
                        }
                    }
                }
            }
        }
    }

    // Note: Status is queried via backend connection state changes
    // Not publishing on component load to avoid duplicate events
}