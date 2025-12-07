# Multimedia, Wireless & Android Auto Implementation

## Overview

This document describes the implementation of multimedia (audio/video), wireless (WiFi/Bluetooth), and Android Auto features in Crankshaft MVP.

## Architecture

### Multimedia HAL

The multimedia subsystem uses a Hardware Abstraction Layer (HAL) pattern to decouple implementation details from the core application.

#### Audio System

**File**: `core/hal/multimedia/AudioDevice.h`, `AudioManager.h`

The audio system is built around:
- **AudioDevice**: Base class for audio input/output devices
- **AudioOutputDevice**: Speakers, headphones, Bluetooth devices
- **AudioInputDevice**: Microphones
- **AudioManager**: Centralized audio management with device enumeration and routing

Key features:
- Device enumeration (speakers, headphones, Bluetooth, USB)
- Audio routing (direct output switching)
- Volume control per stream type (music, navigation, calls)
- Mute/unmute support

**GStreamer Integration**:
- Audio capture via `alsasrc` (ALSA source)
- Audio playback via `alsasink` (ALSA sink)
- Optional PulseAudio support via `pulsesrc`/`pulsesink`
- Audio encoding via `opus`, `vorbis`, `flac` codecs

#### Video System

**File**: `core/hal/multimedia/VideoDevice.h`, `VideoManager.h`

The video system manages:
- **VideoDevice**: Base class for video input/output
- **VideoOutputDevice**: Display renderer (primary/secondary displays)
- **VideoInputDevice**: Camera input
- **VideoManager**: Display management and global video settings

Key features:
- Multi-display support (primary, secondary, mirror modes)
- Resolution and refresh rate configuration
- Brightness, contrast, night mode (colour temperature)
- Touch input support
- V-Sync control

**GStreamer Integration**:
- Video decoding via `h264parse` + `nvv4l2decoder` (NVIDIA) or `v4l2h264dec`
- Video encoding via `videoscale` + `videorate` for frame rate control
- Display output via Qt6 GStreamer QML plugin or EGL sink for Raspberry Pi

#### Media Pipeline

**File**: `core/hal/multimedia/MediaPipeline.h`

Provides abstraction for GStreamer pipeline management:
- Pipeline state transitions (NULL → READY → PAUSED → PLAYING)
- Audio/video path linking with transcoding
- Error handling and recovery
- Performance monitoring (CPU/memory usage, frame drops)

### Wireless HAL

#### WiFi Management

**File**: `core/hal/wireless/WiFiManager.h`

Features:
- Network scanning and discovery
- Connection management (connect, disconnect)
- Saved network management
- Signal strength monitoring
- SSID and security type detection (WEP, WPA, WPA2, WPA3)

**Backend**: Uses NetworkManager via D-Bus interface on Linux

#### Bluetooth Management

**File**: `core/hal/wireless/BluetoothManager.h`

Features:
- Device discovery and pairing
- Connection management
- Audio profile support (A2DP, HFP, HSP, AVRCP)
- Device type classification (phone, audio, peripheral, etc.)

**Backend**: Uses BlueZ via D-Bus interface on Linux

#### Network Service

**File**: `core/hal/wireless/NetworkService.h`

Features:
- Network interface enumeration
- Connectivity status monitoring
- DNS configuration
- Hostname management
- Network latency monitoring (ping)

### Android Auto Service

**File**: `core/services/android_auto/AndroidAutoService.h`, `AndroidAutoService.cpp`

The Android Auto implementation uses AASDK (Android Auto Software Development Kit) from the OpenCarDev project.

#### Features

- **USB Device Discovery**: Detects Android devices connected via USB
- **Protocol Handling**: Implements Android Auto protocol for video/audio/input streaming
- **Video Projection**: Receives encoded video frames from Android device
- **Audio Streaming**: Handles audio output from Android device
- **Touch/Key Input**: Sends user input back to Android device
- **Frame Rate & Resolution**: Configurable video parameters
- **Latency Monitoring**: Tracks and reports transmission latency and frame drops

#### Integration Points

1. **Event Bus**: Publishes events for connection status, video frames, audio data
2. **Video Manager**: Uses for display resolution configuration
3. **Audio Manager**: Routes Android Auto audio through device audio system
4. **WebSocket Server**: Provides real-time data to UI for status display

#### AASDK Integration

The AASDK library is added as a Git submodule:
```
third_party/aasdk/
```

AASDK components used:
- `libusb` - USB device enumeration and communication
- `aasdk-core` - Protocol implementation
- `aasdk-transport` - USB transport layer
- `aasdk-video` - Video stream handling
- `aasdk-audio` - Audio stream handling

## QML UI Components

### Android Auto Screen

**File**: `ui/qml/AndroidAutoScreen.qml`

Displays Android device projection with:
- Status indicator (connected/disconnected)
- Video rendering surface
- Touch input handling (scaled to device coordinates)
- Control buttons (back, home, settings, disconnect)
- Connection status and performance metrics (FPS, latency)

### Settings Screens

#### Audio Settings

**File**: `ui/qml/AudioSettingsScreen.qml`

Settings for:
- Master volume with slider
- Mute/unmute toggle
- Audio output route (speaker, headphone, Bluetooth, USB)
- Stream-specific volumes (music, navigation, calls)

#### Video Settings

**File**: `ui/qml/VideoSettingsScreen.qml`

Settings for:
- Display resolution selection
- Refresh rate configuration
- Brightness and contrast sliders
- Night mode toggle with colour temperature control

#### Connectivity Settings

**File**: `ui/qml/ConnectivitySettingsScreen.qml`

Settings for:
- WiFi enable/disable
- WiFi network scanning and connection
- Signal strength display
- Bluetooth enable/disable
- Bluetooth device discovery and pairing
- Connected devices status

## Event Bus Protocol

### Audio Events

```json
{
  "topic": "audio/volume",
  "payload": { "master": 75 }
}

{
  "topic": "audio/route",
  "payload": { "route": "SPEAKER" }
}

{
  "topic": "audio/mute",
  "payload": { "enabled": true }
}
```

### Video Events

```json
{
  "topic": "video/resolution",
  "payload": { "resolution": "1920x1080" }
}

{
  "topic": "video/brightness",
  "payload": { "value": 85 }
}

{
  "topic": "video/night-mode",
  "payload": { "enabled": true, "kelvin": 3500 }
}
```

### Wireless Events

```json
{
  "topic": "wifi/enabled",
  "payload": { "enabled": true }
}

{
  "topic": "wifi/connect",
  "payload": { "ssid": "MyNetwork", "password": "pass123" }
}

{
  "topic": "bluetooth/connect",
  "payload": { "device": "Car Phone" }
}
```

### Android Auto Events

```json
{
  "topic": "androidauto/status",
  "payload": {}
}

{
  "topic": "androidauto/touch",
  "payload": { "x": 0.5, "y": 0.3, "action": "down" }
}

{
  "topic": "androidauto/key",
  "payload": { "key": "BACK" }
}
```

## Implementation Notes

### GStreamer Pipeline Examples

#### Audio Playback

```
pulsesrc → audioconvert → alsasink
```

#### H.264 Video Decoding & Display

```
appsrc → h264parse → nvv4l2decoder → videoscale → videorate → eglfs
```

#### Android Auto Video Reception

```
USB (aasdk) → h264parse → nvv4l2decoder → videoscale → qtwindow
```

### Raspberry Pi Considerations

- Use `v4l2h264dec` for hardware video decoding
- Use `eglfs` platform for direct framebuffer rendering
- Consider `VPU` encoding for lower latency
- Use `omxh264dec` if available for hardware acceleration
- Monitor CPU/GPU temperature during Android Auto streaming

### Performance Optimisation

1. **Video**: Use hardware acceleration (NVIDIA's NVENC/NVDEC, V4L2 hardware decoders)
2. **Audio**: Use direct ALSA access for lower latency
3. **Touch**: Process input events on separate thread to avoid blocking video
4. **Memory**: Use ring buffers for audio/video data to avoid allocation overhead

## Future Enhancements

1. **Multiple Android Auto Sessions**: Support projecting on different displays
2. **Wireless Android Auto**: Support WiFi-based connection via adb
3. **Audio Format Support**: Add support for AAC, MP3, Opus
4. **Video Codec Support**: Add H.265/HEVC, VP9, AV1 support
5. **Audio Mixing**: Implement proper audio mixing for multiple sources
6. **Gesture Support**: Detect swipe gestures for navigation
7. **Voice Input**: Microphone input to Android device
8. **Permission Management**: Implement Android Auto permission handling

## Testing

### Unit Tests

- `tests/test_audio_device.cpp` - Audio device enumeration and control
- `tests/test_video_device.cpp` - Video resolution and display management
- `tests/test_wifi_manager.cpp` - WiFi scanning and connection
- `tests/test_bluetooth_manager.cpp` - Bluetooth device discovery and pairing
- `tests/test_android_auto_service.cpp` - Android Auto protocol handling

### Integration Tests

- Test audio streaming with actual GStreamer pipeline
- Test video decoding from Android device
- Test touch input translation and latency
- Test WiFi/Bluetooth connection stability
- Test concurrent audio/video streaming

## References

- AASDK Project: https://github.com/opencardev/aasdk
- OpenAuto Reference: https://github.com/opencardev/openauto
- GStreamer Documentation: https://gstreamer.freedesktop.org/documentation
- Qt6 QML Documentation: https://doc.qt.io/qt-6/
