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

Item {
    id: root
    
    property string name: ""
    property int size: 24
    property color color: Theme.textPrimary
    
    width: size
    height: size
    
    // Placeholder for Material Design Icons
    // In a real implementation, this would load SVG icons or use an icon font
    Text {
        anchors.fill: parent
        text: getIconChar(root.name)
        font.pixelSize: root.size
        font.family: "Material Design Icons"
        color: root.color
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
    
    function getIconChar(iconName) {
        // Map icon names to unicode characters (placeholder)
        const iconMap = {
            "navigation": "🧭",
            "music": "🎵",
            "phone": "📱",
            "settings": "⚙️",
            "home": "🏠",
            "back": "←"
        }
        return iconMap[iconName] || "?"
    }
}
