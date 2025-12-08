# Multimedia HAL Implementation

## Overview

The Multimedia HAL provides hardware abstraction for audio and video processing in Crankshaft. It consists of swappable decoder and mixer interfaces that allow the system to adapt to different hardware capabilities.

## Components

### Video Decoder Interface

**File:** `core/hal/multimedia/IVideoDecoder.h`

Abstract interface for video decoding that supports multiple codec types and pixel formats.

**Features:**
- Codec support: H.264, H.265, VP8, VP9, AV1
- Output formats: RGBA, RGB, NV12, YUV420P
- Hardware acceleration support
- Statistics tracking (decoded/dropped frames, average decode time)

**Signals:**
- `frameDecoded(width, height, data, size)` - Emitted when a frame is successfully decoded
- `errorOccurred(error)` - Emitted on decoder errors
- `statsUpdated(decoded, dropped, avgTime)` - Emitted periodically with statistics

### GStreamer Video Decoder

**Files:** `core/hal/multimedia/GStreamerVideoDecoder.{h,cpp}`

GStreamer-based implementation of `IVideoDecoder` with automatic hardware acceleration detection.

**Pipeline Architecture:**
```
appsrc → h264parse → decoder → videoconvert → appsink
```

**Hardware Acceleration:**
The decoder automatically detects and uses available hardware acceleration:

1. **VA-API** (`vaapih264dec`) - Intel/AMD GPUs on Linux
2. **OMX** (`omxh264dec`) - Raspberry Pi VideoCore IV/VI
3. **NVDEC** (`nvh264dec`) - NVIDIA GPUs
4. **Software Fallback** (`avdec_h264`) - FFmpeg libavcodec

**Usage:**
```cpp
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
if (decoder->initialize(config)) {
  // Connect signal
  connect(decoder.get(), &IVideoDecoder::frameDecoded,
          this, &MyClass::onFrameDecoded);
  
  // Decode frames
  decoder->decodeFrame(h264Data);
}
```

**Integration with Android Auto:**
In `RealAndroidAutoService`, the decoder is used to process H.264 video streams:

```cpp
// In header
IVideoDecoder* m_videoDecoder = nullptr;

// In setupChannels()
m_videoDecoder = new GStreamerVideoDecoder(this);
IVideoDecoder::DecoderConfig config;
config.codec = IVideoDecoder::CodecType::H264;
config.width = 1920;  // From Android Auto handshake
config.height = 1080;
config.fps = 30;
config.outputFormat = IVideoDecoder::PixelFormat::RGBA;
config.useHardwareAcceleration = true;

if (!m_videoDecoder->initialize(config)) {
  Logger::instance().error("Failed to initialize video decoder");
}

connect(m_videoDecoder, &IVideoDecoder::frameDecoded,
        this, [this](int width, int height, const uint8_t* data, int size) {
  emit videoFrameReady(QByteArray(reinterpret_cast<const char*>(data), size), 
                       width, height);
});

// In onVideoChannelUpdate()
void RealAndroidAutoService::onVideoChannelUpdate(const aasdk::common::Data& data) {
  if (m_videoDecoder && m_videoDecoder->isReady()) {
    m_videoDecoder->decodeFrame(data);
  } else {
    // Fallback: emit raw H.264
    emit videoDataReady(QByteArray(reinterpret_cast<const char*>(data.data()), 
                                  data.size()));
  }
}
```

**Performance:**
- Hardware decoding: ~5-10ms per frame at 1080p30
- Software decoding: ~30-50ms per frame at 1080p30
- Buffer management: 10MB max buffer size to prevent memory issues

### Audio Mixer Interface

**File:** `core/hal/multimedia/IAudioMixer.h`

Abstract interface for mixing multiple audio streams with priority-based routing.

**Features:**
- 4 audio channels: MEDIA, SYSTEM, SPEECH, TELEPHONY
- Per-channel volume control (0.0 - 1.0)
- Per-channel muting
- Master volume control
- Priority-based mixing
- Format conversion (sample rate, channels, bit depth)

**Channels:**
- **MEDIA** (Priority 1): Music, video audio
- **SYSTEM** (Priority 2): Navigation prompts, alerts
- **SPEECH** (Priority 3): Assistant responses
- **TELEPHONY** (Priority 4): Phone calls

**Usage:**
```cpp
// Create mixer
auto mixer = std::make_unique<AudioMixer>();

// Configure master format
IAudioMixer::AudioFormat masterFormat;
masterFormat.sampleRate = 48000;
masterFormat.channels = 2;
masterFormat.bitsPerSample = 16;

mixer->initialize(masterFormat);

// Add channels
IAudioMixer::ChannelConfig mediaConfig;
mediaConfig.id = IAudioMixer::ChannelId::MEDIA;
mediaConfig.volume = 0.8f;
mediaConfig.muted = false;
mediaConfig.priority = 1;
mediaConfig.format = masterFormat;

mixer->addChannel(mediaConfig);

// Connect signal
connect(mixer.get(), &IAudioMixer::audioMixed,
        this, &MyClass::onAudioMixed);

// Mix audio
mixer->mixAudioData(IAudioMixer::ChannelId::MEDIA, pcmData);
```

### Audio Mixer Implementation

**Files:** `core/hal/multimedia/AudioMixer.{h,cpp}`

Software PCM mixer with format conversion and resampling.

**Mixing Algorithm:**
1. Convert all channel formats to master format (resample + channel conversion)
2. Buffer audio data for each channel
3. When minimum buffer size reached across all active channels:
   - Sort channels by priority (highest first)
   - Mix samples with volume scaling
   - Apply soft saturation to prevent clipping
   - Emit mixed audio
   - Remove mixed data from channel buffers

**Format Conversion:**
- **Mono ↔ Stereo:**
  - Mono to stereo: Duplicate samples
  - Stereo to mono: Average left/right channels
- **Resampling:** Linear interpolation
  - Example: 16kHz → 48kHz (3x upsampling)
  - Example: 48kHz → 44.1kHz (0.91875x downsampling)

**Saturation:**
Soft clipping algorithm to prevent harsh distortion:
```cpp
if (sample > 32767) {
  excess = sample - 32767;
  sample = 32767 - (excess * 0.5);  // Reduce excess by 50%
  clamp to 32767;
}
```

**Integration with Android Auto:**
```cpp
// In RealAndroidAutoService

// In header
IAudioMixer* m_audioMixer = nullptr;

// In setupChannels()
m_audioMixer = new AudioMixer(this);

IAudioMixer::AudioFormat masterFormat;
masterFormat.sampleRate = 48000;
masterFormat.channels = 2;
masterFormat.bitsPerSample = 16;

if (!m_audioMixer->initialize(masterFormat)) {
  Logger::instance().error("Failed to initialize audio mixer");
}

// Add media channel (48kHz stereo)
IAudioMixer::ChannelConfig mediaConfig;
mediaConfig.id = IAudioMixer::ChannelId::MEDIA;
mediaConfig.volume = 0.8f;
mediaConfig.priority = 1;
mediaConfig.format = masterFormat;
m_audioMixer->addChannel(mediaConfig);

// Add system channel (16kHz mono)
IAudioMixer::ChannelConfig systemConfig;
systemConfig.id = IAudioMixer::ChannelId::SYSTEM;
systemConfig.volume = 1.0f;
systemConfig.priority = 2;
systemConfig.format = {16000, 1, 16};  // 16kHz mono
m_audioMixer->addChannel(systemConfig);

// Add speech channel (16kHz mono)
IAudioMixer::ChannelConfig speechConfig;
speechConfig.id = IAudioMixer::ChannelId::SPEECH;
speechConfig.volume = 1.0f;
speechConfig.priority = 3;
speechConfig.format = {16000, 1, 16};
m_audioMixer->addChannel(speechConfig);

connect(m_audioMixer, &IAudioMixer::audioMixed,
        this, [this](const QByteArray& mixedData) {
  emit audioDataReady(mixedData);
});

// In channel update handlers
void RealAndroidAutoService::onMediaAudioChannelUpdate(const aasdk::common::Data& data) {
  if (m_audioMixer) {
    QByteArray audioData(reinterpret_cast<const char*>(data.data()), data.size());
    m_audioMixer->mixAudioData(IAudioMixer::ChannelId::MEDIA, audioData);
  }
}

void RealAndroidAutoService::onSystemAudioChannelUpdate(const aasdk::common::Data& data) {
  if (m_audioMixer) {
    QByteArray audioData(reinterpret_cast<const char*>(data.data()), data.size());
    m_audioMixer->mixAudioData(IAudioMixer::ChannelId::SYSTEM, audioData);
  }
}
```

## Protocol Helpers

**Files:** `core/services/android_auto/ProtocolHelpers.{h,cpp}`

Helper functions for constructing AASDK protocol buffer messages.

### Touch Input

```cpp
using namespace crankshaft::protocol;

// Touch down
auto touchDown = createTouchInputReport(
  0.5f, 0.5f,                    // x, y (normalized 0-1)
  TouchAction::ACTION_DOWN,       // action
  0,                              // pointer ID
  getCurrentTimestampMicros()     // timestamp
);

// Send via input channel
m_inputChannel->sendInputReport(touchDown, promise);
```

### Key Input

```cpp
// Home button press
auto keyDown = createKeyInputReport(
  3,  // Android KeyEvent.KEYCODE_HOME
  KeyAction::ACTION_DOWN
);

m_inputChannel->sendInputReport(keyDown, promise);

// Home button release
auto keyUp = createKeyInputReport(
  3,
  KeyAction::ACTION_UP
);

m_inputChannel->sendInputReport(keyUp, promise);
```

### Audio Focus

```cpp
// Give audio focus to Android Auto
auto gainFocus = createAudioFocusNotification(AudioFocusState::GAIN);
m_controlChannel->sendAudioFocusNotification(gainFocus, promise);

// Take audio focus from Android Auto (e.g., incoming phone call)
auto loseFocus = createAudioFocusNotification(AudioFocusState::LOSS);
m_controlChannel->sendAudioFocusNotification(loseFocus, promise);
```

## Dependencies

### GStreamer Packages

**Debian/Ubuntu/Raspberry Pi OS:**
```bash
sudo apt-get install \
  libgstreamer1.0-dev \
  libgstreamer-plugins-base1.0-dev \
  libgstreamer-plugins-good1.0-dev \
  libgstreamer-plugins-bad1.0-dev \
  gstreamer1.0-plugins-base \
  gstreamer1.0-plugins-good \
  gstreamer1.0-plugins-bad \
  gstreamer1.0-plugins-ugly \
  gstreamer1.0-libav \
  gstreamer1.0-tools
```

**Hardware Acceleration:**
- **Raspberry Pi:** `sudo apt-get install gstreamer1.0-omx`
- **Intel/AMD (VA-API):** `sudo apt-get install gstreamer1.0-vaapi`
- **NVIDIA:** Included in NVIDIA driver package

### CMake Configuration

Already configured in `core/CMakeLists.txt`:
```cmake
find_package(PkgConfig REQUIRED)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)
pkg_check_modules(GSTREAMER_APP REQUIRED gstreamer-app-1.0)
pkg_check_modules(GSTREAMER_VIDEO REQUIRED gstreamer-video-1.0)
pkg_check_modules(GSTREAMER_AUDIO REQUIRED gstreamer-audio-1.0)

target_link_libraries(crankshaft-core PRIVATE
  ${GSTREAMER_LIBRARIES}
  ${GSTREAMER_APP_LIBRARIES}
  ${GSTREAMER_VIDEO_LIBRARIES}
  ${GSTREAMER_AUDIO_LIBRARIES}
)
```

## Testing

### Video Decoder Test
Create `tests/test_video_decoder.cpp`:
```cpp
TEST_CASE("GStreamerVideoDecoder decodes H.264") {
  GStreamerVideoDecoder decoder;
  
  IVideoDecoder::DecoderConfig config;
  config.codec = IVideoDecoder::CodecType::H264;
  config.width = 800;
  config.height = 480;
  config.fps = 30;
  config.outputFormat = IVideoDecoder::PixelFormat::RGBA;
  
  REQUIRE(decoder.initialize(config));
  REQUIRE(decoder.isReady());
  
  // Load sample H.264 frame
  QFile file("test_frame.h264");
  file.open(QIODevice::ReadOnly);
  QByteArray h264Data = file.readAll();
  
  bool frameReceived = false;
  QObject::connect(&decoder, &IVideoDecoder::frameDecoded,
                   [&](int w, int h, const uint8_t* data, int size) {
    REQUIRE(w == 800);
    REQUIRE(h == 480);
    REQUIRE(size == 800 * 480 * 4);  // RGBA
    frameReceived = true;
  });
  
  aasdk::common::Data data(h264Data.begin(), h264Data.end());
  REQUIRE(decoder.decodeFrame(data));
  
  // Wait for async decode
  QTest::qWait(100);
  REQUIRE(frameReceived);
}
```

### Audio Mixer Test
Create `tests/test_audio_mixer.cpp`:
```cpp
TEST_CASE("AudioMixer mixes multiple channels") {
  AudioMixer mixer;
  
  IAudioMixer::AudioFormat format;
  format.sampleRate = 48000;
  format.channels = 2;
  format.bitsPerSample = 16;
  
  REQUIRE(mixer.initialize(format));
  
  // Add media channel
  IAudioMixer::ChannelConfig mediaConfig;
  mediaConfig.id = IAudioMixer::ChannelId::MEDIA;
  mediaConfig.volume = 0.8f;
  mediaConfig.priority = 1;
  mediaConfig.format = format;
  
  REQUIRE(mixer.addChannel(mediaConfig));
  
  // Generate test PCM data (1 second of 440Hz sine wave)
  QByteArray pcmData = generateSineWave(440, 1.0, format);
  
  bool audioMixed = false;
  QObject::connect(&mixer, &IAudioMixer::audioMixed,
                   [&](const QByteArray& mixed) {
    REQUIRE(mixed.size() > 0);
    audioMixed = true;
  });
  
  REQUIRE(mixer.mixAudioData(IAudioMixer::ChannelId::MEDIA, pcmData));
  REQUIRE(audioMixed);
}
```

## Troubleshooting

### Video Decoder Issues

**Symptom:** "Failed to create GStreamer pipeline"
- **Cause:** Missing GStreamer plugins
- **Solution:** Install required packages (see Dependencies section)

**Symptom:** "Hardware decoder not available, using software fallback"
- **Cause:** Hardware acceleration not supported or drivers missing
- **Solution:** 
  - Raspberry Pi: Install `gstreamer1.0-omx`
  - Intel/AMD: Install `gstreamer1.0-vaapi` and `intel-media-va-driver`
  - NVIDIA: Update GPU drivers

**Symptom:** Decoder crashes or produces corrupted frames
- **Cause:** Invalid H.264 stream or incorrect configuration
- **Solution:**
  - Verify H.264 stream format (byte-stream, AU-aligned)
  - Check resolution matches Android Auto handshake
  - Enable debug logging: `export GST_DEBUG=3`

### Audio Mixer Issues

**Symptom:** Distorted or clipped audio
- **Cause:** Volume levels too high, causing saturation
- **Solution:** Reduce channel volumes or master volume

**Symptom:** Audio dropouts or stuttering
- **Cause:** Resampling overhead or insufficient buffering
- **Solution:**
  - Use matching sample rates when possible
  - Increase buffer sizes
  - Check CPU usage

**Symptom:** One channel dominates others
- **Cause:** Incorrect priority or volume settings
- **Solution:** Adjust channel priorities and volumes

## Performance Considerations

### Video Decoding
- **Hardware acceleration recommended** for 1080p+ resolutions
- Software decoding acceptable for 800x480 (Raspberry Pi official display)
- Monitor CPU usage and thermal throttling on Raspberry Pi
- Consider lowering FPS if performance issues occur

### Audio Mixing
- **Resampling overhead:** 16kHz→48kHz conversion adds ~5% CPU per channel
- Keep channel counts low (3-4 maximum)
- Use same sample rate across channels when possible
- Monitor mixer buffer sizes to prevent memory growth

## Future Enhancements

### Video
- [ ] Hardware-accelerated H.265 decoding
- [ ] VP9 codec support
- [ ] OpenGL texture output (zero-copy to Qt Quick)
- [ ] Frame rate adaptation based on CPU load

### Audio
- [ ] Real-time resampling with SRC (libsamplerate)
- [ ] Audio effects (EQ, compression, reverb)
- [ ] Per-channel ducking (lower media volume when navigation speaks)
- [ ] ALSA/PulseAudio output integration
- [ ] Multi-zone audio routing (front/rear speakers)
