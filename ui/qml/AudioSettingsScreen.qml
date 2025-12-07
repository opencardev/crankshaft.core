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
import Crankshaft 1.0

Page {
    id: audioSettingsScreen
    title: qsTr("Audio Settings")

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
            anchors.margins: Theme.spacingMd

            AppButton {
                text: "⬅ " + qsTr("Back")
                onClicked: {
                    if (stack) stack.pop()
                    else if (StackView.view) StackView.view.pop()
                }
            }

            Text {
                text: qsTr("Audio Settings")
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
            padding: Theme.spacingMd

            // Master Volume
            Rectangle {
                Layout.fillWidth: true
                height: volumeCol.height + Theme.spacingLg * 2
                color: Theme.surface
                radius: Theme.radiusMd

                ColumnLayout {
                    id: volumeCol
                    anchors.centerIn: parent
                    width: parent.width - Theme.spacingLg * 2
                    spacing: Theme.spacingMd

                    Text {
                        text: qsTr("Master Volume")
                        font.pixelSize: Theme.fontSizeHeading3
                        font.bold: true
                        color: Theme.textPrimary
                    }

                    RowLayout {
                        spacing: Theme.spacingMd

                        Text {
                            text: "0%"
                            font.pixelSize: Theme.fontSizeBody
                            color: Theme.textSecondary
                        }

                        Slider {
                            Layout.fillWidth: true
                            from: 0
                            to: 100
                            value: 70
                            onValueChanged: wsClient.publish("audio/volume", {
                                "master": Math.round(value)
                            })
                        }

                        Text {
                            text: "100%"
                            font.pixelSize: Theme.fontSizeBody
                            color: Theme.textSecondary
                        }
                    }

                    RowLayout {
                        spacing: Theme.spacingMd
                        Layout.fillWidth: true

                        Text {
                            text: qsTr("Mute")
                            font.pixelSize: Theme.fontSizeBody
                            color: Theme.textPrimary
                        }

                        Item { Layout.fillWidth: true }

                        Switch {
                            id: muteSwitch
                            onCheckedChanged: wsClient.publish("audio/mute", {
                                "enabled": checked
                            })
                        }
                    }
                }
            }

            // Audio Route
            Rectangle {
                Layout.fillWidth: true
                height: routeCol.height + Theme.spacingLg * 2
                color: Theme.surface
                radius: Theme.radiusMd

                ColumnLayout {
                    id: routeCol
                    anchors.centerIn: parent
                    width: parent.width - Theme.spacingLg * 2
                    spacing: Theme.spacingMd

                    Text {
                        text: qsTr("Audio Output")
                        font.pixelSize: Theme.fontSizeHeading3
                        font.bold: true
                        color: Theme.textPrimary
                    }

                    ColumnLayout {
                        spacing: Theme.spacingSm

                        RadioButton {
                            text: qsTr("Speaker")
                            onClicked: wsClient.publish("audio/route", {
                                "route": "SPEAKER"
                            })
                        }

                        RadioButton {
                            text: qsTr("Headphone")
                            onClicked: wsClient.publish("audio/route", {
                                "route": "HEADPHONE"
                            })
                        }

                        RadioButton {
                            text: qsTr("Bluetooth")
                            onClicked: wsClient.publish("audio/route", {
                                "route": "BLUETOOTH"
                            })
                        }

                        RadioButton {
                            text: qsTr("USB")
                            onClicked: wsClient.publish("audio/route", {
                                "route": "USB"
                            })
                        }
                    }
                }
            }

            // Stream Volumes
            Rectangle {
                Layout.fillWidth: true
                height: streamsCol.height + Theme.spacingLg * 2
                color: Theme.surface
                radius: Theme.radiusMd

                ColumnLayout {
                    id: streamsCol
                    anchors.centerIn: parent
                    width: parent.width - Theme.spacingLg * 2
                    spacing: Theme.spacingMd

                    Text {
                        text: qsTr("Stream Volumes")
                        font.pixelSize: Theme.fontSizeHeading3
                        font.bold: true
                        color: Theme.textPrimary
                    }

                    // Music Volume
                    ColumnLayout {
                        spacing: Theme.spacingSm

                        Text {
                            text: qsTr("Music")
                            font.pixelSize: Theme.fontSizeBody
                            color: Theme.textPrimary
                        }

                        Slider {
                            Layout.fillWidth: true
                            from: 0
                            to: 100
                            value: 85
                            onValueChanged: wsClient.publish("audio/stream-volume", {
                                "type": "MUSIC",
                                "volume": Math.round(value)
                            })
                        }
                    }

                    // Navigation Volume
                    ColumnLayout {
                        spacing: Theme.spacingSm

                        Text {
                            text: qsTr("Navigation")
                            font.pixelSize: Theme.fontSizeBody
                            color: Theme.textPrimary
                        }

                        Slider {
                            Layout.fillWidth: true
                            from: 0
                            to: 100
                            value: 90
                            onValueChanged: wsClient.publish("audio/stream-volume", {
                                "type": "NAVIGATION",
                                "volume": Math.round(value)
                            })
                        }
                    }

                    // Call Volume
                    ColumnLayout {
                        spacing: Theme.spacingSm

                        Text {
                            text: qsTr("Calls")
                            font.pixelSize: Theme.fontSizeBody
                            color: Theme.textPrimary
                        }

                        Slider {
                            Layout.fillWidth: true
                            from: 0
                            to: 100
                            value: 100
                            onValueChanged: wsClient.publish("audio/stream-volume", {
                                "type": "CALL",
                                "volume": Math.round(value)
                            })
                        }
                    }
                }
            }

            Item { Layout.fillHeight: true }
        }
    }
}
