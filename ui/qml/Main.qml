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
import Crankshaft 1.0

ApplicationWindow {
    id: window
    
    visible: true
    width: 1024
    height: 600
    title: Strings.appTitle
    
    color: Theme.background
    
    // Connection status indicator
    Rectangle {
        id: statusBar
        anchors.top: parent.top
        width: parent.width
        height: 4
        color: wsClient.connected ? Theme.success : Theme.error
        
        Behavior on color {
            ColorAnimation { duration: Theme.animationDuration }
        }
    }
    
    // Main content
    StackView {
        id: stackView
        anchors.fill: parent
        anchors.topMargin: statusBar.height
        
        initialItem: homeScreenComponent
        
        onItemPushed: (item) => {
            if (item.stack !== undefined) {
                item.stack = stackView
            }
        }
    }
    
    Component {
        id: homeScreenComponent
        HomeScreen { stack: stackView }
    }
    
    Component {
        id: settingsScreen
        SettingsScreen {}
    }
    
    Component {
        id: androidautoScreen
        AndroidAutoScreen {}
    }
    
    // Handle WebSocket events
    Connections {
        target: wsClient
        
        function onEventReceived(topic, payload) {
            console.log("Event received:", topic, JSON.stringify(payload))
            
            if (topic === "ui/theme/changed") {
                Theme.isDark = payload.mode === "dark"
            }
        }
        
        function onErrorOccurred(error) {
            console.error("WebSocket error:", error)
        }
    }
}
