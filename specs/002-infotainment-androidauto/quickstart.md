# Quickstart — Developer guide (Infotainment + Android Auto)

This quickstart shows how to build and run the core, UI and a sample extension locally for development. Use WSL on Windows or a Linux development environment for consistent behaviour.

## Prerequisites

### System Requirements
- **OS**: Ubuntu 22.04 LTS or later (WSL2 or native Linux)
- **RAM**: 2 GB minimum (4 GB recommended for build)
- **Disk**: 6 GB free space
- **CPU**: 2 cores minimum (4+ recommended for faster builds)

### Install Dependencies

```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    pkg-config \
    qt6-base-dev \
    qt6-declarative-dev \
    qt6-websockets-dev \
    libsqlite3-dev \
    libpipewire-0.3-dev \
    libpulse-dev
```

**Packages Installed**:
- **Build tools**: gcc, g++, make, cmake, ninja-build, pkg-config
- **Qt6**: qt6-base-dev (Core, Gui, Widgets), qt6-declarative-dev (QML, Quick), qt6-websockets-dev
- **Audio**: libpipewire-0.3-dev (recommended), libpulse-dev (fallback)
- **Database**: libsqlite3-dev (preferences and session persistence)

**Build Time Estimates** (8 cores):
- Core service: ~1.5 minutes
- UI application: ~2 minutes
- Full build with tests: ~4.5 minutes

## Build (development)

1. Configure and build core and UI (from repo root):

```bash
cd /mnt/c/Users/matth/install/repos/opencardev/oct_2025/crankshaft-mvp
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target core -j
cmake --build build --target ui -j
```

## Run the Application

### 1. Run core (starts WebSocket server on port 9000 by default):

```bash
export CRANKSHAFT_WS_PORT=9000
./build/core/crankshaft-core --ws-port $CRANKSHAFT_WS_PORT
```

The core daemon will log startup timing information. Look for `[STARTUP]` log entries showing elapsed time at each initialization stage. Total startup time is logged when the core is ready.

### 2. Run UI (connects to core's WebSocket):

```bash
./build/ui/crankshaft-ui --ws ws://localhost:9000
```

The UI will display the Home screen with tiles for:
- **Navigation** - GPS and route planning (placeholder)
- **Phone** - Calls and contacts (placeholder)
- **Media** - Music and audio controls
- **Android Auto** - Connect your Android device
- **Tools** - Utilities and diagnostics

The status bar at the top shows the WebSocket connection status (green = connected, red = disconnected). The current system time is displayed in the top-right corner of the Home screen.

The UI also logs startup timing with `[STARTUP]` markers. Total UI startup time is shown when the interface is ready.

### 3. Run a sample extension (process-isolated):

```bash
./specs/002-infotainment-androidauto/examples/sample-extension/run-sample-extension.sh --ws ws://localhost:9000
```

## Performance Benchmarking

### Cold-Start Benchmark

User Story 1 requires the system to boot to the Home screen within **10 seconds** from a cold start. To measure this performance:

```bash
cd tests/benchmarks
./benchmark_cold_start.sh [iterations]
```

The benchmark script will:
1. Launch the core daemon
2. Wait for the WebSocket server to be ready
3. Launch the UI client
4. Measure total time from core start to UI visible
5. Repeat for the specified number of iterations (default: 3)
6. Report average, minimum, and maximum times
7. Compare against the 10-second target

**Example output:**
```
═══════════════════════════════════════════════════════════
   Crankshaft Cold-Start Performance Benchmark
═══════════════════════════════════════════════════════════

Target:      ≤10.0 seconds
Iterations:  3
Platform:    vnc:size=1024x600,port=5901

─────────────────────────────────────────────────────────
Iteration 1 of 3

Starting core daemon...
Core ready in 1250ms
Starting UI client...
✓ Cold start: 3420ms (PASS)

[... iterations 2-3 ...]

═══════════════════════════════════════════════════════════
   Benchmark Results
═══════════════════════════════════════════════════════════

Iterations:  3
Average:     3215ms
Minimum:     3180ms
Maximum:     3420ms

Target:      ≤10000ms

Result: ALL PASSED (3/3)
```

### Startup Logging

Both core and UI executables include detailed startup timing logs with `[STARTUP]` markers showing elapsed time at each initialization phase:

**Core startup logs:**
- Configuration loaded
- Event bus initialized
- ProfileManager initialized
- WebSocket server listening
- ServiceManager initialized
- Services started
- Total startup time

**UI startup logs:**
- QGuiApplication created
- Command line parsed
- Translations loaded
- WebSocket client created
- QML engine created
- Context properties set
- QML module loaded
- Total UI startup time

These logs help identify bottlenecks during cold-start optimization.

## Android Auto (AA) Connection & Testing

User Story 2 adds Android Auto projection support with audio routing and session persistence. This section covers device setup, connection benchmarking, and integration testing.

### Android Auto Prerequisites

#### Device Requirements
- **Android device:** Android 5.1 (API 22) or higher with Google Play Services
- **AndroidAuto version:** 4.0.584 or compatible (auto-updated via Google Play)
- **USB cable:** For initial pairing and fallback connection (USB OTG cable for vehicle display)
- **Wireless hardware:** Bluetooth 4.0+ for wireless pairing (WiFi optional)
- **Tested devices:** Pixel 3/4/5/6 series, Samsung Galaxy S series (verified in development)

#### System Requirements
- PipeWire (recommended) or PulseAudio daemon running for audio routing
- SQLite3 development packages for session persistence
- Port 5037 available for ADB (Android Debug Bridge) if debugging USB connections
- Ports 5035-5037 available for wireless AA discovery (mDNS on 5353 by default)

### Setting Up USB Connection

1. **Enable Developer Mode** on Android device:
   - Open **Settings > About Phone**
   - Tap **Build Number** 7 times (until you see "You are now a developer!")
   - Return to **Settings > System > Developer Options**
   - Enable **USB Debugging** and **Install via USB**

2. **Prepare USB Connection:**
   - Connect Android device via USB cable to development machine
   - Accept any prompts on device to allow debugging
   - Verify connection: `adb devices` should list your device

3. **Enable Android Auto over USB:**
   - Start Crankshaft core and UI
   - Open Android Auto app on device
   - Tap **Settings > Connected Car** and select your Crankshaft instance
   - Android Auto should detect the vehicle system via AOAP (Android Open Accessory Protocol)
   - The device will request permission - **tap Allow**

4. **Verify Connection:**
   - Check core logs for `[AndroidAutoService] AA session established: device_id=<id>`
   - UI shows Android Auto tile with status indicator (blue = connected)
   - Audio channels should route automatically through vehicle audio system

### Setting Up Wireless (WiFi Direct) Connection

For wireless Android Auto, devices must be on the same network or using WiFi Direct:

1. **Enable Wireless AA** on Android device:
   - Open Android Auto settings
   - Toggle **Wireless Projection** on
   - Device will advertise via mDNS (`_aap._tcp.local`)

2. **Configure Crankshaft for Wireless:**
   - Core automatically discovers AA devices via mDNS
   - Check logs for `[AndroidAutoService] AA device discovered via mDNS: <device_name>`
   - Optional: Edit `config/androidauto.json`:
     ```json
     {
       "wireless_enabled": true,
       "mdns_discovery": true,
       "discovery_timeout_ms": 5000,
       "auto_connect": false
     }
     ```

3. **Complete Wireless Pairing:**
   - First wireless connection requires verification
   - Device shows PIN on screen
   - Crankshaft core requests PIN: `[AndroidAutoService] Wireless pairing PIN required`
   - Enter PIN via core CLI or web interface
   - Device connects and is remembered for future sessions

4. **Troubleshooting Wireless Connection:**
   - Verify both systems on same WiFi network or WiFi Direct capable
   - Check firewall rules: ports 5035-5037 must be open (or use --no-firewall-check flag during development)
   - Restart mDNS if unavailable: `systemctl restart avahi-daemon` (Linux) or enable via Settings (Raspberry Pi OS)
   - Check core logs for `[AndroidAutoService]` entries: `grep "AndroidAutoService" /tmp/crankshaft-core.log`

### Android Auto Connection Benchmark

User Story 2 requires AA device connection to complete **within 15 seconds** from device detection to projection visible on screen. Measure this performance:

```bash
cd tests/benchmarks
./benchmark_aa_connect.sh [iterations]
```

The benchmark script:
1. Launches core daemon with WebSocket on configurable port
2. Simulates AA device connection via mock mode or actual USB/wireless device
3. Monitors logs for AA session established event (`android-auto/status/connected` WebSocket event)
4. Measures elapsed time from connection attempt to session ready
5. Repeats for specified iterations (default: 3)
6. Reports average, minimum, maximum times with pass/fail status

**Configuration (environment variables):**
```bash
# Run with 5 iterations and custom target time
ITERATIONS=5 TARGET_AA_CONNECT_MS=12000 ./benchmark_aa_connect.sh

# Enable mock mode (no physical device required)
CRANKSHAFT_AA_MOCK=true ./benchmark_aa_connect.sh

# Custom WebSocket port (if port 9000 unavailable)
WS_PORT=9002 ./benchmark_aa_connect.sh
```

**Example output:**
```
═══════════════════════════════════════════════════════════
   Android Auto Connection Performance Benchmark
═══════════════════════════════════════════════════════════

Target:      ≤15.0 seconds
Iterations:  3
Mode:        USB (default) / Wireless / Mock

─────────────────────────────────────────────────────────
Iteration 1 of 3

Starting core daemon...
Waiting for device connection...
Core ready in 1250ms
Device detected: Pixel 5 (Android 12)
✓ AA connection: 4230ms (PASS)

[... iterations 2-3 ...]

═══════════════════════════════════════════════════════════
   Benchmark Results
═══════════════════════════════════════════════════════════

Iterations:  3
Average:     4150ms
Minimum:     4120ms
Maximum:     4230ms

Target:      ≤15000ms

Result: ALL PASSED (3/3)
```

### Android Auto Session Persistence & Testing

Session state (device info, connection history, audio routing config) persists in SQLite database. Verify integration with automated tests:

```bash
# Build and run AA lifecycle integration tests
cd build
cmake --build . --target test_aa_lifecycle -j
ctest --test-dir . -R "AALifecycleTest" --verbose
```

This test suite covers:
- **Device creation & persistence:** AndroidDevice record created on first connection, survives core restart
- **Session state transitions:** Full state machine (NEGOTIATING → ACTIVE → SUSPENDED → ACTIVE → ENDED) with proper timestamps
- **Heartbeat tracking:** Session heartbeat updated every ~30s during projection, timestamp persists in database
- **Device reconnection:** Same device reconnects, previous session cleaned up, new session created
- **Error handling:** Failed connections logged with ERROR state, retries scheduled
- **Concurrent sessions:** Multiple devices can connect simultaneously, each session tracked independently
- **Cleanup:** Sessions properly ended with closure timestamp when device disconnects

**Example test output:**
```
Testing Android Auto Session Lifecycle...
┌─ testCreateAndroidDevice................ PASS (Device persistent across restarts)
├─ testDeviceStateTransitions............ PASS (State machine: NEGOTIATING → ACTIVE → ENDED)
├─ testSessionHeartbeat.................. PASS (Heartbeat updates every 30s)
├─ testSessionReconnection............... PASS (Same device reconnects with new session)
├─ testSessionErrorState................ PASS (Error state logged and handled)
├─ testMultipleConcurrentSessions....... PASS (Multiple devices tracked independently)
├─ testDeviceLastSeenUpdate............ PASS (Last-seen timestamp updated on connection)
└─ testEndSession....................... PASS (Session cleanup with closure timestamp)

Result: 8/8 PASSED (All session persistence verified)
```

### Android Auto Audio Routing

Audio from Android Auto is automatically routed to vehicle audio outputs via PipeWire (preferred) or PulseAudio:

#### Audio Stream Roles
- **MEDIA** — Music, podcasts, audiobooks (ducked during guidance)
- **GUIDANCE** — Navigation directions (high priority, ducks media)
- **SYSTEM_AUDIO** — Notifications, alerts (ducks media, lower than guidance)

#### Audio Device Selection (Automatic)
1. **Media:** Prefer Bluetooth headphones or external speakers
2. **Guidance:** Primary audio output (vehicle speaker)
3. **System:** Primary audio output (for alerts/ringtones)

#### Audio Ducking
When navigation guidance plays:
- Media volume automatically reduced to 40% of original level
- Other streams remain at full volume
- Ducking released when guidance finishes
- User can override via volume controls in UI

#### Troubleshooting Audio Routing
- **No audio from AA:** Check `[AudioRouter]` logs for backend detection failures
- **Audio to wrong device:** Verify primary audio device in system settings (`pactl list`)
- **Ducking not working:** Check guidance audio stream is being routed (logs should show `[AudioRouter] Routing GUIDANCE audio`)

## Development Notes

- Use `contracts/ws-schema.json` to validate WebSocket messages during development; production deployments should run the WebSocket server with TLS (wss) once certificates are provisioned.
- The Home screen automatically updates the system clock every second
- All UI components follow Design for Driving guidelines with appropriate touch target sizes and contrast ratios
- The application supports light and dark themes, toggled via Settings
- Translations are available for en-GB (default) and de-DE

---

## Troubleshooting

### Build Failures

**Issue**: `ninja: command not found`  
**Solution**: Install ninja-build: `sudo apt-get install ninja-build`

**Issue**: CMake cannot find Qt6 packages  
**Solution**: Ensure Qt6 development packages are installed:
```bash
sudo apt-get install qt6-base-dev qt6-declarative-dev qt6-websockets-dev
```

**Issue**: Missing PipeWire headers  
**Solution**: Install PipeWire development package:
```bash
sudo apt-get install libpipewire-0.3-dev
```
Alternatively, use PulseAudio: `sudo apt-get install libpulse-dev`

### Runtime Failures

**Issue**: VNC connection refused  
**Solution**: Check if port 5900 is already in use:
```bash
lsof -i :5900
```
Kill conflicting process or use different port: `-platform vnc:size=1024x600,port=5901`

**Issue**: Android Auto device not detected  
**Solution**:
1. Check USB connection: `lsusb` (should show phone)
2. Enable USB debugging on Android device
3. Check dmesg for USB errors: `dmesg | tail -n 50`

**Issue**: High memory usage (>1 GB)  
**Solution**: Check for memory leaks:
```bash
valgrind --leak-check=full ./build/core/crankshaft-core
```

### Performance Issues

**Issue**: Cold start takes >10 seconds  
**Solution**: 
1. Check system load: `top` (CPU should be <80%)
2. Verify Qt Quick Compiler is enabled (Release builds)
3. Disable debug logging: Remove `QT_DEBUG_PLUGINS=1`

**Issue**: Media playback stuttering  
**Solution**:
1. Check audio backend: Prefer PipeWire over PulseAudio
2. Verify CPU isn't throttling: `cat /sys/devices/system/cpu/cpu*/cpufreq/scaling_cur_freq`
3. Check latency metrics in performance dashboard

For additional support, see [TROUBLESHOOTING.md](../../docs/TROUBLESHOOTING.md) or file an issue on GitHub.

