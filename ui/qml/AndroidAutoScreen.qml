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

Rectangle {
    id: androidAutoScreen
    color: '#000000'

    property alias videoSurface: videoSurface
    property alias connectionStatus: connectionStatus
    property var stack: null

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
                    color: '#F44336'
                }

                Text {
                    id: connectionStatus
                    color: '#FFFFFF'
                    font.pixelSize: 14
                    font.family: 'Roboto'
                    text: qsTr('Android Auto - Disconnected')
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
                    onClicked: {
                        if (stack) {
                            stack.pop()
                        }
                    }
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
                    id: settingsButton
                    Layout.preferredWidth: 40
                    Layout.preferredHeight: 40
                    text: '⚙'
                    font.pixelSize: 16
                    onClicked: wsClient.publish("androidauto/settings", {})
                }

                Button {
                    id: disconnectButton
                    Layout.preferredWidth: 40
                    Layout.preferredHeight: 40
                    text: '✕'
                    font.pixelSize: 16
                    onClicked: wsClient.publish("androidauto/disconnect", {})
                }
            }
        }
    }

    Component.onCompleted: {
        console.log("AndroidAutoScreen loaded")
        wsClient.publish("androidauto/status", {})
    }
}