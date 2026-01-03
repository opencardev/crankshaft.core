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

Column {
    id: root
    
    spacing: 16
    
    // List of supported locales with their display names
    property var locales: [
        { code: 'en_GB', name: qsTr('English (GB)', 'LocaleSelector') },
        { code: 'en_US', name: qsTr('English (US)', 'LocaleSelector') },
        { code: 'de_DE', name: qsTr('Deutsch (DE)', 'LocaleSelector') },
        { code: 'fr_FR', name: qsTr('Français (FR)', 'LocaleSelector') },
        { code: 'es_ES', name: qsTr('Español (ES)', 'LocaleSelector') }
    ]
    
    // Currently selected locale
    property string selectedLocale: Qt.locale().name.replace(/-/g, '_')
    
    // Signal emitted when locale changes
    signal localeChanged(string newLocale)
    
    Text {
        text: qsTr('Language', 'LocaleSelector')
        font.pixelSize: Theme.fontSizeHeading2
        color: Theme.textPrimary
    }
    
    // Button grid for locale selection
    GridLayout {
        columns: 2
        rowSpacing: 8
        columnSpacing: 8
        width: parent.width
        
        Repeater {
            model: root.locales
            
            Button {
                id: localeButton
                
                required property var modelData
                
                Layout.fillWidth: true
                implicitHeight: 56
                
                text: modelData.name
                
                // Visual feedback for selected locale
                background: Rectangle {
                    color: localeButton.modelData.code === root.selectedLocale ? 
                           Theme.primary : Theme.surface
                    radius: 4
                    border.color: Theme.divider
                    border.width: 1
                    
                    Behavior on color {
                        ColorAnimation { duration: 150 }
                    }
                }
                
                contentItem: Text {
                    text: localeButton.text
                    font.pixelSize: Theme.fontSizeBody
                    color: localeButton.modelData.code === root.selectedLocale ? 
                           '#FFFFFF' : Theme.textPrimary
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    
                    Behavior on color {
                        ColorAnimation { duration: 150 }
                    }
                }
                
                onClicked: {
                    root.selectedLocale = modelData.code
                    root.localeChanged(modelData.code)
                    
                    // Store preference
                    Qt.locale.setLocale(modelData.code)
                }
                
                // Visual feedback on press
                scale: pressed ? 0.95 : 1.0
                Behavior on scale {
                    NumberAnimation { duration: 100 }
                }
            }
        }
    }
    
    // Display current locale code for debugging
    Text {
        text: qsTr('Current: %1', 'LocaleSelector').arg(root.selectedLocale)
        font.pixelSize: Theme.fontSizeCaption
        color: Theme.textSecondary
        opacity: 0.7
    }
}
