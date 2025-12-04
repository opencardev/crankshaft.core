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
                text: Strings.appTitle
                font.pixelSize: Theme.fontSizeHeading2
                font.bold: true
                color: Theme.textPrimary
                Layout.fillWidth: true
            }
            
            AppButton {
                text: Strings.buttonSettings
                onClicked: stackView.push(settingsScreen)
            }
        }
    }
    
    // Main content
    ColumnLayout {
        anchors.centerIn: parent
        spacing: Theme.spacingXl
        
        Text {
            text: Strings.homeWelcome
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
                title: Strings.cardNavigationTitle
                description: Strings.cardNavigationDesc
                icon: "navigation"
                Layout.preferredWidth: 250
                Layout.preferredHeight: 200
                
                onClicked: {
                    wsClient.publish("ui/navigation/opened", {})
                }
            }
            
            Card {
                title: Strings.cardMediaTitle
                description: Strings.cardMediaDesc
                icon: "music"
                Layout.preferredWidth: 250
                Layout.preferredHeight: 200
                
                onClicked: {
                    wsClient.publish("ui/media/opened", {})
                }
            }
            
            Card {
                title: Strings.cardPhoneTitle
                description: Strings.cardPhoneDesc
                icon: "phone"
                Layout.preferredWidth: 250
                Layout.preferredHeight: 200
                
                onClicked: {
                    wsClient.publish("ui/phone/opened", {})
                }
            }
            Card {
                title: Strings.cardAndroidAutoTitle
                description: Strings.cardAndroidAutoDesc
                icon: "phone"
                Layout.preferredWidth: 250
                Layout.preferredHeight: 200
                onClicked: {
                    stackView.push(androidautoScreen, { stack: stackView })
                }
            }
            Card {
                title: Strings.cardSystemTitle
                description: Strings.cardSystemDesc
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
