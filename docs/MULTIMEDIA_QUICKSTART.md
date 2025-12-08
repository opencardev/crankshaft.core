# Multimedia HAL Quick Reference

## Quick Start

### 1. Video Decoder Usage

```cpp
#include "core/hal/multimedia/GStreamerVideoDecoder.h"

// Create decoder
auto decoder = std::make_unique<GStreamerVideoDecoder>();

// Configure
IVideoDecoder::DecoderConfig config;
config.codec = IVideoDecoder::CodecType::H264;
config.width = 1920;
config.height = 1080;
config.fps = 30;
config.outputFormat = IVideoDecoder::PixelFormat::RGBA;
config.useHardwareAcceleration = true;

// Initialize
if (!decoder->initialize(config)) {
  // Handle error
}

// Connect signal
connect(decoder.get(), &IVideoDecoder::frameDecoded,
        this, [](int width, int height, const uint8_t* data, int size) {
  // Process decoded RGBA frame
  qDebug() << "Frame:" << width << "x" << height << "size:" << size;
});

// Decode H.264 frames
aasdk::common::Data h264Frame = ...;
decoder->decodeFrame(h264Frame);
```

### 2. Audio Mixer Usage

```cpp
#include "core/hal/multimedia/AudioMixer.h"

// Create mixer
auto mixer = std::make_unique<AudioMixer>();

// Configure master format (48kHz stereo 16-bit)
IAudioMixer::AudioFormat masterFormat{48000, 2, 16};
mixer->initialize(masterFormat);

// Add media channel (48kHz stereo)
IAudioMixer::ChannelConfig mediaConfig;
mediaConfig.id = IAudioMixer::ChannelId::MEDIA;
mediaConfig.volume = 0.8f;
mediaConfig.priority = 1;
mediaConfig.format = masterFormat;
mixer->addChannel(mediaConfig);

// Add system channel (16kHz mono - will be converted)
IAudioMixer::ChannelConfig systemConfig;
systemConfig.id = IAudioMixer::ChannelId::SYSTEM;
systemConfig.volume = 1.0f;
systemConfig.priority = 2;
systemConfig.format = {16000, 1, 16};
mixer->addChannel(systemConfig);

// Connect signal
connect(mixer.get(), &IAudioMixer::audioMixed,
        this, [](const QByteArray& mixedData) {
  // Process mixed PCM audio
  qDebug() << "Mixed audio size:" << mixedData.size();
});

// Mix audio data
QByteArray pcmData = ...;
mixer->mixAudioData(IAudioMixer::ChannelId::MEDIA, pcmData);
```

### 3. Protocol Helpers Usage

```cpp
#include "core/services/android_auto/ProtocolHelpers.h"

using namespace crankshaft::protocol;

// Send touch input
float x = 0.5f;  // Center of screen
float y = 0.5f;
auto touchDown = createTouchInputReport(x, y, TouchAction::ACTION_DOWN);
inputChannel->sendInputReport(touchDown, promise);

// Send key input (Home button)
auto keyDown = createKeyInputReport(3, KeyAction::ACTION_DOWN);  // KEYCODE_HOME
inputChannel->sendInputReport(keyDown, promise);

// Request audio focus
auto gainFocus = createAudioFocusNotification(AudioFocusState::GAIN);
controlChannel->sendAudioFocusNotification(gainFocus, promise);
```

## Integration Checklist

### RealAndroidAutoService Integration

- [ ] Add `#include "../../hal/multimedia/GStreamerVideoDecoder.h"`
- [ ] Add `#include "../../hal/multimedia/AudioMixer.h"`
- [ ] Add `#include "ProtocolHelpers.h"`
- [ ] Add `IVideoDecoder* m_videoDecoder` member
- [ ] Add `IAudioMixer* m_audioMixer` member
- [ ] Initialize decoder in `setupChannels()`
- [ ] Initialize mixer in `setupChannels()`
- [ ] Connect decoder signals
- [ ] Connect mixer signals
- [ ] Update `onVideoChannelUpdate()` to use decoder
- [ ] Update `onMediaAudioChannelUpdate()` to use mixer
- [ ] Update `onSystemAudioChannelUpdate()` to use mixer
- [ ] Update `onSpeechAudioChannelUpdate()` to use mixer
- [ ] Update `sendTouchInput()` to use protocol helpers
- [ ] Update `sendKeyInput()` to use protocol helpers
- [ ] Update `requestAudioFocus()` to use protocol helpers
- [ ] Cleanup decoder/mixer in `cleanupChannels()`

### Build System

- [ ] Verify GStreamer packages installed
- [ ] Verify CMakeLists.txt includes new sources
- [ ] Build with: `wsl bash -lc "cmake --build build -j"`
- [ ] Check for compilation errors
- [ ] Check for linker errors

### Testing

- [ ] Test video decoder with sample H.264 file
- [ ] Test audio mixer with sample PCM files
- [ ] Test protocol helpers with AASDK connection
- [ ] Test hardware acceleration detection
- [ ] Test format conversion (mono↔stereo, resampling)
- [ ] Test on Raspberry Pi 4
- [ ] Monitor CPU usage
- [ ] Monitor memory usage
- [ ] Check for audio distortion
- [ ] Check for video corruption

## Common Android KeyCodes

```cpp
// Navigation
KEYCODE_HOME = 3
KEYCODE_BACK = 4
KEYCODE_MENU = 82

// Media
KEYCODE_MEDIA_PLAY_PAUSE = 85
KEYCODE_MEDIA_STOP = 86
KEYCODE_MEDIA_NEXT = 87
KEYCODE_MEDIA_PREVIOUS = 88

// Volume
KEYCODE_VOLUME_UP = 24
KEYCODE_VOLUME_DOWN = 25
KEYCODE_VOLUME_MUTE = 164

// Phone
KEYCODE_CALL = 5
KEYCODE_ENDCALL = 6
```

## Troubleshooting

### "Failed to create GStreamer pipeline"
```bash
# Check GStreamer installation
gst-inspect-1.0 --version

# Install missing packages
sudo apt-get install gstreamer1.0-plugins-base gstreamer1.0-plugins-good
```

### "Hardware decoder not available"
```bash
# Raspberry Pi
sudo apt-get install gstreamer1.0-omx

# Intel/AMD
sudo apt-get install gstreamer1.0-vaapi intel-media-va-driver

# Check available decoders
gst-inspect-1.0 | grep h264dec
```

### Audio distortion
```cpp
// Reduce volumes
mixer->setChannelVolume(IAudioMixer::ChannelId::MEDIA, 0.5f);
mixer->setMasterVolume(0.7f);
```

### Enable GStreamer debug logging
```bash
export GST_DEBUG=3
./build/core/crankshaft-core
```

## Performance Tips

1. **Use hardware acceleration** - Enables 1080p30 on Raspberry Pi 4
2. **Match sample rates** - Avoid resampling overhead (use 48kHz for all channels)
3. **Reduce channel count** - Keep to 3-4 active channels maximum
4. **Monitor CPU usage** - Use `htop` to check core utilization
5. **Check thermal throttling** - Raspberry Pi throttles at 80°C

## Documentation

- Full documentation: `docs/MULTIMEDIA_HAL.md`
- Implementation summary: `docs/fix_summaries/multimedia_implementation.md`
- Android Auto channels: `docs/ANDROID_AUTO_CHANNELS.md`

## Files Overview

```
core/
├── hal/
│   └── multimedia/
│       ├── IVideoDecoder.h             (95 lines)
│       ├── GStreamerVideoDecoder.h     (61 lines)
│       ├── GStreamerVideoDecoder.cpp   (407 lines)
│       ├── IAudioMixer.h               (168 lines)
│       ├── AudioMixer.h                (72 lines)
│       └── AudioMixer.cpp              (431 lines)
└── services/
    └── android_auto/
        ├── ProtocolHelpers.h           (80 lines)
        └── ProtocolHelpers.cpp         (128 lines)

Total: ~1,442 lines of new code
```
