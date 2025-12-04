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

Item {
    id: root
    
    property string title: ""
    property string description: ""
    property string icon: ""
    signal clicked()
    
    width: 200
    height: 150
    
    scale: mouseArea.pressed ? 0.95 : (mouseArea.containsMouse ? 1.02 : 1.0)
    
    Behavior on scale {
        NumberAnimation { duration: Theme.animationDuration }
    }
    
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: root.clicked()
    }
    
    Rectangle {
        id: background
        anchors.fill: parent
        color: Theme.surface
        radius: Theme.radiusLg
        border.color: Theme.divider
        border.width: 1
        
        Behavior on color {
            ColorAnimation { duration: Theme.animationDuration }
        }
        
        Column {
            anchors.centerIn: parent
            spacing: Theme.spacingMd
            width: parent.width - Theme.spacingLg * 2
            
            Icon {
                name: root.icon
                size: 48
                anchors.horizontalCenter: parent.horizontalCenter
            }
            
            Text {
                text: root.title
                font.pixelSize: Theme.fontSizeHeading3
                font.bold: true
                color: Theme.textPrimary
                horizontalAlignment: Text.AlignHCenter
                width: parent.width
                wrapMode: Text.Wrap
            }
            
            Text {
                text: root.description
                font.pixelSize: Theme.fontSizeCaption
                color: Theme.textSecondary
                horizontalAlignment: Text.AlignHCenter
                width: parent.width
                wrapMode: Text.Wrap
            }
        }
        
        // Ripple effect
        Rectangle {
            id: ripple
            anchors.centerIn: parent
            width: 0
            height: width
            radius: width / 2
            color: Theme.primary
            opacity: 0
            
            ParallelAnimation {
                id: rippleAnimation
                NumberAnimation {
                    target: ripple
                    property: "width"
                    from: 0
                    to: background.width * 2
                    duration: 400
                }
                NumberAnimation {
                    target: ripple
                    property: "opacity"
                    from: 0.3
                    to: 0
                    duration: 400
                }
            }
        }
    }
    
    Connections {
        target: mouseArea
        function onPressed() {
            rippleAnimation.start()
        }
    }
}
