# API Usage Examples & Best Practices

## Audio System Usage

### Basic Audio Control

```cpp
#include "core/hal/multimedia/AudioManager.h"

// Create and initialise audio manager
auto audio_mgr = std::make_shared<AudioManagerImpl>();
if (!audio_mgr->initialise()) {
    qWarning() << "Failed to initialise audio";
    return;
}

// Get available output devices
auto outputs = audio_mgr->getOutputDevices();
for (const auto& device : outputs) {
    qDebug() << "Available output:" << device;
}

// Set master volume
audio_mgr->setMasterVolume(75);

// Handle volume changes
connect(audio_mgr.get(), &AudioManager::masterVolumeChanged,
    this, [](int volume) {
        qDebug() << "Volume changed to:" << volume;
    });
```

### Stream-Specific Volume Control

```cpp
// Set individual stream volumes
audio_mgr->setStreamVolume("MUSIC", 85);
audio_mgr->setStreamVolume("NAVIGATION", 95);
audio_mgr->setStreamVolume("CALL", 100);

// Mute audio
audio_mgr->setMuted(true);

// Listen for mute state changes
connect(audio_mgr.get(), &AudioManager::muteStateChanged,
    this, [](bool muted) {
        qDebug() << (muted ? "Muted" : "Unmuted");
    });
```

### Audio Routing

```cpp
// Check available routes
AudioManager::AudioRoute current_route = audio_mgr->getAudioRoute();

// Switch to different output
audio_mgr->setAudioRoute(AudioManager::AudioRoute::BLUETOOTH);

// Listen for route changes
connect(audio_mgr.get(), &AudioManager::audioRouteChanged,
    this, [](AudioManager::AudioRoute route) {
        switch(route) {
            case AudioManager::AudioRoute::SPEAKER:
                qDebug() << "Now using speaker";
                break;
            case AudioManager::AudioRoute::HEADPHONE:
                qDebug() << "Now using headphone";
                break;
            case AudioManager::AudioRoute::BLUETOOTH:
                qDebug() << "Now using Bluetooth";
                break;
            case AudioManager::AudioRoute::USB:
                qDebug() << "Now using USB";
                break;
            default:
                break;
        }
    });
```

## Video System Usage

### Display Configuration

```cpp
#include "core/hal/multimedia/VideoManager.h"

auto video_mgr = std::make_shared<VideoManagerImpl>();
video_mgr->initialise();

// Get available displays
auto displays = video_mgr->getDisplays();
qDebug() << "Primary display:" << video_mgr->getPrimaryDisplay();

// Configure resolution
video_mgr->setResolution(QSize(1920, 1080));
QSize current = video_mgr->getResolution();
qDebug() << "Resolution:" << current.width() << "x" << current.height();

// Handle resolution changes
connect(video_mgr.get(), &VideoManager::resolutionChanged,
    this, [](const QSize& res) {
        qDebug() << "Resolution changed to:" << res;
    });
```

### Brightness and Contrast

```cpp
// Adjust brightness (0-100)
video_mgr->setBrightness(85);

// Adjust contrast (0-100)
video_mgr->setContrast(55);

// Listen for changes
connect(video_mgr.get(), &VideoManager::brightnessChanged,
    this, [](int percent) {
        qDebug() << "Brightness:" << percent << "%";
    });
```

### Night Mode

```cpp
// Enable night mode with warm colour temperature
video_mgr->setNightModeEnabled(true);
video_mgr->setColourTemperature(3500);  // Kelvin (2700K-6500K)

// Listen for night mode changes
connect(video_mgr.get(), &VideoManager::nightModeChanged,
    this, [](bool enabled) {
        qDebug() << (enabled ? "Night mode enabled" : "Night mode disabled");
    });
```

## WiFi System Usage

### WiFi Network Management

```cpp
#include "core/hal/wireless/WiFiManager.h"

auto wifi_mgr = std::make_shared<WiFiManagerImpl>();
wifi_mgr->initialise();

// Check if WiFi is enabled
if (!wifi_mgr->isEnabled()) {
    wifi_mgr->setEnabled(true);
}

// Start scanning for networks
wifi_mgr->startScan();

// Get available networks after scan
auto networks = wifi_mgr->getAvailableNetworks();
for (const auto& net : networks) {
    qDebug() << "Network:" << net.ssid 
             << "Signal:" << net.signal_strength << "%"
             << "Security:" << static_cast<int>(net.security_type);
}
```

### WiFi Connection

```cpp
// Connect to a network
bool connected = wifi_mgr->connect("MyNetwork", "password123",
    WiFiManager::Security::WPA2);

if (!connected) {
    qWarning() << "Connection failed";
    return;
}

// Monitor connection state
connect(wifi_mgr.get(), &WiFiManager::connectionStateChanged,
    this, [](WiFiManager::ConnectionState state) {
        switch(state) {
            case WiFiManager::ConnectionState::DISCONNECTED:
                qDebug() << "Disconnected";
                break;
            case WiFiManager::ConnectionState::CONNECTING:
                qDebug() << "Connecting...";
                break;
            case WiFiManager::ConnectionState::CONNECTED:
                qDebug() << "Connected!";
                break;
            case WiFiManager::ConnectionState::FAILED:
                qDebug() << "Connection failed";
                break;
            default:
                break;
        }
    });

// Get connection info
QString ssid = wifi_mgr->getConnectedSSID();
QString ip = wifi_mgr->getIPAddress();
int signal = wifi_mgr->getSignalStrength();

qDebug() << "Connected to:" << ssid 
         << "IP:" << ip 
         << "Signal:" << signal << "%";
```

### Saved Networks

```cpp
// Get saved networks
auto saved = wifi_mgr->getSavedNetworks();
for (const auto& ssid : saved) {
    qDebug() << "Saved:" << ssid;
}

// Forget a network
wifi_mgr->forgetNetwork("OldNetwork");
```

## Bluetooth System Usage

### Bluetooth Device Discovery

```cpp
#include "core/hal/wireless/BluetoothManager.h"

auto bt_mgr = std::make_shared<BluetoothManagerImpl>();
bt_mgr->initialise();

// Enable Bluetooth
if (!bt_mgr->isEnabled()) {
    bt_mgr->setEnabled(true);
}

// Start device discovery
bt_mgr->startDiscovery();

// Listen for discovered devices
connect(bt_mgr.get(), &BluetoothManager::deviceDiscovered,
    this, [](const BluetoothManager::BluetoothDevice& device) {
        qDebug() << "Found device:" << device.name 
                 << "Signal:" << device.rssi << "dBm"
                 << "Type:" << static_cast<int>(device.type);
    });

// Get all discovered devices
auto discovered = bt_mgr->getDiscoveredDevices();
```

### Bluetooth Pairing and Connection

```cpp
// Pair with a device
bool paired = bt_mgr->pair("AA:BB:CC:DD:EE:FF");

if (paired) {
    // Connect after pairing
    bt_mgr->connect("AA:BB:CC:DD:EE:FF");
}

// Listen for pairing events
connect(bt_mgr.get(), &BluetoothManager::devicePaired,
    this, [](const QString& address) {
        qDebug() << "Paired with:" << address;
    });

// Get paired devices
auto paired_devices = bt_mgr->getPairedDevices();
for (const auto& device : paired_devices) {
    qDebug() << "Paired:" << device.name << device.address;
}
```

### Bluetooth Audio Profiles

```cpp
// Connect audio profile (A2DP for music)
bt_mgr->connectAudio("AA:BB:CC:DD:EE:FF", 
    BluetoothManager::AudioProfile::A2DP);

// Or HFP for hands-free calling
bt_mgr->connectAudio("AA:BB:CC:DD:EE:FF",
    BluetoothManager::AudioProfile::HFP);

// Listen for audio connection
connect(bt_mgr.get(), &BluetoothManager::audioConnected,
    this, [](const QString& address, BluetoothManager::AudioProfile profile) {
        QString profile_name = (profile == BluetoothManager::AudioProfile::A2DP)
            ? "A2DP" : "HFP";
        qDebug() << "Audio connected:" << address << profile_name;
    });

// Get connected devices with their audio profiles
auto connected = bt_mgr->getConnectedDevices();
for (const auto& device : connected) {
    if (device.connected) {
        qDebug() << "Connected:" << device.name;
        for (const auto& profile : device.supported_profiles) {
            qDebug() << "  Profile:" << static_cast<int>(profile);
        }
    }
}
```

## Android Auto Usage

### USB Device Discovery and Connection

```cpp
#include "core/services/android_auto/AndroidAutoService.h"

auto aa_service = std::make_shared<AndroidAutoServiceImpl>();
aa_service->initialise();

// Start searching for devices
aa_service->startSearching();

// Listen for found devices
connect(aa_service.get(), &AndroidAutoService::deviceFound,
    this, [aa_service](const AndroidAutoService::AndroidDevice& device) {
        qDebug() << "Found device:" << device.model 
                 << "Serial:" << device.serial
                 << "Android:" << device.android_version;
        
        // Auto-connect to first device found
        aa_service->connectToDevice(device.serial);
    });

// Listen for connection events
connect(aa_service.get(), &AndroidAutoService::connected,
    this, [](const AndroidAutoService::AndroidDevice& device) {
        qDebug() << "Connected to:" << device.model;
    });

// Listen for disconnection
connect(aa_service.get(), &AndroidAutoService::disconnected,
    this, []() {
        qDebug() << "Disconnected from Android device";
    });
```

### Video Frame Handling

```cpp
// Listen for video frames
connect(aa_service.get(), 
    QOverload<int, int, const uint8_t*, int>::of(
        &AndroidAutoService::videoFrameReady),
    this, [](int width, int height, const uint8_t* data, int size) {
        qDebug() << "Video frame:" << width << "x" << height 
                 << "Size:" << size << "bytes";
        
        // Render frame to screen
        // renderVideoFrame(data, size, width, height);
    });

// Configure display resolution
aa_service->setDisplayResolution(QSize(1024, 600));

// Set video framerate
aa_service->setFramerate(30);
```

### Touch Input

```cpp
// Send touch down event (normalized coordinates 0.0-1.0)
aa_service->sendTouchInput(0.5, 0.3, 0);  // action=0 for DOWN

// Send touch move event
aa_service->sendTouchInput(0.5, 0.35, 2);  // action=2 for MOVE

// Send touch up event
aa_service->sendTouchInput(0.5, 0.4, 1);  // action=1 for UP
```

### Key Input

```cpp
// Send key codes (Android key codes)
// Common codes: 4=BACK, 3=HOME, 27=DPAD_CENTER

// Back button
aa_service->sendKeyInput(4, 0);  // DOWN
aa_service->sendKeyInput(4, 1);  // UP

// Home button
aa_service->sendKeyInput(3, 0);  // DOWN
aa_service->sendKeyInput(3, 1);  // UP
```

### Audio Handling

```cpp
// Listen for audio data
connect(aa_service.get(), &AndroidAutoService::audioDataReady,
    this, [](const QByteArray& data) {
        qDebug() << "Audio data received:" << data.size() << "bytes";
        // Send to audio device
        // audio_output->write(data);
    });

// Request/abandon audio focus
aa_service->requestAudioFocus();
// ... use audio ...
aa_service->abandonAudioFocus();
```

### Performance Monitoring

```cpp
// Listen for statistics updates
connect(aa_service.get(), &AndroidAutoService::statsUpdated,
    this, [](int fps, int latency, int dropped_frames) {
        qDebug() << "Stats - FPS:" << fps 
                 << "Latency:" << latency << "ms"
                 << "Dropped:" << dropped_frames;
        
        // Update UI display
        ui->fpsLabel->setText(QString::number(fps));
        ui->latencyLabel->setText(QString::number(latency) + "ms");
    });

// Get metrics directly
int frame_drops = aa_service->getFrameDropCount();
int latency = aa_service->getLatency();

if (latency > 100) {
    qWarning() << "High latency detected:" << latency << "ms";
}
```

## Event Bus Integration

### Publishing Events from Core

```cpp
#include "EventBus.h"

auto event_bus = EventBus::getInstance();

// Publish audio event
QJsonObject audio_payload;
audio_payload["master"] = 75;
event_bus->publish("audio/volume", audio_payload);

// Publish video event
QJsonObject video_payload;
video_payload["value"] = 80;
event_bus->publish("video/brightness", video_payload);

// Publish Android Auto status
QJsonObject aa_status;
aa_status["connected"] = true;
aa_status["device"] = "Pixel 6";
aa_status["fps"] = 30;
aa_status["latency"] = 45;
event_bus->publish("androidauto/status", aa_status);
```

### Listening for Events in UI

```qml
// In QML component
Connections {
    target: wsClient
    
    function onEventReceived(topic, payload) {
        if (topic === "audio/volume") {
            volumeSlider.value = payload.master;
        } else if (topic === "video/brightness") {
            brightnessSlider.value = payload.value;
        } else if (topic === "androidauto/status") {
            statusText.text = payload.device + " (FPS: " + payload.fps + ")";
        }
    }
}
```

## Error Handling Best Practices

```cpp
// Check initialisation status
if (!audio_mgr->initialise()) {
    qCritical() << "Audio initialisation failed";
    // Fallback to speaker only
    return;
}

// Validate connections
if (!wifi_mgr->connect("Network", "password", WiFiManager::Security::WPA2)) {
    qWarning() << "WiFi connection failed";
    // Show error to user, allow retry
    return;
}

// Handle async errors
connect(audio_mgr.get(), &AudioManager::errorOccurred,
    this, [](const QString& error) {
        qCritical() << "Audio error:" << error;
        // Handle error appropriately
    });

// Graceful shutdown
void shutdown() {
    audio_mgr->deinitialise();
    video_mgr->deinitialise();
    wifi_mgr->deinitialise();
    bt_mgr->deinitialise();
    if (aa_service) {
        aa_service->deinitialise();
    }
}
```

## Threading Considerations

```cpp
// Audio/Video operations are blocking
// Run on separate thread to avoid UI freeze

QThread* audio_thread = new QThread();
audio_mgr->moveToThread(audio_thread);

connect(audio_thread, &QThread::started, audio_mgr.get(), [audio_mgr]() {
    audio_mgr->initialise();
});

audio_thread->start();

// Connect signals to update UI on main thread
connect(audio_mgr.get(), &AudioManager::volumeChanged,
    this, [this](int volume) {
        // This runs on main thread
        updateVolumeUI(volume);
    }, Qt::QueuedConnection);
```

---

**Remember**: Always check return values, handle errors gracefully, and clean up resources with `deinitialise()` methods.
