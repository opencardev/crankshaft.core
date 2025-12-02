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
    
    contentItem: Text {
        text: control.text
        font.pixelSize: Theme.fontSizeBody
        font.bold: true
        color: control.pressed ? Theme.primary : (control.hovered ? Theme.primaryVariant : Theme.textPrimary)
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
    
    background: Rectangle {
        implicitWidth: 120
        implicitHeight: 48
        color: control.pressed ? Theme.surfaceVariant : (control.hovered ? Theme.surface : "transparent")
        border.color: Theme.primary
        border.width: 2
        radius: Theme.radiusMd
        
        Behavior on color {
            ColorAnimation { duration: Theme.animationDuration }
        }
    }
}
