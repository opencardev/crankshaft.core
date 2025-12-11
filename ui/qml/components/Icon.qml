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
import "MaterialDesignIcons.js" as MDI

Item {
    id: root
    
    property string name: ""
    property int size: 24
    property color color: Theme.textPrimary
    
    width: size
    height: size
    
    // Load Material Design Icons font
    FontLoader {
        id: mdiFont
        source: "qrc:/fonts/materialdesignicons-webfont.ttf"
    }
    
    // Use Material Design Icons font for mdi- prefixed icons
    Text {
        anchors.fill: parent
        text: getIcon(root.name)
        font.pixelSize: root.size
        font.family: isMaterialIcon(root.name) ? mdiFont.name : font.family
        color: root.color
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
    
    function isMaterialIcon(iconName) {
        return iconName.startsWith("mdi-")
    }
    
    function getIcon(iconName) {
        // If it's a Material Design Icon, use the comprehensive MDI library
        if (isMaterialIcon(iconName)) {
            return MDI.getMaterialIcon(iconName.substring(4)) // Remove "mdi-" prefix
        }
        
        // Fallback icon mappings using Unicode characters for better compatibility
        const iconMap = {
            // Navigation & UI
            "navigation": "🧭",
            "arrow-left": "←",
            "arrow-right": "→",
            "close": "✕",
            "menu": "☰",
            
            // Media & Entertainment
            "music": "♫",
            "play": "▶",
            "pause": "⏸",
            "stop": "⏹",
            "volume": "🔊",
            "mute": "🔇",
             
            // Communication
            "phone": "☎",
            "call": "📞",
            "message": "💬",
            "bluetooth": "🔵",
            
            // Home & Settings
            "home": "⌂",
            "settings": "⚙",
            "gear": "⚙",
            
            // Status
            "star": "★",
            "heart": "♥",
            "check": "✓",
            "error": "⚠",
            
            // Common Actions
            "back": "◄",
            "forward": "►",
            "refresh": "🔄",
            "search": "🔍",
            "add": "✚",
            "remove": "✕",
            "download": "⬇",
            "upload": "⬆",
            "info": "ⓘ",
            "help": "?"
        };
        
        return iconMap[iconName] || "?";
    }
}
