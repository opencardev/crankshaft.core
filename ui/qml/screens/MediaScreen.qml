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
import QtQuick.Dialogs
import "../components"
import Crankshaft 1.0

Page {
    id: root
    
    property var wsClient: null
    property var stack: null
    
    background: Rectangle {
        color: Theme.background
    }
    
    // Command response timeout
    Timer {
        id: commandTimeout
        interval: 5000
        onTriggered: {
            statusMessage.text = Strings.mediaCommandTimeout
            statusMessage.visible = true
        }
    }
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 0
        spacing: 0
        
        // Header bar
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 80
            color: Theme.surface
            
            RowLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 16
                
                AppButton {
                    text: "‹"
                    implicitWidth: 76
                    implicitHeight: 76
                    onClicked: {
                        if (stack) stack.pop()
                    }
                }
                
                Text {
                    text: Strings.mediaTitle
                    font.pixelSize: 32
                    font.bold: true
                    color: Theme.textPrimary
                    Layout.fillWidth: true
                }
            }
        }
        
        // Now Playing Info Section
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 200
            color: Theme.surface
            
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 24
                spacing: 16
                
                // Album art placeholder
                Rectangle {
                    Layout.alignment: Qt.AlignHCenter
                    width: 120
                    height: 120
                    color: Theme.surfaceVariant
                    radius: 4
                    
                    Text {
                        anchors.centerIn: parent
                        text: "🎵"
                        font.pixelSize: 48
                    }
                }
                
                // Media Info
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 8
                    
                    Text {
                        id: mediaTitle
                        text: Strings.mediaNoSelection
                        font.pixelSize: 24
                        font.bold: true
                        color: Theme.textPrimary
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                    }
                    
                    Text {
                        id: mediaArtist
                        text: ""
                        font.pixelSize: 16
                        color: Theme.textSecondary
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                    }
                    
                    Text {
                        id: mediaDuration
                        text: "0:00 / 0:00"
                        font.pixelSize: 14
                        color: Theme.textTertiary
                        Layout.fillWidth: true
                    }
                }
            }
        }
        
        // Progress Bar
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 80
            color: Theme.background
            
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 24
                spacing: 12
                
                // Slider
                Slider {
                    id: progressSlider
                    Layout.fillWidth: true
                    from: 0
                    to: 1000
                    value: 0
                    stepSize: 10
                    
                    background: Rectangle {
                        x: progressSlider.leftPadding
                        y: progressSlider.topPadding + progressSlider.availableHeight / 2 - height / 2
                        implicitWidth: 200
                        implicitHeight: 4
                        width: progressSlider.availableWidth
                        height: implicitHeight
                        radius: 2
                        color: Theme.surfaceVariant
                        
                        Rectangle {
                            width: progressSlider.visualPosition * parent.width
                            height: parent.height
                            color: Theme.primary
                            radius: 2
                        }
                    }
                    
                    handle: Rectangle {
                        x: progressSlider.leftPadding + progressSlider.visualPosition *
                           (progressSlider.availableWidth - width)
                        y: progressSlider.topPadding + progressSlider.availableHeight / 2 - height / 2
                        implicitWidth: 24
                        implicitHeight: 24
                        radius: 12
                        color: Theme.primary
                    }
                    
                    onMoved: {
                        if (wsClient) {
                            const position = value * (mediaDuration.text.split(" / ")[1] || "0");
                            // Seek command will be sent via WebSocket
                            const durationText = mediaDuration.text.split(" / ")[1] || "1";
                            const maxPosition = timeToMs(durationText);
                            const seekMs = (value / 1000) * maxPosition;
                            
                            wsClient.sendCommand("media.seek", {
                                "position_ms": Math.floor(seekMs)
                            });
                        }
                    }
                }
                
                // Time labels
                RowLayout {
                    Layout.fillWidth: true
                    
                    Text {
                        id: currentTime
                        text: "0:00"
                        font.pixelSize: 12
                        color: Theme.textSecondary
                    }
                    
                    Item {
                        Layout.fillWidth: true
                    }
                    
                    Text {
                        text: mediaDuration.text.split(" / ")[1] || "0:00"
                        font.pixelSize: 12
                        color: Theme.textSecondary
                    }
                }
            }
        }
        
        // Playback Controls Section
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 140
            color: Theme.surface
            
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 16
                
                // Main playback controls
                RowLayout {
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignHCenter
                    spacing: 16
                    
                    // Previous button (skip -1)
                    AppButton {
                        text: "⏮"
                        implicitWidth: 80
                        implicitHeight: 80
                        onClicked: {
                            if (wsClient) {
                                wsClient.sendCommand("media.skip", { "direction": -1 });
                                commandTimeout.restart();
                            }
                        }
                    }
                    
                    // Play/Pause button (toggle)
                    AppButton {
                        id: playPauseBtn
                        text: "▶"
                        implicitWidth: 100
                        implicitHeight: 100
                        onClicked: {
                            if (wsClient) {
                                if (mediaState === "playing") {
                                    wsClient.sendCommand("media.pause", {});
                                } else if (mediaState === "paused") {
                                    wsClient.sendCommand("media.resume", {});
                                } else {
                                    wsClient.sendCommand("media.play", { "file": currentMediaFile });
                                }
                                commandTimeout.restart();
                            }
                        }
                    }
                    
                    // Next button (skip +1)
                    AppButton {
                        text: "⏭"
                        implicitWidth: 80
                        implicitHeight: 80
                        onClicked: {
                            if (wsClient) {
                                wsClient.sendCommand("media.skip", { "direction": 1 });
                                commandTimeout.restart();
                            }
                        }
                    }
                }
                
                // Volume control
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 12
                    
                    Text {
                        text: "🔊"
                        font.pixelSize: 20
                    }
                    
                    Slider {
                        id: volumeSlider
                        Layout.fillWidth: true
                        from: 0
                        to: 100
                        value: 80
                        stepSize: 5
                        
                        background: Rectangle {
                            x: volumeSlider.leftPadding
                            y: volumeSlider.topPadding + volumeSlider.availableHeight / 2 - height / 2
                            implicitWidth: 200
                            implicitHeight: 4
                            width: volumeSlider.availableWidth
                            height: implicitHeight
                            radius: 2
                            color: Theme.surfaceVariant
                            
                            Rectangle {
                                width: volumeSlider.visualPosition * parent.width
                                height: parent.height
                                color: Theme.primary
                                radius: 2
                            }
                        }
                        
                        handle: Rectangle {
                            x: volumeSlider.leftPadding + volumeSlider.visualPosition *
                               (volumeSlider.availableWidth - width)
                            y: volumeSlider.topPadding + volumeSlider.availableHeight / 2 - height / 2
                            implicitWidth: 20
                            implicitHeight: 20
                            radius: 10
                            color: Theme.primary
                        }
                        
                        onMoved: {
                            if (wsClient) {
                                wsClient.sendCommand("media.set_volume", {
                                    "level": Math.floor(value)
                                });
                                commandTimeout.restart();
                            }
                        }
                    }
                    
                    Text {
                        text: volumeSlider.value.toFixed(0) + "%"
                        font.pixelSize: 16
                        color: Theme.textSecondary
                        width: 40
                    }
                }
            }
        }
        
        // File Selection
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 80
            color: Theme.background
            
            RowLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 12
                
                AppButton {
                    text: Strings.mediaOpenFile
                    Layout.fillWidth: true
                    Layout.preferredHeight: 60
                    onClicked: {
                        fileDialog.open();
                    }
                }
            }
        }
        
        // Status Message
        Rectangle {
            id: statusMessageBg
            Layout.fillWidth: true
            Layout.preferredHeight: statusMessage.visible ? 60 : 0
            color: Theme.surface
            visible: statusMessage.visible
            
            Text {
                id: statusMessage
                anchors.fill: parent
                anchors.margins: 12
                text: ""
                visible: false
                font.pixelSize: 14
                color: Theme.textSecondary
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.WordWrap
                
                Timer {
                    id: statusMessageTimer
                    interval: 3000
                    onTriggered: statusMessage.visible = false
                }
            }
        }
        
        // Spacer
        Item {
            Layout.fillHeight: true
        }
    }
    
    // File dialog
    FolderDialog {
        id: fileDialog
        onAccepted: {
            const selectedPath = fileDialog.selectedFolder.toString().replace("file://", "");
            currentMediaFile = selectedPath;
            
            if (wsClient) {
                wsClient.sendCommand("media.play", { "file": selectedPath });
                commandTimeout.restart();
            }
        }
    }
    
    // State tracking
    property string mediaState: "stopped"
    property string currentMediaFile: ""
    
    // Helper: convert time string to milliseconds
    function timeToMs(timeStr) {
        const parts = timeStr.split(":");
        if (parts.length === 3) {
            // hours:minutes:seconds
            return (parseInt(parts[0]) * 3600 + parseInt(parts[1]) * 60 + parseInt(parts[2])) *
                   1000;
        } else if (parts.length === 2) {
            // minutes:seconds
            return (parseInt(parts[0]) * 60 + parseInt(parts[1])) * 1000;
        }
        return 0;
    }
    
    // Helper: convert milliseconds to time string
    function msToTime(ms) {
        const totalSeconds = Math.floor(ms / 1000);
        const minutes = Math.floor(totalSeconds / 60);
        const seconds = totalSeconds % 60;
        return minutes + ":" + (seconds < 10 ? "0" : "") + seconds;
    }
    
    // WebSocket event handlers
    Component.onCompleted: {
        if (wsClient) {
            // Subscribe to media state updates
            wsClient.subscribe("media/status/state-changed");
            wsClient.subscribe("media/status/media-info");
            wsClient.subscribe("media/status/position");
            wsClient.subscribe("media/status/volume");
            
            // Handle events
            wsClient.onEvent.connect(function(topic, payload) {
                if (topic === "media/status/state-changed") {
                    mediaState = payload.state || "stopped";
                    playPauseBtn.text = mediaState === "playing" ? "⏸" : "▶";
                    statusMessage.text = Strings.mediaStateChanged.replace("%1", mediaState);
                    statusMessage.visible = true;
                    statusMessageTimer.restart();
                } else if (topic === "media/status/media-info") {
                    mediaTitle.text = payload.title || Strings.mediaNoSelection;
                    mediaArtist.text = payload.artist || "";
                    if (payload.duration && payload.position !== undefined) {
                        mediaDuration.text = msToTime(payload.position) + " / " + msToTime(payload.duration);
                        currentTime.text = msToTime(payload.position);
                        
                        // Update progress slider
                        if (payload.duration > 0) {
                            progressSlider.value = (payload.position / payload.duration) * 1000;
                        }
                    }
                } else if (topic === "media/status/position") {
                    if (mediaDuration.text) {
                        const duration = timeToMs(mediaDuration.text.split(" / ")[1] || "0");
                        currentTime.text = msToTime(payload.position_ms);
                        if (duration > 0) {
                            progressSlider.value = (payload.position_ms / duration) * 1000;
                        }
                    }
                } else if (topic === "media/status/volume") {
                    volumeSlider.value = payload.level || 80;
                }
            });
        }
    }
}
