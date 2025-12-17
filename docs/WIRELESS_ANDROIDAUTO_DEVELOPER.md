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

# Wireless Android Auto Implementation

## Overview

Crankshaft now supports wireless Android Auto connections over TCP/IP in addition to the standard USB transport. This feature is particularly useful for:

- **PC Development**: Test Android Auto on development machines without USB hardware
- **Network Testing**: Test connectivity over WiFi networks
- **Debugging**: Easier to inspect and log network traffic
- **Multi-Device**: Connect to phones on the same network without physical USB cables

## Architecture

### Transport Modes

The Android Auto service supports three connection modes:

1. **Auto** (default)
   - Attempts USB connection first
   - Falls back to wireless if USB device not found
   - Requires both USB and wireless settings to be configured

2. **USB**
   - USB-only connection
   - Requires physical USB cable
   - Uses libusb for device enumeration and AOAP protocol

3. **Wireless**
   - TCP/IP connection over network (port 5277)
   - No USB cable required
   - Requires device hostname/IP and network connectivity

### AASDK Transport Layers

The implementation uses AASDK's transport abstraction:

```
┌─────────────────────────────────────────┐
│      AndroidAutoService                 │
├─────────────────────────────────────────┤
│  configureTransport(settings)           │
│  - Reads connectionMode, wireless.*     │
│  - Sets m_transportMode, m_wirelessHost │
├─────────────────────────────────────────┤
│  startSearching()                       │
│  - USB mode: USBHub device enumeration  │
│  - Wireless: setupTCPTransport()        │
├─────────────────────────────────────────┤
│      ITransport                         │
│  ├─ USBTransport (libusb)              │
│  └─ TCPTransport (ASIO)                │
├─────────────────────────────────────────┤
│  Messenger → Channels → Android Phone   │
└─────────────────────────────────────────┘
```

## Configuration

### ProfileManager Settings

Android Auto device configuration in host profile:

```cpp
DeviceConfig androidAutoDevice;
androidAutoDevice.name = "AndroidAuto";
androidAutoDevice.type = "AndroidAuto";
androidAutoDevice.enabled = true;
androidAutoDevice.useMock = false;

// Connection mode: "auto" | "usb" | "wireless"
androidAutoDevice.settings["connectionMode"] = "auto";

// Wireless settings (optional, used when wireless mode active)
androidAutoDevice.settings["wireless.enabled"] = false;
androidAutoDevice.settings["wireless.host"] = "";      // e.g., "192.168.1.100"
androidAutoDevice.settings["wireless.port"] = 5277;    // Android Auto standard port
```

### JSON Configuration File

In profile configuration (e.g., host profiles):

```json
{
  "AndroidAuto": {
    "name": "AndroidAuto",
    "type": "AndroidAuto",
    "enabled": true,
    "useMock": false,
    "connectionMode": "auto",
    "wireless": {
      "enabled": false,
      "host": "192.168.1.100",
      "port": 5277
    },
    "channels": {
      "video": true,
      "mediaAudio": true,
      "systemAudio": true,
      "speechAudio": true,
      "microphone": true,
      "input": true,
      "sensor": true,
      "bluetooth": false
    }
  }
}
```

## Usage Examples

### Example 1: USB-Only (Default)

```cpp
// In profile:
androidAutoDevice.settings["connectionMode"] = "usb";
```

**Flow:**
1. ServiceManager creates RealAndroidAutoService
2. Service calls `configureTransport()` - sets `m_transportMode = USB`
3. `startSearching()` triggers USB hub to detect devices
4. Device detection initiates AOAP protocol sequence

### Example 2: Wireless-Only (PC Development)

```cpp
// In profile:
androidAutoDevice.settings["connectionMode"] = "wireless";
androidAutoDevice.settings["wireless.enabled"] = true;
androidAutoDevice.settings["wireless.host"] = "192.168.1.100";  // Phone IP
androidAutoDevice.settings["wireless.port"] = 5277;
```

**Flow:**
1. ServiceManager creates RealAndroidAutoService
2. Service calls `configureTransport()` - sets `m_transportMode = Wireless`
3. `startSearching()` attempts TCP connection to 192.168.1.100:5277
4. Phone must have Android Auto wireless mode enabled
5. Upon successful connection, channels are established over TCP

### Example 3: Auto Mode with Fallback

```cpp
// In profile:
androidAutoDevice.settings["connectionMode"] = "auto";
androidAutoDevice.settings["wireless.enabled"] = true;
androidAutoDevice.settings["wireless.host"] = "192.168.1.100";
```

**Flow:**
1. ServiceManager creates RealAndroidAutoService
2. Service calls `configureTransport()` - sets `m_transportMode = Auto`
3. `startSearching()` checks USB first
4. If no USB device found, attempts wireless connection
5. Logs indicate which transport was used

## Phone Configuration

### Android 9+: Enable Wireless Android Auto

1. **On Phone:**
   - Settings → Apps & notifications → Special app access → Display over other apps
   - Find Android Auto → Enable permission
   - Settings → Connected devices → Connection preferences → Android Auto
   - Enable "Wireless projection" option

2. **Network Requirements:**
   - Phone and Crankshaft must be on same WiFi network
   - Network should support mDNS (.local hostname resolution) or use IP directly
   - Port 5277 must not be blocked by firewall

### Debugging Phone Connection

```bash
# From Crankshaft device, verify connectivity:
ping 192.168.1.100
telnet 192.168.1.100 5277

# Check AASDK logs:
QT_LOGGING_RULES="*=true" ./crankshaft-ui
journalctl -u crankshaft-core -n 100 -f | grep -i "wireless\|tcp\|transport"
```

## Implementation Details

### ServiceManager Integration

When starting AndroidAuto service:

```cpp
bool ServiceManager::startAndroidAutoService(const DeviceConfig& device) {
  // Create service instance
  m_androidAutoService = AndroidAutoService::create(...);
  
  // Configure transport with device settings
  m_androidAutoService->configureTransport(device.settings);
  
  // Initialize service
  if (!m_androidAutoService->initialise()) {
    // Handle error
  }
}
```

### RealAndroidAutoService::configureTransport()

Parses connection settings:

```cpp
void RealAndroidAutoService::configureTransport(const QMap<QString, QVariant>& settings) {
  // 1. Parse connection mode
  QString mode = settings.value("connectionMode", "auto").toString().toLower();
  
  // 2. Set transport mode (USB/Wireless/Auto)
  if (mode == "usb") m_transportMode = TransportMode::USB;
  else if (mode == "wireless") m_transportMode = TransportMode::Wireless;
  else m_transportMode = TransportMode::Auto;
  
  // 3. Read wireless configuration
  m_wirelessHost = settings.value("wireless.host", "").toString();
  m_wirelessPort = settings.value("wireless.port", 5277).toUInt();
  m_wirelessEnabled = settings.value("wireless.enabled", false).toBool();
  
  // 4. Log configuration
  Logger::instance().info(QString("[RealAndroidAutoService] Wireless AA configured: %1:%2")
      .arg(m_wirelessHost).arg(m_wirelessPort));
}
```

### Transport Setup Methods

**USB Transport:**
```cpp
bool RealAndroidAutoService::setupUSBTransport() {
  // USB hub already initialized in setupAASDK()
  // This method confirms USB mode is selected
  return m_usbHub && m_ioService;
}
```

**Wireless Transport:**
```cpp
bool RealAndroidAutoService::setupTCPTransport(const QString& host, quint16 port) {
  // 1. Create TCP wrapper (ASIO-based)
  auto tcpWrapper = std::make_shared<aasdk::tcp::TCPWrapper>();
  
  // 2. Create socket
  auto socket = std::make_shared<boost::asio::ip::tcp::socket>(*m_ioService);
  
  // 3. Connect to phone
  boost::system::error_code ec = tcpWrapper->connect(*socket, host.toStdString(), port);
  if (ec) return false;
  
  // 4. Create TCP endpoint and transport
  auto tcpEndpoint = std::make_shared<aasdk::tcp::TCPEndpoint>(tcpWrapper, std::move(socket));
  m_transport = std::make_shared<aasdk::transport::TCPTransport>(*m_ioService, tcpEndpoint);
  
  // 5. Setup channels with TCP transport
  setupChannelsWithTransport();
  return true;
}
```

### startSearching() Flow

```cpp
bool RealAndroidAutoService::startSearching() {
  // Wireless mode check
  if (m_transportMode == TransportMode::Wireless || m_wirelessEnabled) {
    if (!m_wirelessHost.isEmpty()) {
      // Attempt TCP connection directly
      if (setupTCPTransport(m_wirelessHost, m_wirelessPort)) {
        emit connected(m_device);
        return true;
      }
    }
    return false;
  }
  
  // USB mode: start USB hub detection
  m_usbHub->start(...);
  return true;
}
```

## Logging and Debugging

### Enable Debug Logging

```bash
# Enable all logging
export QT_LOGGING_RULES='*=true'

# Enable specific component
export QT_LOGGING_RULES='crankshaft*=true;aasdk*=true'

# Check AASDK transport logs
AASDK_LOG_LEVEL=DEBUG ./crankshaft-ui
```

### Key Log Messages

**Wireless Configuration:**
```
[ServiceManager] Processing device: AndroidAuto (type: AndroidAuto, enabled: true, useMock: false)
[RealAndroidAutoService] Configuring transport mode: wireless
[RealAndroidAutoService] Wireless AA configured: 192.168.1.100:5277
```

**Connection Attempts:**
```
[RealAndroidAutoService] Starting wireless connection to 192.168.1.100:5277
[RealAndroidAutoService] Wireless connection established
[RealAndroidAutoService] TCP transport connected to 192.168.1.100:5277
```

**Fallback in Auto Mode:**
```
[RealAndroidAutoService] Transport mode: auto
[RealAndroidAutoService] Attempting USB connection first...
[RealAndroidAutoService] No USB device found, attempting wireless...
[RealAndroidAutoService] Wireless connection established
```

## Performance Considerations

### WiFi vs USB

| Aspect | USB | Wireless |
|--------|-----|----------|
| Bandwidth | 480 Mbps (USB 2.0) | 50-150 Mbps (WiFi) |
| Latency | <1ms | 10-50ms |
| Stability | Very stable | Network-dependent |
| Use Case | Production vehicles | Development/Testing |

### Optimization Tips

1. **Use 5GHz WiFi** for better throughput and lower latency
2. **Position devices close** to WiFi router to maintain signal strength
3. **Monitor network congestion** - other traffic can impact video quality
4. **Use mDNS hostname** (.local) to handle IP changes automatically
5. **Set appropriate resolution** - lower resolution reduces bandwidth

## Troubleshooting

### "No host configured" Error

**Problem:** Wireless mode selected but `wireless.host` is empty

**Solution:**
```cpp
androidAutoDevice.settings["wireless.host"] = "192.168.1.100";
// or use hostname if mDNS is available
androidAutoDevice.settings["wireless.host"] = "phone.local";
```

### Connection Timeout

**Problem:** TCP connection fails to establish

**Diagnosis:**
```bash
# 1. Check network connectivity
ping 192.168.1.100

# 2. Check port accessibility
telnet 192.168.1.100 5277

# 3. Verify wireless Android Auto is enabled on phone
# 4. Check firewall rules
sudo ufw status
sudo iptables -L | grep 5277
```

### Video Stuttering/Freezing

**Problem:** TCP transport experiencing network issues

**Solutions:**
1. Reduce video resolution: `settings["resolution"] = "800x480"`
2. Lower frame rate: `settings["fps"] = 20`
3. Move closer to WiFi router
4. Switch to USB (if available)
5. Check for WiFi interference on same channel

### "Wireless mode selected but no host configured"

**Problem:** Transport mode is wireless but host is empty string

**Solution:** Either set the host or change mode to "auto" or "usb"

## Future Enhancements

- [ ] mDNS auto-discovery of Android Auto devices on network
- [ ] Connection retry logic with exponential backoff
- [ ] Bandwidth adaptation for varying network conditions
- [ ] Simultaneous USB + Wireless connection support
- [ ] WebSocket UI for real-time transport/connection status
- [ ] Network quality of service (QoS) monitoring

## References

- [Android Auto Protocol (AASDK)](https://github.com/opencardev/aasdk)
- [Android Auto Wireless Requirements](https://support.google.com/maps/answer/6191595)
- [ASIO TCP Documentation](https://www.boost.org/doc/libs/1_76_0/doc/html/boost_asio.html)
- Crankshaft ProjectManager Settings
- AASDK Transport Layer Documentation
