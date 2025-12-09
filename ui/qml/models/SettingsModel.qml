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

// Settings data model - extensions can append to this
QtObject {
    id: settingsModel
    
    // Add your settings categories here
    property var categories: [
        {
            id: "appearance",
            name: "Appearance",
            icon: "mdi-palette",
            settings: [
                {
                    key: "ui.theme.dark",
                    label: "Dark Mode",
                    description: "Use dark theme for the interface",
                    type: "toggle",
                    value: Theme.isDark,
                    onChange: function(val) {
                        Theme.isDark = val
                        wsClient.publish("ui/theme/changed", {
                            "mode": val ? "dark" : "light"
                        })
                    }
                }
            ]
        },
        {
            id: "language",
            name: "Language",
            icon: "mdi-earth",
            settings: [
                {
                    key: "ui.language",
                    label: "Interface Language",
                    description: "Choose the language for the user interface",
                    type: "select",
                    value: currentLanguage || "en-GB",
                    options: [
                        { value: "en-GB", label: "English (UK)" },
                        { value: "de-DE", label: "Deutsch" }
                    ],
                    onChange: function(val) {
                        wsClient.publish("ui/language/changed", {
                            "language": val
                        })
                    }
                }
            ]
        },
        {
            id: "system",
            name: "System",
            icon: "mdi-cog",
            settings: [
                {
                    key: "system.connection",
                    label: "WebSocket Status",
                    description: wsClient.connected ? "Connected to backend" : "Disconnected from backend",
                    type: "info",
                    value: wsClient.connected ? "Connected" : "Disconnected"
                }
            ]
        },
        {
            id: "audio",
            name: "Audio",
            icon: "mdi-volume-high",
            settings: [
                {
                    key: "audio.volume.master",
                    label: "Master Volume",
                    description: "Main volume control for all audio",
                    type: "slider",
                    value: 70,
                    min: 0,
                    max: 100,
                    step: 1,
                    onChange: function(val) {
                        wsClient.publish("audio/volume", { "master": Math.round(val) })
                    }
                },
                {
                    key: "audio.mute",
                    label: "Mute",
                    description: "Mute all audio output",
                    type: "toggle",
                    value: false,
                    onChange: function(val) {
                        wsClient.publish("audio/mute", { "enabled": val })
                    }
                },
                {
                    key: "audio.route",
                    label: "Audio Output",
                    description: "Select audio output device",
                    type: "select",
                    value: "SPEAKER",
                    options: [
                        { value: "SPEAKER", label: "Speaker" },
                        { value: "HEADPHONE", label: "Headphone" },
                        { value: "BLUETOOTH", label: "Bluetooth" },
                        { value: "USB", label: "USB" }
                    ],
                    onChange: function(val) {
                        wsClient.publish("audio/route", { "route": val })
                    }
                },
                {
                    key: "audio.stream.music",
                    label: "Music Volume",
                    description: "Volume for music playback",
                    type: "slider",
                    value: 85,
                    min: 0,
                    max: 100,
                    step: 1,
                    onChange: function(val) {
                        wsClient.publish("audio/stream-volume", { "type": "MUSIC", "volume": Math.round(val) })
                    }
                },
                {
                    key: "audio.stream.navigation",
                    label: "Navigation Volume",
                    description: "Volume for navigation guidance",
                    type: "slider",
                    value: 90,
                    min: 0,
                    max: 100,
                    step: 1,
                    onChange: function(val) {
                        wsClient.publish("audio/stream-volume", { "type": "NAVIGATION", "volume": Math.round(val) })
                    }
                },
                {
                    key: "audio.stream.call",
                    label: "Call Volume",
                    description: "Volume for phone calls",
                    type: "slider",
                    value: 100,
                    min: 0,
                    max: 100,
                    step: 1,
                    onChange: function(val) {
                        wsClient.publish("audio/stream-volume", { "type": "CALL", "volume": Math.round(val) })
                    }
                }
            ]
        },
        {
            id: "androidauto",
            name: "Android Auto",
            icon: "mdi-android-auto",
            settings: [
                {
                    key: "connectivity.android-auto.enabled",
                    label: "Android Auto",
                    description: "Enable Android Auto projection",
                    type: "toggle",
                    value: true,
                    onChange: function(val) {
                        wsClient.publish("android-auto/enabled", { "enabled": val })
                    }
                },
                {
                    key: "connectivity.android-auto.autoconnect",
                    label: "Auto-connect on USB",
                    description: "Automatically connect when USB device is detected",
                    type: "toggle",
                    value: true,
                    onChange: function(val) {
                        wsClient.publish("android-auto/autoconnect", { "enabled": val })
                    }
                },
                {
                    key: "connectivity.android-auto.wireless",
                    label: "Enable Wireless Android Auto",
                    description: "Allow wireless connection over Bluetooth",
                    type: "toggle",
                    value: false,
                    onChange: function(val) {
                        wsClient.publish("android-auto/wireless", { "enabled": val })
                    }
                },
                {
                    key: "connectivity.android-auto.resolution",
                    label: "Video Resolution",
                    description: "Choose the video resolution for display",
                    type: "select",
                    value: "1024x600",
                    options: [
                        { value: "800x480", label: "800x480" },
                        { value: "1024x600", label: "1024x600" },
                        { value: "1280x720", label: "1280x720" },
                        { value: "1920x1080", label: "1920x1080" }
                    ],
                    onChange: function(val) {
                        wsClient.publish("android-auto/resolution", { "resolution": val })
                    }
                },
                {
                    key: "connectivity.android-auto.framerate",
                    label: "Video Framerate",
                    description: "Set the video framerate (fps)",
                    type: "select",
                    value: "30",
                    options: [
                        { value: "15", label: "15 FPS" },
                        { value: "30", label: "30 FPS" },
                        { value: "60", label: "60 FPS" }
                    ],
                    onChange: function(val) {
                        wsClient.publish("android-auto/fps", { "value": parseInt(val) })
                    }
                },
                {
                    key: "connectivity.android-auto.video-channel",
                    label: "Video Channel",
                    description: "Display projection from Android device",
                    type: "toggle",
                    value: true,
                    onChange: function(val) {
                        wsClient.publish("android-auto/channels/video", { "enabled": val })
                    }
                },
                {
                    key: "connectivity.android-auto.media-audio",
                    label: "Media Audio Channel",
                    description: "Music and media playback audio",
                    type: "toggle",
                    value: true,
                    onChange: function(val) {
                        wsClient.publish("android-auto/channels/media-audio", { "enabled": val })
                    }
                },
                {
                    key: "connectivity.android-auto.system-audio",
                    label: "System Audio Channel",
                    description: "System sounds and notifications",
                    type: "toggle",
                    value: true,
                    onChange: function(val) {
                        wsClient.publish("android-auto/channels/system-audio", { "enabled": val })
                    }
                },
                {
                    key: "connectivity.android-auto.speech-audio",
                    label: "Speech Audio Channel",
                    description: "Navigation guidance and voice assistant",
                    type: "toggle",
                    value: true,
                    onChange: function(val) {
                        wsClient.publish("android-auto/channels/speech-audio", { "enabled": val })
                    }
                },
                {
                    key: "connectivity.android-auto.microphone",
                    label: "Microphone Channel",
                    description: "Voice commands and phone calls",
                    type: "toggle",
                    value: true,
                    onChange: function(val) {
                        wsClient.publish("android-auto/channels/microphone", { "enabled": val })
                    }
                },
                {
                    key: "connectivity.android-auto.input-channel",
                    label: "Input Channel",
                    description: "Touch screen and button inputs",
                    type: "toggle",
                    value: true,
                    onChange: function(val) {
                        wsClient.publish("android-auto/channels/input", { "enabled": val })
                    }
                },
                {
                    key: "connectivity.android-auto.sensor-channel",
                    label: "Sensor Channel",
                    description: "GPS, speed and vehicle sensors",
                    type: "toggle",
                    value: true,
                    onChange: function(val) {
                        wsClient.publish("android-auto/channels/sensor", { "enabled": val })
                    }
                },
                {
                    key: "connectivity.android-auto.bluetooth-channel",
                    label: "Bluetooth Channel",
                    description: "Wireless Android Auto over Bluetooth",
                    type: "toggle",
                    value: false,
                    onChange: function(val) {
                        wsClient.publish("android-auto/channels/bluetooth", { "enabled": val })
                    }
                },
                {
                    key: "connectivity.android-auto.mock-device",
                    label: "Use Mock Device",
                    description: "Use simulated Android Auto for testing",
                    type: "toggle",
                    value: true,
                    onChange: function(val) {
                        wsClient.publish("android-auto/use-mock", { "enabled": val })
                    }
                },
                {
                    key: "connectivity.android-auto.test-video",
                    label: "Generate Test Video",
                    description: "Display test pattern when using mock device",
                    type: "toggle",
                    value: true,
                    onChange: function(val) {
                        wsClient.publish("android-auto/test-video", { "enabled": val })
                    }
                },
                {
                    key: "connectivity.android-auto.test-audio",
                    label: "Generate Test Audio",
                    description: "Play test tone when using mock device",
                    type: "toggle",
                    value: false,
                    onChange: function(val) {
                        wsClient.publish("android-auto/test-audio", { "enabled": val })
                    }
                }
            ]
        },
        {
            id: "connectivity",
            name: "Connectivity",
            icon: "mdi-access-point",
            settings: [
                {
                    key: "connectivity.wifi.enabled",
                    label: "WiFi",
                    description: "Enable WiFi connectivity",
                    type: "toggle",
                    value: false,
                    onChange: function(val) {
                        wsClient.publish("wifi/enabled", { "enabled": val })
                    }
                },
                {
                    key: "connectivity.wifi.page",
                    label: "WiFi Networks",
                    description: "Manage WiFi connections",
                    type: "page",
                    pageComponent: "WiFiSettingsPage"
                },
                {
                    key: "connectivity.bluetooth.enabled",
                    label: "Bluetooth",
                    description: "Enable Bluetooth connectivity",
                    type: "toggle",
                    value: false,
                    onChange: function(val) {
                        wsClient.publish("bluetooth/enabled", { "enabled": val })
                    }
                },
                {
                    key: "connectivity.bluetooth.page",
                    label: "Bluetooth Devices",
                    description: "Manage Bluetooth connections",
                    type: "page",
                    pageComponent: "BluetoothSettingsPage"
                }
            ]
        },
        {
            id: "about",
            name: "About",
            icon: "mdi-information",
            settings: [
                {
                    key: "about.version",
                    label: "Version",
                    description: "Crankshaft version information",
                    type: "info",
                    value: "1.0.0"
                }
            ]
        },
        {
            id: "profiles",
            name: "Device Profiles",
            icon: "mdi-account",
            settings: [
                {
                    key: "profiles.host.active",
                    label: "Active Host Profile",
                    description: "Select the active host configuration profile",
                    type: "select",
                    value: "development",
                    options: [
                        { value: "development", label: "Development Host" },
                        { value: "production", label: "Production Host" },
                        { value: "test", label: "Test Host" }
                    ],
                    onChange: function(val) {
                        wsClient.publish("profiles/host/changed", { "profile": val })
                    }
                },
                {
                    key: "profiles.vehicle.active",
                    label: "Active Vehicle Profile",
                    description: "Select the active vehicle configuration profile",
                    type: "select",
                    value: "sedan",
                    options: [
                        { value: "sedan", label: "Test Vehicle - Sedan" },
                        { value: "suv", label: "Test Vehicle - SUV" },
                        { value: "truck", label: "Test Vehicle - Truck" }
                    ],
                    onChange: function(val) {
                        wsClient.publish("profiles/vehicle/changed", { "profile": val })
                    }
                },
                {
                    key: "profiles.devices.canbus.enabled",
                    label: "CAN Bus",
                    description: "Vehicle data bus interface",
                    type: "toggle",
                    value: true,
                    onChange: function(val) {
                        wsClient.publish("profiles/devices/canbus", { "enabled": val })
                    }
                },
                {
                    key: "profiles.devices.canbus.mock",
                    label: "CAN Bus Mock Mode",
                    description: "Use mock CAN bus data instead of real hardware",
                    type: "toggle",
                    value: true,
                    onChange: function(val) {
                        wsClient.publish("profiles/devices/canbus/mock", { "enabled": val })
                    }
                },
                {
                    key: "profiles.devices.gps.enabled",
                    label: "GPS",
                    description: "Global positioning system",
                    type: "toggle",
                    value: true,
                    onChange: function(val) {
                        wsClient.publish("profiles/devices/gps", { "enabled": val })
                    }
                },
                {
                    key: "profiles.devices.gps.mock",
                    label: "GPS Mock Mode",
                    description: "Use mock GPS data instead of real hardware",
                    type: "toggle",
                    value: true,
                    onChange: function(val) {
                        wsClient.publish("profiles/devices/gps/mock", { "enabled": val })
                    }
                },
                {
                    key: "profiles.devices.bluetooth.enabled",
                    label: "Bluetooth",
                    description: "Wireless connectivity",
                    type: "toggle",
                    value: true,
                    onChange: function(val) {
                        wsClient.publish("profiles/devices/bluetooth", { "enabled": val })
                    }
                },
                {
                    key: "profiles.devices.bluetooth.mock",
                    label: "Bluetooth Mock Mode",
                    description: "Use mock Bluetooth adapter instead of real hardware",
                    type: "toggle",
                    value: true,
                    onChange: function(val) {
                        wsClient.publish("profiles/devices/bluetooth/mock", { "enabled": val })
                    }
                },
                {
                    key: "profiles.devices.wifi.enabled",
                    label: "WiFi",
                    description: "Wireless networking",
                    type: "toggle",
                    value: true,
                    onChange: function(val) {
                        wsClient.publish("profiles/devices/wifi", { "enabled": val })
                    }
                },
                {
                    key: "profiles.devices.wifi.mock",
                    label: "WiFi Mock Mode",
                    description: "Use mock WiFi adapter instead of real hardware",
                    type: "toggle",
                    value: true,
                    onChange: function(val) {
                        wsClient.publish("profiles/devices/wifi/mock", { "enabled": val })
                    }
                },
                {
                    key: "profiles.devices.androidauto.enabled",
                    label: "Android Auto",
                    description: "Android Auto projection",
                    type: "toggle",
                    value: true,
                    onChange: function(val) {
                        wsClient.publish("profiles/devices/androidauto", { "enabled": val })
                    }
                },
                {
                    key: "profiles.devices.androidauto.mock",
                    label: "Android Auto Mock Mode",
                    description: "Use mock Android Auto device instead of real hardware",
                    type: "toggle",
                    value: true,
                    onChange: function(val) {
                        wsClient.publish("profiles/devices/androidauto/mock", { "enabled": val })
                    }
                },
                {
                    key: "profiles.devices.i2c.enabled",
                    label: "I2C Bus",
                    description: "Inter-integrated circuit bus",
                    type: "toggle",
                    value: true,
                    onChange: function(val) {
                        wsClient.publish("profiles/devices/i2c", { "enabled": val })
                    }
                },
                {
                    key: "profiles.devices.i2c.mock",
                    label: "I2C Mock Mode",
                    description: "Use mock I2C bus instead of real hardware",
                    type: "toggle",
                    value: true,
                    onChange: function(val) {
                        wsClient.publish("profiles/devices/i2c/mock", { "enabled": val })
                    }
                },
                {
                    key: "profiles.devices.uart.enabled",
                    label: "UART Serial",
                    description: "Serial communication interface",
                    type: "toggle",
                    value: true,
                    onChange: function(val) {
                        wsClient.publish("profiles/devices/uart", { "enabled": val })
                    }
                },
                {
                    key: "profiles.devices.uart.mock",
                    label: "UART Mock Mode",
                    description: "Use mock UART serial instead of real hardware",
                    type: "toggle",
                    value: true,
                    onChange: function(val) {
                        wsClient.publish("profiles/devices/uart/mock", { "enabled": val })
                    }
                }
            ]
        }
    ]
    
    // Function to add a category dynamically (for extensions)
    function addCategory(category) {
        var cats = categories
        cats.push(category)
        categories = cats
    }
    
    // Function to add settings to an existing category
    function addSettingsToCategory(categoryId, newSettings) {
        var cats = categories
        for (var i = 0; i < cats.length; i++) {
            if (cats[i].id === categoryId) {
                cats[i].settings = cats[i].settings.concat(newSettings)
                categories = cats
                return true
            }
        }
        return false
    }
    
    // Function to get category by ID
    function getCategoryById(id) {
        for (var i = 0; i < categories.length; i++) {
            if (categories[i].id === id) {
                return categories[i]
            }
        }
        return null
    }
}
