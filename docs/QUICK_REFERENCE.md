# Quick Reference Guide

## File Locations Summary

| Component | Header | Implementation | QML |
|-----------|--------|-----------------|-----|
| **Audio** | `core/hal/multimedia/AudioDevice.h`, `AudioManager.h` | `core/hal/functional/AudioManagerImpl.cpp` | `ui/qml/AudioSettingsScreen.qml` |
| **Video** | `core/hal/multimedia/VideoDevice.h`, `VideoManager.h` | `core/hal/functional/VideoManagerImpl.cpp` | `ui/qml/VideoSettingsScreen.qml` |
| **Media Pipeline** | `core/hal/multimedia/MediaPipeline.h` | `core/hal/functional/MediaPipelineImpl.cpp` | — |
| **WiFi** | `core/hal/wireless/WiFiManager.h` | `core/hal/functional/WiFiManagerImpl.cpp` | `ui/qml/ConnectivitySettingsScreen.qml` |
| **Bluetooth** | `core/hal/wireless/BluetoothManager.h` | `core/hal/functional/BluetoothManagerImpl.cpp` | `ui/qml/ConnectivitySettingsScreen.qml` |
| **Network** | `core/hal/wireless/NetworkService.h` | `core/hal/functional/NetworkServiceImpl.cpp` | — |
| **Android Auto** | `core/services/android_auto/AndroidAutoService.h` | `core/services/android_auto/AndroidAutoService.cpp` | `ui/qml/AndroidAutoScreen.qml` |

## Key Classes

### Audio System
```cpp
AudioManager          // Main audio manager
AudioOutputDevice     // Speaker/headphone/Bluetooth output
AudioInputDevice      // Microphone input
AudioDevice::AudioConfig
AudioManager::AudioRoute { SPEAKER, HEADPHONE, BLUETOOTH, USB }
```

### Video System
```cpp
VideoManager          // Main video manager
VideoOutputDevice     // Display output
VideoInputDevice      // Camera input
VideoManager::DisplayMode { FULLSCREEN, WINDOWED, EXTENDED, MIRROR }
```

### Wireless System
```cpp
WiFiManager           // WiFi networks
BluetoothManager      // Bluetooth devices
NetworkService        // Network utilities
WiFiManager::Security { NONE, WEP, WPA, WPA2, WPA3 }
BluetoothManager::AudioProfile { A2DP, HFP, HSP, AVRCP }
```

### Android Auto
```cpp
AndroidAutoService    // Android Auto projection
AndroidAutoService::ConnectionState { DISCONNECTED, SEARCHING, CONNECTING, CONNECTED, ERROR }
```

## Common Operations

### Audio
```cpp
// Volume control
audio_mgr->setMasterVolume(75);
audio_mgr->setStreamVolume("MUSIC", 85);
audio_mgr->setMuted(true);

// Device switching
audio_mgr->setAudioRoute(AudioManager::AudioRoute::BLUETOOTH);

// Device enumeration
auto devices = audio_mgr->getOutputDevices();
```

### Video
```cpp
// Display configuration
video_mgr->setResolution(QSize(1920, 1080));
video_mgr->setRefreshRate(60);

// Adjustment
video_mgr->setBrightness(85);
video_mgr->setContrast(50);

// Night mode
video_mgr->setNightModeEnabled(true);
video_mgr->setColourTemperature(3500);
```

### WiFi
```cpp
// Scanning
wifi_mgr->startScan();
auto networks = wifi_mgr->getAvailableNetworks();

// Connection
wifi_mgr->connect("SSID", "password", WiFiManager::Security::WPA2);

// Info
int signal = wifi_mgr->getSignalStrength();
QString ip = wifi_mgr->getIPAddress();
```

### Bluetooth
```cpp
// Discovery
bt_mgr->startDiscovery();
auto devices = bt_mgr->getDiscoveredDevices();

// Pairing
bt_mgr->pair("AA:BB:CC:DD:EE:FF");
bt_mgr->connect("AA:BB:CC:DD:EE:FF");

// Audio
bt_mgr->connectAudio("AA:BB:CC:DD:EE:FF", 
    BluetoothManager::AudioProfile::A2DP);
```

### Android Auto
```cpp
// Connection
aa_service->startSearching();
aa_service->connectToDevice(device.serial);

// Input
aa_service->sendTouchInput(x, y, action);
aa_service->sendKeyInput(keycode, action);

// Configuration
aa_service->setDisplayResolution(QSize(1024, 600));
aa_service->setFramerate(30);
```

## Signal Connections

### Audio Signals
```cpp
masterVolumeChanged(int percent)
streamVolumeChanged(QString type, int percent)
muteStateChanged(bool muted)
audioRouteChanged(AudioRoute route)
errorOccurred(QString error)
```

### Video Signals
```cpp
resolutionChanged(QSize resolution)
refreshRateChanged(int hz)
brightnessChanged(int percent)
contrastChanged(int percent)
nightModeChanged(bool enabled)
errorOccurred(QString error)
```

### WiFi Signals
```cpp
connectionStateChanged(ConnectionState state)
enabledChanged(bool enabled)
networksUpdated(QVector<WiFiNetwork> networks)
signalStrengthChanged(int percent)
errorOccurred(QString error)
```

### Bluetooth Signals
```cpp
stateChanged(ConnectionState state)
enabledChanged(bool enabled)
deviceDiscovered(BluetoothDevice device)
deviceConnected(QString address)
deviceDisconnected(QString address)
audioConnected(QString address, AudioProfile profile)
errorOccurred(QString error)
```

### Android Auto Signals
```cpp
connectionStateChanged(ConnectionState state)
connected(AndroidDevice device)
disconnected()
videoFrameReady(int w, int h, const uint8_t* data, int size)
audioDataReady(QByteArray data)
statsUpdated(int fps, int latency, int dropped_frames)
errorOccurred(QString error)
```

## Event Bus Topics

| Topic | Payload | Direction |
|-------|---------|-----------|
| `audio/volume` | `{"master": int}` | UI→Core |
| `audio/route` | `{"route": string}` | UI→Core |
| `audio/mute` | `{"enabled": bool}` | UI→Core |
| `video/resolution` | `{"resolution": string}` | UI→Core |
| `video/brightness` | `{"value": int}` | UI→Core |
| `video/night-mode` | `{"enabled": bool, "kelvin": int}` | UI→Core |
| `wifi/enabled` | `{"enabled": bool}` | UI→Core |
| `wifi/connect` | `{"ssid": string, "password": string}` | UI→Core |
| `bluetooth/enabled` | `{"enabled": bool}` | UI→Core |
| `bluetooth/connect` | `{"address": string}` | UI→Core |
| `androidauto/touch` | `{"x": float, "y": float, "action": string}` | UI→Core |
| `androidauto/key` | `{"key": string}` | UI→Core |
| `androidauto/status` | `{}` | UI→Core |

## Dependency Installation

### Ubuntu/Raspberry Pi OS
```bash
# GStreamer
sudo apt-get install gstreamer1.0-dev gstreamer1.0-plugins-base-dev \
    gstreamer1.0-plugins-good gstreamer1.0-plugins-bad libasound2-dev

# DBus
sudo apt-get install libdbus-1-dev

# USB (AASDK)
sudo apt-get install libusb-1.0-0-dev

# Qt6 (already required)
sudo apt-get install qt6-base-dev qt6-declarative-dev
```

## Building

```bash
# Clone with submodules
git clone --recurse-submodules <url>

# Configure
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON

# Build
cmake --build build -j$(nproc)

# Run
./build/core/crankshaft-core
./build/ui/crankshaft-ui  # In another terminal
```

## Troubleshooting Checklist

- [ ] All dependencies installed (`gstreamer1.0-dev`, `libdbus-1-dev`, `libusb-1.0-0-dev`)
- [ ] AASDK submodule initialized (`git submodule update --init`)
- [ ] CMake finds GStreamer (`pkg-config --cflags --libs gstreamer-1.0`)
- [ ] Qt6 is available (`qt6-base-dev` installed)
- [ ] Build succeeds without errors
- [ ] Tests pass (`ctest --test-dir build`)
- [ ] Audio device detected (`aplay -l`)
- [ ] WiFi/Bluetooth available on target hardware
- [ ] USB ADB enabled for Android Auto testing

## Documentation Files

| File | Purpose |
|------|---------|
| `docs/IMPLEMENTATION_SUMMARY.md` | Overview and status |
| `docs/MULTIMEDIA_WIRELESS_ANDROIDAUTO.md` | Detailed architecture |
| `docs/IMPLEMENTATION_SETUP_GUIDE.md` | Setup and integration guide |
| `docs/API_USAGE_EXAMPLES.md` | Code examples and patterns |
| `docs/QUICK_REFERENCE.md` | This file |

## Next Steps

1. **Review**: Read `IMPLEMENTATION_SUMMARY.md` for overview
2. **Setup**: Follow `IMPLEMENTATION_SETUP_GUIDE.md`
3. **Code**: Check `API_USAGE_EXAMPLES.md` for patterns
4. **Implement**: Replace placeholder code in `*Impl.cpp` files
5. **Test**: Write unit tests and integration tests
6. **Deploy**: Build and test on Raspberry Pi 4

## Contact & Support

For questions about the implementation:
1. Check `docs/MULTIMEDIA_WIRELESS_ANDROIDAUTO.md`
2. Review code examples in `docs/API_USAGE_EXAMPLES.md`
3. Check placeholder implementations in `core/hal/functional/`
4. Review test files in `tests/`

---

**Last Updated**: December 6, 2025  
**Status**: ✅ Ready for Implementation
