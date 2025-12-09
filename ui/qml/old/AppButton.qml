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

Button {
    id: control
    
    // Design for Driving: Minimum touch target 76 x 76dp
    implicitWidth: 76
    implicitHeight: 76
    
    contentItem: Text {
        text: control.text
        // Primary text: 24dp minimum per guidelines
        font.pixelSize: 24
        font.bold: true
        // Ensure 4.5:1 contrast ratio
        color: control.pressed ? Theme.primary : (control.hovered ? Theme.primaryVariant : Theme.textPrimary)
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
    
    background: Rectangle {
        implicitWidth: 76
        implicitHeight: 76
        // High contrast background for visibility
        color: control.pressed ? Theme.surfaceVariant : (control.hovered ? Theme.surface : "transparent")
        border.color: Theme.primary
        border.width: 2
        radius: 4  // 4dp border radius
        
        // Quick feedback (250ms requirement)
        Behavior on color {
            ColorAnimation { duration: 150 }
        }
    }
}
