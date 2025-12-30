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

pragma Singleton
import QtQuick
import Crankshaft 1.0

QtObject {
    id: androidAutoStatus

    // Persistent Android Auto status shared across screens
    property string statusText: qsTr('Android Auto - Disconnected')
    property color statusColor: '#F44336'
    property bool isConnected: false

    Component.onCompleted: {
        console.log('[AndroidAutoStatus] Initialising; subscribing to android-auto/status/#');
        if (wsClient && wsClient.subscribe) {
            wsClient.subscribe('android-auto/status/#');
        }
    }

    Connections {
        target: wsClient

        function onConnectedChanged() {
            if (wsClient.connected) {
                console.log('[AndroidAutoStatus] WebSocket connected; re-subscribing');
                wsClient.subscribe('android-auto/status/#');
            }
        }

        function onEventReceived(topic, payload) {
            // Update shared status based on backend events
            if (topic === 'android-auto/status/state-changed') {
                let stateName = payload.stateName || 'UNKNOWN';
                console.log('[AndroidAutoStatus] State changed to:', stateName);
                androidAutoStatus.statusText = 'Android Auto - ' + stateName;

                if (stateName === 'CONNECTED') {
                    androidAutoStatus.statusColor = '#4CAF50';
                    androidAutoStatus.isConnected = true;
                } else if (stateName === 'DISCONNECTED') {
                    androidAutoStatus.statusColor = '#F44336';
                    androidAutoStatus.isConnected = false;
                } else {
                    androidAutoStatus.statusColor = '#FF9800';
                    androidAutoStatus.isConnected = false;
                }
            } else if (topic === 'android-auto/status/connected') {
                let device = payload.device;
                console.log('[AndroidAutoStatus] Connected, device:', device);
                let label = device && device.model ? device.model : 'Device';
                androidAutoStatus.statusText = 'Android Auto - CONNECTED (' + label + ')';
                androidAutoStatus.statusColor = '#4CAF50';
                androidAutoStatus.isConnected = true;
            } else if (topic === 'android-auto/status/disconnected') {
                console.log('[AndroidAutoStatus] Disconnected');
                androidAutoStatus.statusText = 'Android Auto - Disconnected';
                androidAutoStatus.statusColor = '#F44336';
                androidAutoStatus.isConnected = false;
            } else if (topic === 'android-auto/status/error') {
                console.log('[AndroidAutoStatus] Error:', payload.error);
                androidAutoStatus.statusText = 'Android Auto - Error: ' + payload.error;
                androidAutoStatus.statusColor = '#F44336';
                androidAutoStatus.isConnected = false;
            }
        }
    }
}
