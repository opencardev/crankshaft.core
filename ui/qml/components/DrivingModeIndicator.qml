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

// Floating restriction indicator shown when driving mode is active
Rectangle {
    id: root
    
    // Properties from DrivingModeService
    property bool isDrivingMode: false
    property float vehicleSpeed: 0
    property bool isRestricted: false
    property string restrictionReason: ""
    
    // Styling
    property real cornerRadius: 8
    property real contentMargin: 12
    
    width: parent.width
    height: childrenRect.height + (contentMargin * 2)
    color: Theme.warning
    radius: cornerRadius
    opacity: isDrivingMode ? 1.0 : 0.0
    
    // Smooth appearance/disappearance
    Behavior on opacity {
        NumberAnimation { duration: 300 }
    }
    
    // Only visible when driving
    visible: isDrivingMode
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: contentMargin
        spacing: 8
        
        // Header: Driving Mode Active
        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            
            Text {
                text: "🚗"  // Car emoji for visual impact
                font.pixelSize: 24
                verticalAlignment: Text.AlignVCenter
            }
            
            Text {
                text: qsTr('Driving Mode Active', 'DrivingModeIndicator')
                font.pixelSize: Theme.fontSizeHeading2
                font.bold: true
                color: Theme.textPrimary
                Layout.fillWidth: true
                verticalAlignment: Text.AlignVCenter
            }
            
            Text {
                text: vehicleSpeed.toFixed(0) + " mph"
                font.pixelSize: Theme.fontSizeBody
                color: Theme.textSecondary
                verticalAlignment: Text.AlignVCenter
            }
        }
        
        // Restriction message
        Text {
            text: restrictionReason
            font.pixelSize: Theme.fontSizeBody
            color: Theme.textPrimary
            wrapMode: Text.Wrap
            Layout.fillWidth: true
            visible: restrictionReason.length > 0
        }
        
        // Allowed actions during driving
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: allowedActionsColumn.height + 12
            color: rgba(0, 0, 0, 0.1)
            radius: 4
            
            ColumnLayout {
                id: allowedActionsColumn
                anchors.fill: parent
                anchors.margins: 8
                spacing: 4
                
                Text {
                    text: qsTr('Safe Controls:', 'DrivingModeIndicator')
                    font.pixelSize: Theme.fontSizeCaption
                    color: Theme.textSecondary
                    font.bold: true
                }
                
                Text {
                    text: qsTr('• Play/Pause • Skip • Volume • Navigation • Calls', 
                              'DrivingModeIndicator')
                    font.pixelSize: Theme.fontSizeCaption
                    color: Theme.textSecondary
                    wrapMode: Text.Wrap
                    Layout.fillWidth: true
                }
            }
        }
        
        // Acknowledge button (for manual dismissal)
        Button {
            Layout.fillWidth: true
            Layout.preferredHeight: 44
            
            text: qsTr('Understood', 'DrivingModeIndicator')
            
            background: Rectangle {
                color: Theme.primary
                radius: 4
                
                Behavior on color {
                    ColorAnimation { duration: 150 }
                }
            }
            
            contentItem: Text {
                text: parent.text
                font.pixelSize: Theme.fontSizeBody
                color: '#FFFFFF'
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            
            onClicked: {
                // Acknowledge the restriction
                // In real app: drivingModeService.acknowledgeRestriction()
                root.opacity = 0.0
            }
        }
    }
    
    // Helper function for rgba colors
    function rgba(r, g, b, a) {
        return Qt.rgba(r/255, g/255, b/255, a)
    }
}
