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
            anchors.margins: 16
            spacing: 16
            
            Text {
                text: Strings.appTitle
                font.pixelSize: 32
                font.bold: true
                color: Theme.textPrimary
                Layout.fillWidth: true
            }
            
            AppButton {
                text: "⚙"
                implicitWidth: 76
                implicitHeight: 76
                onClicked: stackView.push(settingsScreen)
            }
        }
    }
    
    // Main content area following Design for Driving guidelines
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 16
        
        // Status bar - Primary driving information
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 64
            color: Theme.surface
            radius: 4
            
            RowLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 12
                
                Text {
                    text: Strings.homeWelcome
                    font.pixelSize: 28
                    font.bold: true
                    color: Theme.textPrimary
                    Layout.fillWidth: true
                }
                
                Text {
                    text: Qt.formatTime(new Date(), "hh:mm")
                    font.pixelSize: 20
                    color: Theme.textSecondary
                }
            }
        }
        
        // Primary action cards (60% of screen) - Navigation and Media
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: parent.height * 0.6
            color: "transparent"
            
            GridLayout {
                anchors.fill: parent
                columns: 2
                rowSpacing: 12
                columnSpacing: 12
                
                // Navigation - Primary driving task
                Card {
                    title: Strings.cardNavigationTitle
                    description: Strings.cardNavigationDesc
                    icon: "navigation"
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumHeight: 120
                    
                    onClicked: {
                        wsClient.publish("ui/navigation/opened", {})
                    }
                }
                
                // Phone - Secondary driving task
                Card {
                    title: Strings.cardPhoneTitle
                    description: Strings.cardPhoneDesc
                    icon: "phone"
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumHeight: 120
                    
                    onClicked: {
                        wsClient.publish("ui/phone/opened", {})
                    }
                }
                
                // Media - Entertainment
                Card {
                    title: Strings.cardMediaTitle
                    description: Strings.cardMediaDesc
                    icon: "music"
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumHeight: 120
                    
                    onClicked: {
                        wsClient.publish("ui/media/opened", {})
                    }
                }
                
                // Android Auto
                Card {
                    title: Strings.cardAndroidAutoTitle
                    description: Strings.cardAndroidAutoDesc
                    icon: "phone"
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumHeight: 120
                    
                    onClicked: {
                        stackView.push(androidautoScreen, { stack: stackView })
                    }
                }
            }
        }
        
        // Secondary action - Settings (40% of screen)
        Card {
            title: Strings.cardSystemTitle
            description: Strings.cardSystemDesc
            icon: "settings"
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: 100
            
            onClicked: {
                stackView.push(settingsScreen)
            }
        }
    }
}
