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
import Crankshaft 1.0
import "../components"

Item {
    id: root

    WebSocketClient {
        id: wsClient
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20

        // Title
        Label {
            text: qsTr("Android Auto Settings")
            font.pixelSize: 28
            font.bold: true
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            contentWidth: availableWidth

            ColumnLayout {
                width: parent.width
                spacing: 15

                // Connection Status Section
                GroupBox {
                    Layout.fillWidth: true
                    title: qsTr("Connection Status")

                    ColumnLayout {
                        width: parent.width
                        spacing: 10

                        RowLayout {
                            Label {
                                text: qsTr("Status:")
                                Layout.preferredWidth: 150
                            }
                            Label {
                                id: statusLabel
                                text: qsTr("Disconnected")
                                color: Theme.textSecondary
                            }
                        }

                        RowLayout {
                            Label {
                                text: qsTr("Device:")
                                Layout.preferredWidth: 150
                            }
                            Label {
                                id: deviceLabel
                                text: qsTr("None")
                                color: Theme.textSecondary
                            }
                        }

                        Button {
                            text: qsTr("Connect")
                            Layout.alignment: Qt.AlignRight
                            onClicked: {
                                wsClient.publish("android-auto/connect", {})
                            }
                        }
                    }
                }

                // Display Settings Section
                GroupBox {
                    Layout.fillWidth: true
                    title: qsTr("Display Settings")

                    ColumnLayout {
                        width: parent.width
                        spacing: 15

                        RowLayout {
                            Label {
                                text: qsTr("Resolution:")
                                Layout.preferredWidth: 150
                            }
                            ComboBox {
                                id: resolutionCombo
                                Layout.fillWidth: true
                                model: ["800x480", "1024x600", "1280x720", "1920x1080"]
                                currentIndex: 1
                                onActivated: {
                                    wsClient.publish("android-auto/resolution", {
                                        "value": currentText
                                    })
                                }
                            }
                        }

                        RowLayout {
                            Label {
                                text: qsTr("Framerate:")
                                Layout.preferredWidth: 150
                            }
                            SpinBox {
                                id: fpsSpinBox
                                from: 15
                                to: 60
                                value: 30
                                onValueModified: {
                                    wsClient.publish("android-auto/fps", {
                                        "value": value
                                    })
                                }
                            }
                            Label {
                                text: qsTr("fps")
                            }
                        }
                    }
                }

                // Channel Configuration Section
                GroupBox {
                    Layout.fillWidth: true
                    title: qsTr("Channel Configuration")
                    
                    Label {
                        text: qsTr("Note: Changing channel settings requires reconnection")
                        font.pixelSize: 12
                        color: Theme.textSecondary
                        wrapMode: Text.WordWrap
                        width: parent.width
                    }

                    ColumnLayout {
                        width: parent.width
                        spacing: 15

                        // Video Channel
                        RowLayout {
                            Layout.fillWidth: true
                            
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 5
                                
                                Label {
                                    text: qsTr("Video Channel")
                                    font.bold: true
                                }
                                Label {
                                    text: qsTr("Display projection from Android device")
                                    font.pixelSize: 12
                                    color: Theme.textSecondary
                                    wrapMode: Text.WordWrap
                                }
                            }
                            
                            Switch {
                                id: videoChannelSwitch
                                checked: true
                                onToggled: {
                                    wsClient.publish("android-auto/channels/video", {
                                        "enabled": checked
                                    })
                                }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            height: 1
                            color: Theme.border
                        }

                        // Media Audio Channel
                        RowLayout {
                            Layout.fillWidth: true
                            
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 5
                                
                                Label {
                                    text: qsTr("Media Audio Channel")
                                    font.bold: true
                                }
                                Label {
                                    text: qsTr("Music and media playback audio")
                                    font.pixelSize: 12
                                    color: Theme.textSecondary
                                    wrapMode: Text.WordWrap
                                }
                            }
                            
                            Switch {
                                id: mediaAudioChannelSwitch
                                checked: true
                                onToggled: {
                                    wsClient.publish("android-auto/channels/media-audio", {
                                        "enabled": checked
                                    })
                                }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            height: 1
                            color: Theme.border
                        }

                        // System Audio Channel
                        RowLayout {
                            Layout.fillWidth: true
                            
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 5
                                
                                Label {
                                    text: qsTr("System Audio Channel")
                                    font.bold: true
                                }
                                Label {
                                    text: qsTr("System sounds and notifications")
                                    font.pixelSize: 12
                                    color: Theme.textSecondary
                                    wrapMode: Text.WordWrap
                                }
                            }
                            
                            Switch {
                                id: systemAudioChannelSwitch
                                checked: true
                                onToggled: {
                                    wsClient.publish("android-auto/channels/system-audio", {
                                        "enabled": checked
                                    })
                                }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            height: 1
                            color: Theme.border
                        }

                        // Speech Audio Channel
                        RowLayout {
                            Layout.fillWidth: true
                            
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 5
                                
                                Label {
                                    text: qsTr("Speech Audio Channel")
                                    font.bold: true
                                }
                                Label {
                                    text: qsTr("Navigation guidance and voice assistant")
                                    font.pixelSize: 12
                                    color: Theme.textSecondary
                                    wrapMode: Text.WordWrap
                                }
                            }
                            
                            Switch {
                                id: speechAudioChannelSwitch
                                checked: true
                                onToggled: {
                                    wsClient.publish("android-auto/channels/speech-audio", {
                                        "enabled": checked
                                    })
                                }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            height: 1
                            color: Theme.border
                        }

                        // Microphone Channel
                        RowLayout {
                            Layout.fillWidth: true
                            
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 5
                                
                                Label {
                                    text: qsTr("Microphone Channel")
                                    font.bold: true
                                }
                                Label {
                                    text: qsTr("Voice commands and phone calls")
                                    font.pixelSize: 12
                                    color: Theme.textSecondary
                                    wrapMode: Text.WordWrap
                                }
                            }
                            
                            Switch {
                                id: microphoneChannelSwitch
                                checked: true
                                onToggled: {
                                    wsClient.publish("android-auto/channels/microphone", {
                                        "enabled": checked
                                    })
                                }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            height: 1
                            color: Theme.border
                        }

                        // Input Channel
                        RowLayout {
                            Layout.fillWidth: true
                            
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 5
                                
                                Label {
                                    text: qsTr("Input Channel")
                                    font.bold: true
                                }
                                Label {
                                    text: qsTr("Touch screen and button inputs")
                                    font.pixelSize: 12
                                    color: Theme.textSecondary
                                    wrapMode: Text.WordWrap
                                }
                            }
                            
                            Switch {
                                id: inputChannelSwitch
                                checked: true
                                onToggled: {
                                    wsClient.publish("android-auto/channels/input", {
                                        "enabled": checked
                                    })
                                }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            height: 1
                            color: Theme.border
                        }

                        // Sensor Channel
                        RowLayout {
                            Layout.fillWidth: true
                            
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 5
                                
                                Label {
                                    text: qsTr("Sensor Channel")
                                    font.bold: true
                                }
                                Label {
                                    text: qsTr("GPS, speed, night mode and vehicle sensors")
                                    font.pixelSize: 12
                                    color: Theme.textSecondary
                                    wrapMode: Text.WordWrap
                                }
                            }
                            
                            Switch {
                                id: sensorChannelSwitch
                                checked: true
                                onToggled: {
                                    wsClient.publish("android-auto/channels/sensor", {
                                        "enabled": checked
                                    })
                                }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            height: 1
                            color: Theme.border
                        }

                        // Bluetooth Channel
                        RowLayout {
                            Layout.fillWidth: true
                            
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 5
                                
                                Label {
                                    text: qsTr("Bluetooth Channel")
                                    font.bold: true
                                }
                                Label {
                                    text: qsTr("Wireless Android Auto over Bluetooth")
                                    font.pixelSize: 12
                                    color: Theme.textSecondary
                                    wrapMode: Text.WordWrap
                                }
                            }
                            
                            Switch {
                                id: bluetoothChannelSwitch
                                checked: false
                                onToggled: {
                                    wsClient.publish("android-auto/channels/bluetooth", {
                                        "enabled": checked
                                    })
                                }
                            }
                        }
                    }
                }

                // Advanced Settings Section
                GroupBox {
                    Layout.fillWidth: true
                    title: qsTr("Advanced Settings")

                    ColumnLayout {
                        width: parent.width
                        spacing: 15

                        RowLayout {
                            Layout.fillWidth: true
                            
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 5
                                
                                Label {
                                    text: qsTr("Use Mock Device")
                                    font.bold: true
                                }
                                Label {
                                    text: qsTr("Use simulated Android Auto for testing")
                                    font.pixelSize: 12
                                    color: Theme.textSecondary
                                    wrapMode: Text.WordWrap
                                }
                            }
                            
                            Switch {
                                id: useMockSwitch
                                checked: true
                                onToggled: {
                                    wsClient.publish("android-auto/use-mock", {
                                        "enabled": checked
                                    })
                                }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            height: 1
                            color: Theme.border
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 5
                                
                                Label {
                                    text: qsTr("Generate Test Video")
                                    font.bold: true
                                }
                                Label {
                                    text: qsTr("Display test pattern when using mock device")
                                    font.pixelSize: 12
                                    color: Theme.textSecondary
                                    wrapMode: Text.WordWrap
                                }
                            }
                            
                            Switch {
                                id: testVideoSwitch
                                checked: true
                                enabled: useMockSwitch.checked
                                onToggled: {
                                    wsClient.publish("android-auto/test-video", {
                                        "enabled": checked
                                    })
                                }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            height: 1
                            color: Theme.border
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 5
                                
                                Label {
                                    text: qsTr("Generate Test Audio")
                                    font.bold: true
                                }
                                Label {
                                    text: qsTr("Play test tone when using mock device")
                                    font.pixelSize: 12
                                    color: Theme.textSecondary
                                    wrapMode: Text.WordWrap
                                }
                            }
                            
                            Switch {
                                id: testAudioSwitch
                                checked: false
                                enabled: useMockSwitch.checked
                                onToggled: {
                                    wsClient.publish("android-auto/test-audio", {
                                        "enabled": checked
                                    })
                                }
                            }
                        }
                    }
                }

                Item {
                    Layout.fillHeight: true
                }
            }
        }
    }

    Component.onCompleted: {
        // Subscribe to Android Auto status updates
        wsClient.subscribe("android-auto/status")
    }

    Connections {
        target: wsClient

        function onMessageReceived(topic, payload) {
            if (topic === "android-auto/status") {
                if (payload.hasOwnProperty("state")) {
                    statusLabel.text = payload.state
                }
                if (payload.hasOwnProperty("device")) {
                    deviceLabel.text = payload.device
                }
            }
        }
    }
}
