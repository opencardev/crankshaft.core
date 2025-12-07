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
    id: connectivitySettingsScreen
    title: qsTr("Connectivity Settings")

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
                text: qsTr("Connectivity Settings")
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

            // WiFi Settings
            Rectangle {
                Layout.fillWidth: true
                height: wifiCol.height + Theme.spacingLg * 2
                color: Theme.surface
                radius: Theme.radiusMd

                ColumnLayout {
                    id: wifiCol
                    anchors.centerIn: parent
                    width: parent.width - Theme.spacingLg * 2
                    spacing: Theme.spacingMd

                    RowLayout {
                        spacing: Theme.spacingMd
                        Layout.fillWidth: true

                        Text {
                            text: qsTr("WiFi")
                            font.pixelSize: Theme.fontSizeHeading3
                            font.bold: true
                            color: Theme.textPrimary
                        }

                        Item { Layout.fillWidth: true }

                        Switch {
                            id: wifiSwitch
                            onCheckedChanged: wsClient.publish("wifi/enabled", {
                                "enabled": checked
                            })
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        height: 200
                        color: Theme.background
                        radius: Theme.radiusMd

                        ListView {
                            anchors.fill: parent
                            clip: true

                            model: ListModel {
                                id: wifiNetworks
                                ListElement { ssid: "HomeNetwork"; signal: 80; secured: true }
                                ListElement { ssid: "CoffeeShop"; signal: 60; secured: false }
                                ListElement { ssid: "Airport"; signal: 45; secured: true }
                            }

                            delegate: Rectangle {
                                width: parent.width
                                height: 50
                                color: mouseArea.containsMouse ? Theme.surfaceHover : "transparent"

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: Theme.spacingSm

                                    Text {
                                        text: model.ssid
                                        font.pixelSize: Theme.fontSizeBody
                                        color: Theme.textPrimary
                                        Layout.fillWidth: true
                                    }

                                    Text {
                                        text: "📶 " + model.signal + "%"
                                        font.pixelSize: Theme.fontSizeSmall
                                        color: Theme.textSecondary
                                    }

                                    Text {
                                        text: model.secured ? "🔒" : ""
                                        font.pixelSize: 14
                                    }
                                }

                                MouseArea {
                                    id: mouseArea
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    onClicked: wsClient.publish("wifi/connect", {
                                        "ssid": model.ssid
                                    })
                                }
                            }
                        }
                    }
                }
            }

            // Bluetooth Settings
            Rectangle {
                Layout.fillWidth: true
                height: bluetoothCol.height + Theme.spacingLg * 2
                color: Theme.surface
                radius: Theme.radiusMd

                ColumnLayout {
                    id: bluetoothCol
                    anchors.centerIn: parent
                    width: parent.width - Theme.spacingLg * 2
                    spacing: Theme.spacingMd

                    RowLayout {
                        spacing: Theme.spacingMd
                        Layout.fillWidth: true

                        Text {
                            text: qsTr("Bluetooth")
                            font.pixelSize: Theme.fontSizeHeading3
                            font.bold: true
                            color: Theme.textPrimary
                        }

                        Item { Layout.fillWidth: true }

                        Switch {
                            id: bluetoothSwitch
                            onCheckedChanged: wsClient.publish("bluetooth/enabled", {
                                "enabled": checked
                            })
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        height: 200
                        color: Theme.background
                        radius: Theme.radiusMd

                        ListView {
                            anchors.fill: parent
                            clip: true

                            model: ListModel {
                                id: bluetoothDevices
                                ListElement { name: "Car Phone"; connected: true; type: "phone" }
                                ListElement { name: "Headphones"; connected: false; type: "audio" }
                                ListElement { name: "Smartwatch"; connected: false; type: "wearable" }
                            }

                            delegate: Rectangle {
                                width: parent.width
                                height: 50
                                color: mouseArea.containsMouse ? Theme.surfaceHover : "transparent"

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: Theme.spacingSm

                                    Text {
                                        text: model.name
                                        font.pixelSize: Theme.fontSizeBody
                                        color: Theme.textPrimary
                                        Layout.fillWidth: true
                                    }

                                    Rectangle {
                                        width: 12
                                        height: 12
                                        radius: 6
                                        color: model.connected ? "#4CAF50" : "#CCCCCC"
                                    }
                                }

                                MouseArea {
                                    id: mouseArea
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    onClicked: {
                                        if (model.connected) {
                                            wsClient.publish("bluetooth/disconnect", {
                                                "device": model.name
                                            })
                                        } else {
                                            wsClient.publish("bluetooth/connect", {
                                                "device": model.name
                                            })
                                        }
                                    }
                                }
                            }
                        }
                    }

                    AppButton {
                        Layout.fillWidth: true
                        text: qsTr("Pair New Device")
                        onClicked: wsClient.publish("bluetooth/discover", {})
                    }
                }
            }

            Item { Layout.fillHeight: true }
        }
    }
}
