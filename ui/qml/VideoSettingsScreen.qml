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
    id: videoSettingsScreen
    title: qsTr("Video Settings")

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
                text: qsTr("Video Settings")
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

            // Display Settings
            Rectangle {
                Layout.fillWidth: true
                height: displayCol.height + Theme.spacingLg * 2
                color: Theme.surface
                radius: Theme.radiusMd

                ColumnLayout {
                    id: displayCol
                    anchors.centerIn: parent
                    width: parent.width - Theme.spacingLg * 2
                    spacing: Theme.spacingMd

                    Text {
                        text: qsTr("Display Settings")
                        font.pixelSize: Theme.fontSizeHeading3
                        font.bold: true
                        color: Theme.textPrimary
                    }

                    // Resolution
                    ColumnLayout {
                        spacing: Theme.spacingSm

                        Text {
                            text: qsTr("Resolution")
                            font.pixelSize: Theme.fontSizeBody
                            color: Theme.textPrimary
                        }

                        ComboBox {
                            Layout.fillWidth: true
                            model: ["1024x600", "1280x720", "1920x1080"]
                            onCurrentTextChanged: wsClient.publish("video/resolution", {
                                "resolution": currentText
                            })
                        }
                    }

                    // Refresh Rate
                    ColumnLayout {
                        spacing: Theme.spacingSm

                        Text {
                            text: qsTr("Refresh Rate")
                            font.pixelSize: Theme.fontSizeBody
                            color: Theme.textPrimary
                        }

                        ComboBox {
                            Layout.fillWidth: true
                            model: ["30 Hz", "60 Hz", "120 Hz"]
                            onCurrentTextChanged: wsClient.publish("video/refresh-rate", {
                                "rate": currentText.split(" ")[0]
                            })
                        }
                    }
                }
            }

            // Brightness and Contrast
            Rectangle {
                Layout.fillWidth: true
                height: brightnessCol.height + Theme.spacingLg * 2
                color: Theme.surface
                radius: Theme.radiusMd

                ColumnLayout {
                    id: brightnessCol
                    anchors.centerIn: parent
                    width: parent.width - Theme.spacingLg * 2
                    spacing: Theme.spacingMd

                    // Brightness
                    ColumnLayout {
                        spacing: Theme.spacingSm

                        Text {
                            text: qsTr("Brightness")
                            font.pixelSize: Theme.fontSizeBody
                            color: Theme.textPrimary
                        }

                        Slider {
                            Layout.fillWidth: true
                            from: 0
                            to: 100
                            value: 80
                            onValueChanged: wsClient.publish("video/brightness", {
                                "value": Math.round(value)
                            })
                        }
                    }

                    // Contrast
                    ColumnLayout {
                        spacing: Theme.spacingSm

                        Text {
                            text: qsTr("Contrast")
                            font.pixelSize: Theme.fontSizeBody
                            color: Theme.textPrimary
                        }

                        Slider {
                            Layout.fillWidth: true
                            from: 0
                            to: 100
                            value: 50
                            onValueChanged: wsClient.publish("video/contrast", {
                                "value": Math.round(value)
                            })
                        }
                    }
                }
            }

            // Night Mode
            Rectangle {
                Layout.fillWidth: true
                height: nightModeCol.height + Theme.spacingLg * 2
                color: Theme.surface
                radius: Theme.radiusMd

                ColumnLayout {
                    id: nightModeCol
                    anchors.centerIn: parent
                    width: parent.width - Theme.spacingLg * 2
                    spacing: Theme.spacingMd

                    RowLayout {
                        spacing: Theme.spacingMd
                        Layout.fillWidth: true

                        Text {
                            text: qsTr("Night Mode")
                            font.pixelSize: Theme.fontSizeHeading3
                            font.bold: true
                            color: Theme.textPrimary
                        }

                        Item { Layout.fillWidth: true }

                        Switch {
                            id: nightModeSwitch
                            onCheckedChanged: wsClient.publish("video/night-mode", {
                                "enabled": checked
                            })
                        }
                    }

                    // Colour Temperature
                    ColumnLayout {
                        spacing: Theme.spacingSm
                        visible: nightModeSwitch.checked

                        Text {
                            text: qsTr("Colour Temperature")
                            font.pixelSize: Theme.fontSizeBody
                            color: Theme.textPrimary
                        }

                        Slider {
                            Layout.fillWidth: true
                            from: 2700
                            to: 6500
                            value: 3500
                            onValueChanged: wsClient.publish("video/colour-temp", {
                                "kelvin": Math.round(value)
                            })
                        }

                        Text {
                            text: Math.round(colorTempSlider.value) + "K"
                            font.pixelSize: Theme.fontSizeSmall
                            color: Theme.textSecondary
                        }
                    }
                }
            }

            Item { Layout.fillHeight: true }
        }
    }
}
