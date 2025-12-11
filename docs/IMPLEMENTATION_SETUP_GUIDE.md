# Multimedia, Wireless & Android Auto - Implementation Setup Guide

## Quick Start

This guide explains how to set up and use the newly integrated multimedia, wireless, and Android Auto features in Crankshaft MVP.

## File Structure Overview

```
core/
├── hal/
│   ├── CMakeLists.txt (HAL module configuration)
│   ├── multimedia/
│   │   ├── AudioDevice.h (Audio input/output interfaces)
│   │   ├── AudioManager.h (Audio system management)
│   │   ├── VideoDevice.h (Video input/output interfaces)
│   │   ├── VideoManager.h (Display management)
│   │   └── MediaPipeline.h (GStreamer pipeline abstraction)
│   ├── wireless/
│   │   ├── WiFiManager.h (WiFi network management)
│   │   ├── BluetoothManager.h (Bluetooth device management)
│   │   └── NetworkService.h (Network connectivity service)
│   ├── functional/ (Actual implementations with GStreamer, DBus, etc.)
│   │   ├── CMakeLists.txt
│   │   ├── AudioManagerImpl.cpp
│   │   ├── VideoManagerImpl.cpp
│   │   ├── WiFiManagerImpl.cpp
│   │   ├── BluetoothManagerImpl.cpp
│   │   └── NetworkServiceImpl.cpp
│   └── mocks/ (Mock implementations for testing)
├── services/
│   └── android_auto/
│       ├── AndroidAutoService.h (Android Auto service interface)
│       ├── AndroidAutoService.cpp (Service implementation)
│       └── CMakeLists.txt

ui/
├── qml/
│   ├── AndroidAutoScreen.qml (Android Auto projection UI)
│   ├── AudioSettingsScreen.qml (Audio configuration)
│   ├── VideoSettingsScreen.qml (Video configuration)
│   └── ConnectivitySettingsScreen.qml (WiFi/Bluetooth configuration)

docs/
└── MULTIMEDIA_WIRELESS_ANDROIDAUTO.md (Detailed documentation)

third_party/
└── aasdk/ (AASDK library - added as Git submodule)
```

## Building the Project

### 1. Install Dependencies

**Ubuntu/Raspberry Pi OS**:
```bash
# GStreamer development libraries
sudo apt-get install \
    gstreamer1.0-dev \
    gstreamer1.0-plugins-base-dev \
    gstreamer1.0-plugins-good \
    gstreamer1.0-plugins-bad \
    gstreamer1.0-plugins-ugly \
    libgstreamer-plugins-base1.0-dev

# ALSA (audio)
sudo apt-get install libasound2-dev

# DBus (for WiFi/Bluetooth)
sudo apt-get install libdbus-1-dev

# libusb (for Android Auto/AASDK)
sudo apt-get install libusb-1.0-0-dev

# Qt6 (already required)
sudo apt-get install qt6-base-dev qt6-declarative-dev
```

### 2. Initialize AASDK Submodule

```bash
cd crankshaft-mvp
git submodule update --init --recursive third_party/aasdk
```

### 3. Build with CMake

```bash
# Configure Debug build with multimedia support
cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Debug \
    -DBUILD_TESTS=ON \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Build
cmake --build build -j$(nproc)
```

## Usage

### Audio Settings Example

```cpp
// Get audio manager instance
auto audio_mgr = std::make_shared<AudioManagerImpl>();
audio_mgr->initialise();

// Enumerate devices
QVector<QString> outputs = audio_mgr->getOutputDevices();
// Output: ["Speaker", "Headphone", "Bluetooth"]

// Control volume
audio_mgr->setMasterVolume(75);
audio_mgr->setStreamVolume("MUSIC", 85);
audio_mgr->setStreamVolume("NAVIGATION", 90);

// Switch audio route
audio_mgr->setAudioRoute(AudioManager::AudioRoute::BLUETOOTH);
```

### Video Settings Example

```cpp
// Get video manager instance
auto video_mgr = std::make_shared<VideoManagerImpl>();
video_mgr->initialise();

// Configure display
video_mgr->setResolution(QSize(1920, 1080));
video_mgr->setRefreshRate(60);

// Adjust brightness
video_mgr->setBrightness(85);
video_mgr->setContrast(50);

// Enable night mode
video_mgr->setNightModeEnabled(true);
video_mgr->setColourTemperature(3500);  // Warm colour temperature
```

### WiFi Connection Example

```cpp
// Get WiFi manager
auto wifi_mgr = std::make_shared<WiFiManagerImpl>();
wifi_mgr->initialise();

// Scan for networks
wifi_mgr->startScan();

// Connect to network
wifi_mgr->connect("MyNetwork", "password123", WiFiManager::Security::WPA2);

// Monitor connection
QObject::connect(wifi_mgr.get(), &WiFiManager::connectionStateChanged,
    [](WiFiManager::ConnectionState state) {
        if (state == WiFiManager::ConnectionState::CONNECTED) {
            qDebug() << "Connected!";
        }
    });
```

### Bluetooth Connection Example

```cpp
// Get Bluetooth manager
auto bt_mgr = std::make_shared<BluetoothManagerImpl>();
bt_mgr->initialise();

// Start discovery
bt_mgr->startDiscovery();

// Get discovered devices
auto devices = bt_mgr->getDiscoveredDevices();

// Connect and pair
bt_mgr->pair(devices[0].address);
bt_mgr->connect(devices[0].address);

// Connect audio profile
bt_mgr->connectAudio(devices[0].address, 
    BluetoothManager::AudioProfile::A2DP);
```

### Android Auto Integration Example

```cpp
// Get Android Auto service
auto aa_service = std::make_shared<AndroidAutoServiceImpl>();
aa_service->initialise();

// Start searching for devices
aa_service->startSearching();

// Connect to device when found
QObject::connect(aa_service.get(), 
    QOverload<const AndroidAutoService::AndroidDevice&>::of(
        &AndroidAutoService::deviceFound),
    [aa_service](const AndroidAutoService::AndroidDevice& device) {
        qDebug() << "Found device:" << device.model;
        aa_service->connectToDevice(device.serial);
    });

// Handle video frames
QObject::connect(aa_service.get(), 
    QOverload<int, int, const uint8_t*, int>::of(
        &AndroidAutoService::videoFrameReady),
    [](int width, int height, const uint8_t* data, int size) {
        qDebug() << "Video frame:" << width << "x" << height;
        // Render frame to display
    });
```

## Event Bus Integration

The UI communicates with the core backend via WebSocket events published to the EventBus.

### Audio Events

Publish from UI to core:
```json
{
  "topic": "audio/volume",
  "payload": { "master": 75 }
}
```

Response from core:
```json
{
  "topic": "audio/status",
  "payload": { 
    "volume": 75, 
    "route": "SPEAKER",
    "muted": false 
  }
}
```

### Video Events

```json
{
  "topic": "video/brightness",
  "payload": { "value": 80 }
}
```

### Wireless Events

```json
{
  "topic": "wifi/connect",
  "payload": { "ssid": "MyNetwork", "password": "pass" }
}

{
  "topic": "bluetooth/connect",
  "payload": { "address": "AA:BB:CC:DD:EE:FF" }
}
```

### Android Auto Events

```json
{
  "topic": "androidauto/touch",
  "payload": { "x": 0.5, "y": 0.3, "action": "down" }
}

{
  "topic": "androidauto/key",
  "payload": { "key": "BACK" }
}

{
  "topic": "androidauto/status",
  "payload": { 
    "connected": true,
    "device": "Pixel 6",
    "fps": 30,
    "latency": 45
  }
}
```

## GStreamer Pipeline Examples

### Playing Audio from File

```
filesrc → decodebin → audioconvert → alsasink
```

### Decoding and Playing H.264 Video

```
filesrc → h264parse → nvv4l2decoder → videoscale → qtglwindow
```

### Android Auto Video Reception & Display

```
USB (AASDK) → h264parse → nvv4l2decoder → videoscale → qtwindow
```

### Audio Mixing (Multiple Sources)

```
     ┌─→ audiopanorama ─┐
     │                  ├→ audiomixer → audioconvert → alsasink
     └─→ audiopanorama ─┘
```

## Testing

### Running Unit Tests

```bash
# Run all tests
ctest --test-dir build --output-on-failure

# Run specific test
ctest --test-dir build -R "audio" --output-on-failure
```

### Manual Testing Checklist

**Audio**:
- [ ] Master volume slider changes audio level
- [ ] Stream volume (music/nav/call) works independently
- [ ] Mute/unmute toggle functions
- [ ] Audio route switching (speaker → headphone → Bluetooth)

**Video**:
- [ ] Resolution changes apply correctly
- [ ] Refresh rate adjustment works
- [ ] Brightness/contrast sliders function
- [ ] Night mode toggle and colour temperature adjustment work

**WiFi**:
- [ ] WiFi enable/disable toggle
- [ ] Network scanning shows available networks
- [ ] Connection to network succeeds
- [ ] Signal strength display updates
- [ ] Saved networks list displays

**Bluetooth**:
- [ ] Bluetooth enable/disable toggle
- [ ] Device discovery finds paired devices
- [ ] Device connection/disconnection works
- [ ] Audio profile (A2DP, HFP) connection succeeds
- [ ] Connected devices list updates

**Android Auto**:
- [ ] USB connection detection works
- [ ] Video frame rendering on screen
- [ ] Touch input translates to device coordinates
- [ ] Navigation buttons (back, home) function
- [ ] FPS and latency display updates
- [ ] Disconnection cleanups properly

## Troubleshooting

### GStreamer Not Found

```bash
# Check GStreamer installation
gst-inspect-1.0 | head -20

# Set PKG_CONFIG_PATH if needed
export PKG_CONFIG_PATH=/usr/lib/pkgconfig:/usr/local/lib/pkgconfig
```

### AASDK Not Compiling

```bash
# Ensure libusb-dev is installed
sudo apt-get install libusb-1.0-0-dev

# Check AASDK submodule
cd third_party/aasdk && git status
```

### DBus Connection Issues

```bash
# Start DBus session
sudo service dbus start

# Check services available
dbus-send --print-reply --system /

# View DBus logs
journalctl -u dbus -f
```

### Audio No Output

```bash
# Check ALSA configuration
alsamixer

# List audio devices
aplay -l
pactl list sinks

# Debug GStreamer audio
GST_DEBUG=audiosink:5 ./build/CrankshaftReborn
```

### Video Decoding Issues

```bash
# Check available video decoders
gst-inspect-1.0 nvv4l2decoder
gst-inspect-1.0 v4l2h264dec

# Test video pipeline
gst-launch-1.0 filesrc location=test.h264 ! h264parse ! nvv4l2decoder ! videoscale ! videoconvert ! autovideosink
```

## Next Steps

1. **Implement GStreamer Pipelines**: Replace placeholder implementations in `core/hal/functional/` with actual GStreamer pipelines
2. **DBus Integration**: Implement WiFi/Bluetooth managers using D-Bus calls to NetworkManager and BlueZ
3. **AASDK Full Integration**: Complete Android Auto service with full USB device handling and protocol implementation
4. **Performance Optimisation**: Profile and optimise hot paths, especially video decoding and rendering
5. **Error Handling**: Add comprehensive error recovery and user feedback
6. **Documentation**: Extend API documentation with code examples
7. **Testing**: Write comprehensive unit and integration tests

## References

- **GStreamer**: https://gstreamer.freedesktop.org/documentation/
- **AASDK**: https://github.com/opencardev/aasdk
- **Qt6 QML**: https://doc.qt.io/qt-6/
- **DBus**: https://dbus.freedesktop.org/doc/dbus-daemon.1.html
- **BlueZ**: http://www.bluez.org/
- **NetworkManager**: https://networkmanager.dev/
- **Raspberry Pi**: https://www.raspberrypi.org/documentation/
