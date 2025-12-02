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
            
            Text {
                text: qsTr("Crankshaft")
                font.pixelSize: Theme.fontSizeHeading2
                font.bold: true
                color: Theme.textPrimary
                Layout.fillWidth: true
            }
            
            AppButton {
                text: qsTr("Settings")
                onClicked: stackView.push(settingsScreen)
            }
        }
    }
    
    // Main content
    ColumnLayout {
        anchors.centerIn: parent
        spacing: Theme.spacingXl
        
        Text {
            text: qsTr("Welcome to Crankshaft MVP")
            font.pixelSize: Theme.fontSizeHeading1
            font.bold: true
            color: Theme.textPrimary
            Layout.alignment: Qt.AlignHCenter
        }
        
        GridLayout {
            columns: 2
            rowSpacing: Theme.spacingMd
            columnSpacing: Theme.spacingMd
            Layout.alignment: Qt.AlignHCenter
            
            Card {
                title: qsTr("Navigation")
                description: qsTr("GPS and route planning")
                icon: "navigation"
                Layout.preferredWidth: 250
                Layout.preferredHeight: 200
                
                onClicked: {
                    wsClient.publish("ui/navigation/opened", {})
                }
            }
            
            Card {
                title: qsTr("Media")
                description: qsTr("Music and audio")
                icon: "music"
                Layout.preferredWidth: 250
                Layout.preferredHeight: 200
                
                onClicked: {
                    wsClient.publish("ui/media/opened", {})
                }
            }
            
            Card {
                title: qsTr("Phone")
                description: qsTr("Calls and contacts")
                icon: "phone"
                Layout.preferredWidth: 250
                Layout.preferredHeight: 200
                
                onClicked: {
                    wsClient.publish("ui/phone/opened", {})
                }
            }
            
            Card {
                title: qsTr("System")
                description: qsTr("Settings and info")
                icon: "settings"
                Layout.preferredWidth: 250
                Layout.preferredHeight: 200
                
                onClicked: {
                    stackView.push(settingsScreen)
                }
            }
        }
    }
}
