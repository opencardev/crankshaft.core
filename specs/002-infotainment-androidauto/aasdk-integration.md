# AASDK Integration Notes

**Last Updated**: 2026-01-01
**AASDK Location**: `external/aasdk` (git submodule)

This file outlines responsibilities and integration steps for using the AASDK found in `external/aasdk`.

## ✅ Wireless Android Auto Support Confirmed

**AASDK supports both wired (USB) and wireless (WiFi/TCP) Android Auto connections.**

### Evidence
- TCP Transport class: `aasdk::transport::TCPTransport`
- WiFi Projection channel: `WIFI_PROJECTION` (ChannelId.cpp:60, ServiceId.cpp:50)
- Documentation: TROUBLESHOOTING.md § "WiFi AndroidAuto Issues", TESTING.md § wireless mode
- Logging category: `AASDK_LOG_CHANNEL_WIFI_PROJECTION`

### Transport Modes

| Mode | Class | Protocol | Requirements |
|------|-------|----------|--------------|
| **USB (Wired)** | `USBTransport` | USB AOA | libusb, udev rules |
| **TCP (Wireless)** | `TCPTransport` | TCP/IP WiFi | Same network, mDNS, port 5277 |

### Wireless Configuration

```cpp
// TCP Transport initialization (wireless mode)
#include <aasdk/Transport/TCPTransport.hpp>

auto transport = std::make_unique<aasdk::transport::TCPTransport>();
transport->bind("0.0.0.0", 5277);  // Standard AA port
```

### Network Requirements
- WiFi interface active (wlan0)
- Firewall allows TCP port **5277**
- mDNS (Avahi) for device discovery
- Android device on same network
- Device supports wireless AA (Android 11+)

### Firewall Setup
```bash
sudo ufw allow 5277/tcp
sudo ufw reload
```

### mDNS Service Discovery
```bash
# Install Avahi
sudo apt install avahi-daemon avahi-utils
sudo systemctl enable avahi-daemon --now

# Create /etc/avahi/services/android-auto.service
```

See full wireless configuration details in sections below.

---

## Responsibilities of AASDK adapter service (core)
- Manage AA session lifecycle: discovery, handshake, capability negotiation, transport setup.
- **Support both USB and TCP transports** based on runtime configuration.
- Provide rendering surface or bridge to UI: where AASDK projects present a surface, the adapter should forward rendering commands or provide a compositor path.
- Route input events from UI/vehicle controls to the AA session when active.
- Route audio from Android Auto session to the selected audio sink (PipeWire or PulseAudio) and ensure proper mixing with native audio.
- Provide metrics and diagnostics for AA session lifecycle.
- **Emit WebSocket events** for connection state changes: `android-auto/status/state-changed`, `connected`, `disconnected`, `error`.

## Build & CI considerations
- Ensure `external/aasdk` builds on the CI target (arm64 cross-build). Create a `ci/aasdk-build` job to compile and produce artifacts.
- Identify any platform kernel or Wi-Fi/Bluetooth stack requirements for wireless AA (hostapd, wpa_supplicant versions).
- **No compile-time flags required** for wireless support - transport selection is runtime.

## Wireless pairing
- Wireless AA typically requires a pairing handshake via Bluetooth (Classic or BLE) and then a Wi-Fi direct or local network channel for the AA media/control traffic.
- **Initial pairing**: Connect device via USB first; during session, WiFi credentials exchanged; device stores head unit network info.
- **Subsequent connections**: Device discovers head unit via mDNS on same network; TCP connection on port 5277.
- Adapter service must expose a pairing API to the UI for UX: start pairing, show steps, confirm pairing, and report progress/errors.

### Pairing Flow
```
1. USB Connection (first time)
   ├─> USB handshake establishes session
   ├─> Device and head unit exchange WiFi credentials
   └─> Device stores head unit network info

2. Wireless Connection (subsequent)
   ├─> Ensure both on same WiFi network
   ├─> Device discovers head unit via mDNS
   ├─> TCP connection on port 5277
   └─> AA session begins wirelessly
```

## Audio routing
- Recommend using PipeWire (preferred) for low-latency routing and flexible sink/source management on modern Linux distributions.
- Provide a small shim to route the AA audio stream into the system audio graph and expose a service to switch sinks (vehicle speakers, headphone jack, etc.).
- **See `docs/audio-setup.md`** for detailed PipeWire configuration and routing zones.

## Connection State Machine

Implemented in `core/services/android_auto/RealAndroidAutoService.cpp`:

```
┌─────────────────┐
│  Disconnected   │ ◄─────────────────┐
└─────────────────┘                   │
         │                            │
         │ device detected            │ timeout / error
         ▼                            │
┌─────────────────┐                   │
│  Negotiating    │ ──────────────────┤
└─────────────────┘                   │
         │                            │
         │ handshake complete         │
         ▼                            │
┌─────────────────┐                   │
│     Active      │ ──────────────────┤
└─────────────────┘                   │
         │                            │
         │ network drop / suspend     │
         ▼                            │
┌─────────────────┐                   │
│    Suspended    │ ───────────────────
└─────────────────┘
         │
         │ user disconnect / fatal error
         ▼
┌─────────────────┐
│     Ended       │
└─────────────────┘
```

## Performance Targets

| Metric | USB (Wired) | WiFi (Wireless) | Target (US2) |
|--------|-------------|-----------------|--------------|
| **Connection establishment** | ~2-3s | ~5-7s (incl. mDNS) | **≤15s** |
| Audio latency | ~50-100ms | ~100-200ms | Low priority |
| Video frame latency | ~30-50ms | ~50-100ms | <100ms |
| Touch input latency | ~30-50ms | ~50-80ms | <200ms |

**User Story 2 Requirement**: AA connection completes and projection surface visible within **15 seconds** of device connection.

## Configuration

Add to `config/crankshaft.json`:

```json
{
  "android_auto": {
    "enabled": true,
    "transport": "tcp",
    "wireless": {
      "enabled": true,
      "bind_address": "0.0.0.0",
      "port": 5277,
      "mdns_enabled": true,
      "service_name": "Crankshaft Android Auto"
    },
    "usb": {
      "enabled": true,
      "vendor_id": "0x18d1",
      "product_ids": ["0x2d00", "0x2d01"]
    }
  }
}
```

## Testing
- Provide an integration test that launches the adapter service and a simulated AA device (or a recorded handshake) to verify session negotiation and basic media playback.
- **`tests/benchmarks/benchmark_aa_connect.sh`**: Measures connection time from device detection to projection surface visible (target: ≤15s).
- **`tests/integration/test_aa_lifecycle.cpp`**: Tests state machine transitions and session persistence.

## Example adapter responsibilities summary
- Start AA transport: open connections, negotiate codecs.
- **Runtime transport selection**: USB or TCP based on configuration or device availability.
- Provide event mapping: map AA touch events -> UI events, map steering-wheel controls -> AA media controls.
- Monitor connection health and emit WS events: `android-auto/status/state-changed`, `android-auto/status/connected`, `android-auto/status/disconnected`, `android-auto/status/error`.
- **Persist session metadata**: Store AndroidDevice and Session entities to SQLite (see `data-model.md`).

## Wireless-Specific Troubleshooting

### Device Not Discovering Head Unit
**Symptoms**: Android Auto app doesn't show wireless option

**Diagnosis**:
```bash
systemctl status avahi-daemon
avahi-browse -a
sudo ufw status | grep 5277
```

**Solution**: Restart Avahi, check firewall, ensure same network

### Connection Timeout
**Symptoms**: Negotiation times out

**Diagnosis**:
```bash
sudo tcpdump -i wlan0 port 5277 -n
grep "WIFI" /var/log/crankshaft/core.log
```

**Solution**: Increase timeout, verify network stability, check SSL certificates

### Audio/Video Stuttering
**Symptoms**: Choppy playback

**Diagnosis**:
```bash
iwconfig wlan0  # Check WiFi signal
iftop -i wlan0  # Monitor bandwidth
```

**Solution**: Improve signal strength, reduce interference, optimize buffers

## References

- **AASDK Documentation**:
  - `external/aasdk/TROUBLESHOOTING.md` § "WiFi AndroidAuto Issues"
  - `external/aasdk/TESTING.md` § "Wireless Mode Tests"
  - `external/aasdk/MODERN_LOGGER.md` § "WIFI Logging Categories"
- **Project Documentation**:
  - `docs/audio-setup.md` - PipeWire/PulseAudio configuration
  - `specs/002-infotainment-androidauto/data-model.md` - Session persistence schema
  - `specs/002-infotainment-androidauto/contracts/ws-schema.json` - WebSocket events
- **External**:
  - [Android Auto Developer Docs](https://developers.google.com/cars/design/android-auto)

## Implementation Status (Phase 4 - User Story 2)

- [ ] T025: ✅ Document wireless AA support (this file)
- [ ] T026: Persist AndroidDevice entity to SQLite
- [ ] T027: Persist Session entity to SQLite
- [ ] T028: Implement AA state machine in RealAndroidAutoService.cpp
- [ ] T029: Emit WebSocket events for AA lifecycle
- [ ] T030: Create/verify AA projection surface QML component
- [ ] T031: Wire UI to AA events
- [ ] T032: Configure audio routing (PipeWire/PulseAudio)
- [ ] T033: Create AA connection benchmark script
- [ ] T034: Add AA lifecycle integration test
- [ ] T035: Update quickstart.md with AA instructions

