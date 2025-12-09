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
    
    // Properties following Design for Driving guidelines
    property string title: ""
    property string description: ""
    property string icon: ""
    signal clicked()
    
    // Minimum touch target: 76dp, but cards can be larger
    width: 200
    height: 150
    
    // Responsive scale with fast feedback (250ms response time requirement)
    scale: mouseArea.pressed ? 0.95 : (mouseArea.containsMouse ? 1.02 : 1.0)
    
    Behavior on scale {
        NumberAnimation { duration: 150 }  // 150ms for quick visual feedback
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
        // Ensure 4.5:1 contrast ratio
        color: Theme.surface
        radius: 4  // 4dp border radius
        border.color: Theme.divider
        border.width: 1
        
        Behavior on color {
            ColorAnimation { duration: 150 }
        }
        
        Column {
            anchors.centerIn: parent
            anchors.margins: 12  // 12dp padding per guidelines
            spacing: 8  // 8dp spacing (grid multiples)
            width: parent.width - 24
            
            Icon {
                name: root.icon
                size: 48
                anchors.horizontalCenter: parent.horizontalCenter
                // Icon must maintain 4.5:1 contrast
            }
            
            Text {
                text: root.title
                // Primary text: 32dp, but scaled down for cards
                font.pixelSize: 24
                font.bold: true
                color: Theme.textPrimary
                horizontalAlignment: Text.AlignHCenter
                width: parent.width
                wrapMode: Text.Wrap
                elide: Text.ElideRight
                maximumLineCount: 2
            }
            
            Text {
                text: root.description
                // Secondary text: 20dp per guidelines
                font.pixelSize: 16
                color: Theme.textSecondary
                horizontalAlignment: Text.AlignHCenter
                width: parent.width
                wrapMode: Text.Wrap
                elide: Text.ElideRight
                maximumLineCount: 2
            }
        }
        
        // Ripple effect - visual feedback within 250ms
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
                // Fast ripple for quick visual feedback (250ms requirement)
                NumberAnimation {
                    target: ripple
                    property: "width"
                    from: 0
                    to: background.width * 2
                    duration: 250
                }
                NumberAnimation {
                    target: ripple
                    property: "opacity"
                    from: 0.3
                    to: 0
                    duration: 250
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
