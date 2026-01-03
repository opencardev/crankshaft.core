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
import Crankshaft 1.0

Item {
    id: root
    
    // Properties for customisation
    property string timeFormat: "hh:mm"
    property int fontSize: 20
    property color textColor: Theme.textSecondary
    property int updateInterval: 1000
    
    // Current time property
    property string currentTime: Qt.formatTime(new Date(), root.timeFormat)
    
    implicitWidth: clockText.implicitWidth
    implicitHeight: clockText.implicitHeight
    
    // Timer to update clock every second
    Timer {
        id: clockTimer
        interval: root.updateInterval
        running: true
        repeat: true
        onTriggered: {
            root.currentTime = Qt.formatTime(new Date(), root.timeFormat)
        }
    }
    
    // Clock display text
    Text {
        id: clockText
        text: root.currentTime
        font.pixelSize: root.fontSize
        color: root.textColor
        anchors.centerIn: parent
        
        // Smooth transitions for time changes
        Behavior on text {
            SequentialAnimation {
                NumberAnimation {
                    target: clockText
                    property: "opacity"
                    to: 0.7
                    duration: 100
                }
                NumberAnimation {
                    target: clockText
                    property: "opacity"
                    to: 1.0
                    duration: 100
                }
            }
        }
    }
    
    // Ensure clock starts with current time immediately
    Component.onCompleted: {
        root.currentTime = Qt.formatTime(new Date(), root.timeFormat)
    }
}
