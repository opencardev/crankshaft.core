# PipeWire and Audio Routing Configuration

## Overview

Crankshaft MVP implements audio routing through PipeWire with fallback support for PulseAudio on legacy systems. This guide documents the audio architecture, configuration, and integration points.

## Audio Stack Architecture

```
┌─────────────────────────────────┐
│   Application Layer             │
│ (Crankshaft Core / Extensions)  │
└──────────────┬──────────────────┘
               │
         ┌─────▼──────┐
         │   Qt Audio  │
         │   Backend   │
         └─────┬──────┘
               │
    ┌──────────┴──────────┐
    │                     │
┌───▼─────┐        ┌──────▼──┐
│PipeWire │        │PulseAudio│
│ (Primary)│      │(Fallback)│
└───┬──────┘        └──────┬──┘
    │                     │
    └──────────┬──────────┘
               │
        ┌──────▼──────┐
        │ ALSA/Kernel │
        │    Audio    │
        └─────────────┘
```

## PipeWire Configuration

### System Requirements

- PipeWire >= 0.3.60 (recommended 1.0+)
- wireplumber >= 0.4.0 (session/device management)
- alsa-ucm-conf (ALSA use-case manager)
- pipewire-alsa (ALSA compatibility layer)

### Installation

```bash
# On Debian/Raspberry Pi OS
sudo apt install -y \
  pipewire \
  pipewire-alsa \
  wireplumber \
  libpipewire-0.3-modules \
  gstreamer1.0-pipewire

# Verify installation
pw-cli info 0
```

### Configuration Files

PipeWire uses JSON configuration files located in:
- System defaults: `/etc/pipewire/pipewire.conf`
- User overrides: `$HOME/.config/pipewire/pipewire.conf`
- Module configs: `/etc/pipewire/pipewire.conf.d/`

**Crankshaft Audio Configuration** (`/etc/pipewire/pipewire.conf.d/50-crankshaft.conf`):

```json
{
  "context.properties": {
    "log.level": 2,
    "default.clock.min-quantum": 32,
    "default.clock.max-quantum": 4096
  },
  "stream.properties": {
    "application.icon-name": "crankshaft",
    "resample.quality": 4
  }
}
```

**WirePlumber Configuration** (`/etc/pipewire/wireplumber.conf.d/50-crankshaft.lua`):

```lua
-- Route Android Auto audio to designated vehicle output
rule = {
  matches = {
    {
      { "media.class", "=", "Audio/Sink" }
    }
  },
  apply_properties = {
    ["audio.position"] = "MONO"
  }
}

-- Priority rule for media streams
alsa_monitor.rules = {
  {
    matches = {
      {
        { "node.name", "matches", "alsa_output.*" }
      }
    },
    apply_properties = {
      ["priority.session"] = 100
    }
  }
}
```

## Audio Routing Strategy

### Primary Flow: PipeWire

1. **Initialisation** (startup)
   - Crankshaft detects PipeWire daemon running
   - qt-multimedia loads PipeWire plugin
   - Enumerate input/output devices via org.freedesktop.DBus interface

2. **Device Routing**
   - **Navigation**: Route to primary speaker/line-out
   - **Media**: Route to headphone jack or Bluetooth speaker (user configurable)
   - **Phone Call**: Route to Bluetooth headset with microphone (priority)
   - **System Alerts**: Route to all active outputs (ducking other streams)

3. **Stream Management**
   - Each audio source creates PipeWire node
   - WirePlumber assigns streams to sinks per policy
   - Media priority > Navigation > System alerts

### Fallback Flow: PulseAudio

If PipeWire unavailable (e.g., legacy Raspberry Pi OS):

```cpp
// In AudioHAL.cpp
QAudioDevice AudioHAL::selectPrimaryAudioDevice() {
  QMediaDevices mediaDevices;
  
  // Try PipeWire first
  const auto& pipewireDevices = mediaDevices.audioOutputs();
  for (const auto& device : pipewireDevices) {
    if (device.description().contains("PipeWire", Qt::CaseInsensitive)) {
      return device;
    }
  }
  
  // Fallback to PulseAudio
  for (const auto& device : pipewireDevices) {
    if (device.description().contains("pulseaudio", Qt::CaseInsensitive)) {
      return device;
    }
  }
  
  // Fallback to default ALSA device
  return mediaDevices.defaultAudioOutput();
}
```

## Audio Device Management

### Detecting Available Devices

```cpp
// In AudioHAL.cpp
void AudioHAL::enumerateDevices() {
  QMediaDevices mediaDevices;
  
  const auto& outputs = mediaDevices.audioOutputs();
  for (const auto& output : outputs) {
    qDebug() << "Output:" << output.description() 
             << "ID:" << output.id()
             << "Channels:" << output.maximumChannelCount();
  }
  
  const auto& inputs = mediaDevices.audioInputs();
  for (const auto& input : inputs) {
    qDebug() << "Input:" << input.description()
             << "ID:" << input.id();
  }
}
```

### Listening for Device Changes

```cpp
// In AudioRouter.cpp
void AudioRouter::onAudioDevicesChanged() {
  QMediaDevices mediaDevices;
  
  // Disconnect from old device if active
  if (m_currentAudioOutput) {
    m_currentAudioOutput->stop();
    m_currentAudioOutput->setAudioDevice(mediaDevices.defaultAudioOutput());
  }
  
  // Trigger re-routing
  routeStreamToDevice(AudioStreamRole::Media, m_preferredMediaDevice);
}
```

## Stream Routing Configuration

### Per-Stream Configuration

```cpp
// In AudioRouter.cpp
void AudioRouter::routeStreamToDevice(AudioStreamRole role, 
                                      const QString& deviceId) {
  QAudioOutput audioOutput;
  QMediaDevices mediaDevices;
  
  // Find target device
  const auto& devices = mediaDevices.audioOutputs();
  auto it = std::find_if(devices.begin(), devices.end(),
    [&deviceId](const QAudioDevice& d) { return d.id() == deviceId; });
  
  if (it != devices.end()) {
    audioOutput.setAudioDevice(*it);
    audioOutput.setVolume(volumeForRole(role));
  }
}

// Volume levels per stream
int AudioRouter::volumeForRole(AudioStreamRole role) {
  static const std::map<AudioStreamRole, int> volumes = {
    {AudioStreamRole::Phone, 85},
    {AudioStreamRole::Media, 75},
    {AudioStreamRole::Navigation, 70},
    {AudioStreamRole::SystemAlert, 90}
  };
  return volumes.at(role);
}
```

### Priority and Ducking

```cpp
// Audio mixing priority in PipeWire
// High priority streams (Phone) cause ducking of lower priority streams
void AudioRouter::setStreamPriority(const QString& streamName, int priority) {
  // Via PipeWire CLI (for testing)
  // pw-play --volume 1.0 --property media.role=phone audio.wav
  
  // Via Qt (in production)
  QAudioOutput output;
  // Qt doesn't expose priority directly; use PipeWire DBus interface
  // or configure via WirePlumber rules
}
```

## JACK Integration (Optional)

For Professional Audio / Multi-room Audio Scenarios:

```bash
# Install JACK
sudo apt install -y jackd2 libjack-jackd2-0

# Start JACK server
jackd -d alsa -d hw:0 -r 48000 -p 256 &

# Connect JACK and PipeWire
pw-jack

# Route Crankshaft audio via JACK
export LD_LIBRARY_PATH=/usr/lib/jack:$LD_LIBRARY_PATH
```

## Troubleshooting

### No Audio Output

**Symptom**: Application plays audio but no sound from speakers

**Steps**:
1. Check PipeWire status:
   ```bash
   systemctl --user status pipewire
   pw-cli info 0
   ```

2. List devices:
   ```bash
   pw-play --list-sinks
   ```

3. Check volume levels:
   ```bash
   alsamixer -c 0
   ```

4. Verify ALSA configuration:
   ```bash
   cat /proc/asound/cards
   ```

### PipeWire Crashes

**Symptom**: PipeWire dies unexpectedly; audio becomes unavailable

**Steps**:
1. Check PipeWire logs:
   ```bash
   journalctl --user-unit=pipewire -n 50
   ```

2. Increase PipeWire verbosity:
   ```bash
   export PIPEWIRE_DEBUG=D
   systemctl --user restart pipewire
   ```

3. Review configuration for syntax errors:
   ```bash
   pw-config validate /etc/pipewire/pipewire.conf
   ```

### Audio Latency Too High

**Symptom**: Delay between action (button press) and audio response

**Optimisation**:
1. Reduce quantum size in PipeWire config:
   ```json
   "default.clock.min-quantum": 16
   ```

2. Set real-time scheduling:
   ```bash
   # Add to /etc/security/limits.d/audio.conf
   @audio - memlock unlimited
   @audio - rtprio 99
   @audio - nice -11
   ```

3. Disable power saving:
   ```bash
   # In ALSA config
   options snd-hda-intel power_save=0
   ```

### Bluetooth Audio Not Connecting

**Symptom**: Bluetooth device listed but not selectable for audio output

**Steps**:
1. Check Bluetooth stack status:
   ```bash
   systemctl status bluetooth
   hciconfig
   ```

2. Verify PipeWire Bluetooth support:
   ```bash
   pw-cli info | grep -i bluetooth
   ```

3. Manually connect device:
   ```bash
   bluetoothctl
   > power on
   > scan on
   > pair <MAC_ADDRESS>
   > connect <MAC_ADDRESS>
   ```

4. Check WirePlumber Bluetooth config:
   ```bash
   journalctl --user-unit=wireplumber -n 50 | grep -i bluetooth
   ```

## Testing Audio Routing

### Unit Tests

Verify audio device selection logic in `tests/unit/test_audio_routing.cpp`:

```cpp
TEST_CASE("AudioRouter selects PipeWire device", "[audio]") {
  AudioRouter router;
  auto device = router.selectAudioDevice(AudioStreamRole::Media);
  REQUIRE(device.isValid());
  REQUIRE(device.description().contains("PipeWire") || 
          device.description().contains("pulseaudio"));
}
```

### Integration Test

Test actual audio playback with different devices:

```bash
# Play test tone on media output
./build/tests/test_audio_playback --role media --tone 440Hz

# Record from microphone input
./build/tests/test_audio_record --duration 5 --output /tmp/test.wav

# Route test: record navigation audio while playing media
./build/tests/test_audio_concurrent_streams
```

### Manual Testing

```bash
# Play audio file via Crankshaft
./build/ui/crankshaft-ui --test-audio=/path/to/test.mp3

# Verify device enumeration
journalctl --user-unit=crankshaft-core -n 20 | grep -i audio

# Monitor PipeWire during playback
pw-play --list-active-nodes
```

## Performance Considerations

### Latency Budget
- Target latency for user-perceivable operations: < 200ms
  - Button press → audio response: ~50ms
  - Audio route change → active playback: ~100ms
  - Device hotplug → re-route: ~150ms

### CPU Usage
- Crankshaft audio processing: < 5% CPU (RPI4)
- PipeWire daemon overhead: < 2%
- Audio decoding (MP3): ~8% per stream

### Memory Usage
- PipeWire daemon: ~30MB resident
- Audio buffers (16 channels @ 48kHz): ~2MB
- Total audio subsystem: < 50MB

## References

- **PipeWire Documentation**: https://docs.pipewire.org/
- **WirePlumber Configuration**: https://wiki.archlinux.org/title/PipeWire
- **Qt QAudioOutput Documentation**: https://doc.qt.io/qt-6/qaudiooutput.html
- **ALSA Configuration**: https://alsa-project.org/wiki/Main_Page

## Integration with Crankshaft

### Core Service: AudioRouter

Location: `core/services/audio/AudioRouter.{h,cpp}`

**Responsibilities**:
- Enumerate audio devices at startup
- Maintain preferred device configuration (persisted to PreferencesService)
- Route streams to configured devices
- Monitor for device hotplug events
- Implement volume ducking for priority streams

**Event Publishing**:
- `audio/device-enumerated`: List of available devices
- `audio/device-changed`: Device added/removed
- `audio/stream-routed`: Stream successfully routed
- `audio/routing-error`: Routing failure

### HAL Service: AudioHAL

Location: `core/hal/multimedia/AudioHAL.{h,cpp}`

**Responsibilities**:
- Detect PipeWire vs. PulseAudio availability
- Initialise audio subsystem with fallback strategy
- Manage audio device properties (sample rate, channels, format)
- Detect and report audio hardware errors

### Configuration

User audio preferences stored in SQLite:

```sql
-- Audio device preferences
INSERT INTO preferences (key, value, json_value) VALUES
  ('audio.primary-output-device', '', '{"device_id": "...", "description": "HDMI Out"}'),
  ('audio.fallback-output-device', '', '{"device_id": "...", "description": "Internal DAC"}'),
  ('audio.media-volume', '75', NULL),
  ('audio.navigation-volume', '70', NULL),
  ('audio.phone-volume', '85', NULL),
  ('audio.enable-ducking', 'true', NULL);
```

## Future Enhancements

1. **Spatial Audio / Surround Sound**
   - 5.1/7.1 surround sound for Android Auto
   - Dolby Digital (AC3) bitstream pass-through via SPDIF

2. **Multi-zone Audio**
   - Route different streams to different vehicle zones
   - Passenger entertainment zone independent from driver audio

3. **Audio Effects**
   - EQ presets per app/scenario
   - Noise cancellation for phone calls
   - Volume normalization across streams

4. **DSD Audio Support**
   - High-resolution audio (DSD64/128) playback
   - Native DSD support via PipeWire plugins

5. **Network Audio**
   - AirPlay/AirTunes receiver
   - Chromecast audio support
   - DLNA/UPnP integration
