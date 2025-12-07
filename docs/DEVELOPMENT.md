# Crankshaft MVP - Development Guide

## Prerequisites

### Linux / WSL2

- Ubuntu 20.04+ or Debian 11+
- CMake 3.16+
- Qt6 6.5.3+ (Core, Network, WebSockets, Gui, Qml, Quick)
- Catch2 (automatically fetched)
- clang-format (optional, for formatting)
- clang-tidy (optional, for linting)

### Install Dependencies (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install -y \
    build-essential \
    cmake \
    git \
    qt6-base-dev \
    qt6-websockets-dev \
    qt6-declarative-dev \
    libqt6websockets6-dev \
    qml6-module-qtquick \
    qml6-module-qtquick-controls \
    qml6-module-qtquick-layouts \
    clang-format \
    clang-tidy
```

---

## Building

### Quick Build (Linux/WSL)

```bash
# Debug build (default)
./scripts/build.sh

# Release build
./scripts/build.sh --build-type Release

# Build specific component
./scripts/build.sh --component ui

# Build and create DEB packages
./scripts/build.sh --build-type Release --package
```

### Build Script Options

```bash
./scripts/build.sh [OPTIONS]

Options:
  --build-type TYPE      Build configuration (Debug|Release) [default: Debug]
  --component COMP       Component to build (all|core|ui|tests) [default: all]
  --package              Create DEB packages after building [default: false]
  --help                 Display help message
```

### Windows (PowerShell)

```powershell
.\scripts\build.ps1 --build-type Debug
```

### Manual Build

```bash
# Configure
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug

# Build
cmake --build build --config Debug -j$(nproc)

# Create packages
cd build && cpack -G DEB
```

**Outputs:**
- Core: `build/core/crankshaft-core`
- UI: `build/ui/crankshaft-ui`
- Tests: `build/tests/test_eventbus`, `build/tests/test_websocket`
- Packages: `build/crankshaft-core_*.deb`, `build/crankshaft-ui_*.deb`

### Material Design Icons

The build system automatically downloads and includes the [Material Design Icons](https://materialdesignicons.com/) font (TTF format) during compilation. This font provides professional, consistent icons used throughout the UI.

**Font Details:**
- **Name:** Material Design Icons
- **Version:** 7.4.47+
- **Source:** https://github.com/Templarian/MaterialDesign
- **Installation:** Fonts are automatically installed to `/usr/share/fonts/truetype/material-design-icons/` when installing the DEB package
- **Development:** Font is cached in `build/ui/fonts/` during builds

**Using Icons in QML:**
The `Icon` component in `ui/qml/Components/Icon.qml` provides a convenient way to render icons:

```qml
import Crankshaft

Icon {
    name: "music"           // Icon name from Material Design Icons
    size: 24                // Icon size in pixels
    color: Theme.textColor  // Color
}
```

**Available Icon Names:**
Common icon names mapped to Material Design Icons include:
- Navigation: `navigation`, `arrow-left`, `arrow-right`, `menu`, `close`
- Media: `music`, `play`, `pause`, `stop`, `volume`, `mute`
- Communication: `phone`, `message`, `bluetooth`
- Home & Settings: `home`, `settings`, `gear`
- Status: `star`, `heart`, `check`, `error`
- Actions: `back`, `forward`, `refresh`, `search`, `add`, `remove`, `download`, `upload`, `info`, `help`

For a complete list of available icons, visit: https://materialdesignicons.com/

---

## Running

### Start Core Backend

```bash
./build/core/crankshaft-core
```

**Options:**
- `--port <number>` - WebSocket port (default: 8080)
- `--config <path>` - Configuration file (default: config/crankshaft.json)

**Example:**
```bash
./build/core/crankshaft-core --port 9000 --config my-config.json
```

### Start UI Frontend

In a separate terminal:

```bash
./build/ui/crankshaft-ui
```

**Options:**
- `--server <url>` - WebSocket server URL (default: ws://localhost:8080)
- `--language <locale>` - UI language (default: en-GB, options: en-GB, de-DE)

**Example:**
```bash
./build/ui/crankshaft-ui --server ws://localhost:9000 --language de-DE
```

---

## Testing

### Run All Tests

```bash
./scripts/run_tests.sh
```

Or manually:

```bash
cd build
ctest --output-on-failure
```

### Run Individual Tests

```bash
./build/tests/test_eventbus
./build/tests/test_websocket
```

### Test Coverage

Tests use Catch2 framework:

- **test_eventbus.cpp**: EventBus singleton, pub/sub, thread safety
- **test_websocket.cpp**: WebSocket connection, subscribe, publish, broadcast, multi-client

---

## Code Quality

### Format Code

```bash
./scripts/format.sh
```

Uses `.clang-format` (Google style, 100 char limit)

### Lint Code

```bash
./scripts/lint.sh
```

Uses `.clang-tidy` (modernize, bugprone, performance checks)

---

## Project Structure

```
crankshaft-mvp/
├── CMakeLists.txt              # Top-level build
├── .clang-format               # Code formatting rules
├── .clang-tidy                 # Static analysis rules
├── .gitignore                  # Git ignore rules
├── README.md                   # Quick start guide
├── config/
│   └── crankshaft.json         # Default configuration
├── core/                       # Core backend (C++)
│   ├── CMakeLists.txt
│   ├── main.cpp                # Entry point
│   ├── EventBus.h/.cpp         # Pub/sub message bus
│   ├── WebSocketServer.h/.cpp  # WebSocket server
│   ├── ConfigService.h/.cpp    # Configuration service
│   ├── Logger.h/.cpp           # Logging service
│   └── hal/                    # Hardware Abstraction Layer
│       ├── VehicleHAL.h/.cpp   # Vehicle hardware interface
│       ├── HostHAL.h/.cpp      # Host/system hardware interface
│       ├── DeviceHAL.h/.cpp    # Device hardware interfaces
│       ├── MockVehicleHAL.h/.cpp    # Mock vehicle for testing
│       ├── MockHostHAL.h/.cpp       # Mock host for testing
│       ├── MockDeviceHAL.h/.cpp     # Mock devices for testing
│       └── HALManager.h/.cpp   # HAL orchestrator (singleton)
├── ui/                         # UI frontend (Qt/QML)
│   ├── CMakeLists.txt
│   ├── main.cpp                # Entry point
│   ├── WebSocketClient.h/.cpp  # WebSocket client
│   ├── qml/
│   │   ├── qmldir              # QML module definition
│   │   ├── Theme.qml           # Theme singleton
│   │   ├── Main.qml            # Main window
│   │   ├── HomeScreen.qml      # Home screen
│   │   ├── SettingsScreen.qml  # Settings screen
│   │   └── Components/         # Reusable components
│   │       ├── AppButton.qml
│   │       ├── Card.qml
│   │       └── Icon.qml
│   ├── assets/
│   │   └── icons/
│   │       └── mdi.svg         # Material Design Icons
│   └── i18n/
│       ├── ui_en_GB.ts         # English translations
│       └── ui_de_DE.ts         # German translations
├── tests/                      # Unit tests (Catch2)
│   ├── CMakeLists.txt
│   ├── test_eventbus.cpp       # EventBus tests
│   └── test_websocket.cpp      # WebSocket tests
├── scripts/                    # Build/dev scripts
│   ├── build.sh                # Build script (Linux)
│   ├── build.ps1               # Build script (Windows)
│   ├── format.sh               # Format code
│   ├── lint.sh                 # Lint code
│   └── run_tests.sh            # Run tests
└── docs/                       # Documentation
    ├── ARCHITECTURE.md         # System architecture
    ├── API.md                  # API reference
    └── DEVELOPMENT.md          # This file
```

---

## Hardware Abstraction Layer (HAL)

Crankshaft uses a three-tier Hardware Abstraction Layer inspired by Android Automotive to provide flexible hardware integration.

### Overview

The HAL system consists of:

1. **Vehicle HAL** - Vehicle hardware interface (speed, fuel, engine, climate, etc.)
2. **Host HAL** - System/host hardware interface (time, device info, connectivity, etc.)
3. **Device HAL** - Hardware device interfaces (CAN bus, I2C, GPIO, UART, GPS, etc.)
4. **HAL Manager** - Singleton orchestrator managing all HALs

### Quick Start

The system uses realistic **mock implementations by default** for development and testing:

```cpp
// In core/main.cpp, HALManager is already initialized
HALManager &halManager = HALManager::instance();
// Mock Vehicle/Host/Device HALs created automatically

// Get vehicle property
QVariant speed = halManager.getVehicleProperty(VehicleHAL::PropertyType::Speed);

// Subscribe to property changes (automatically broadcast to EventBus)
QObject::connect(&halManager, &HALManager::vehiclePropertyChanged,
    [](const VehicleHAL::PropertyType &type, const QVariant &value) {
        // Property change published to EventBus → WebSocket → UI
    });
```

### Mock Implementations

**MockVehicleHAL** provides realistic driving simulation:
- Speed ranges 0-120 km/h with 25-second acceleration/deceleration cycles
- Fuel consumption linked to speed
- Location tracking with circular driving pattern
- Engine temperature variations
- Odometer and trip distance tracking

**MockHostHAL** simulates system properties:
- System time updates every second
- Memory usage with ±10 MB variations
- CPU temperature with ±2°C variations
- Connectivity status (WiFi, Bluetooth, GPS)

**MockDeviceHAL** simulates hardware devices:
- CAN Bus (500 kbps), I2C, GPIO, UART, GPS, etc.
- State transitions: Offline → Connecting → Online
- Data send/receive with realistic latencies

### Extending with Real Hardware

To implement a real hardware HAL (e.g., CAN-based vehicle data):

1. **Create custom class** inheriting from base HAL:
   ```cpp
   class CANVehicleHAL : public VehicleHAL {
       // Override: initialize(), shutdown(), getProperty(), setProperty(), etc.
   };
   ```

2. **Register with HALManager**:
   ```cpp
   auto canHal = std::make_shared<CANVehicleHAL>();
   canHal->initialize();
   HALManager::instance().setVehicleHAL(canHal);
   ```

3. **Add to CMakeLists.txt** and rebuild

For detailed implementation guide, see [HAL_ARCHITECTURE.md](HAL_ARCHITECTURE.md).

### EventBus Integration

HAL property changes automatically flow to the UI:

```
HALManager
    ↓ (propertyChanged signal)
EventBus (publishes "hal/vehicle/Speed", "hal/host/CPUTemperature", etc.)
    ↓
WebSocketServer (broadcasts to clients)
    ↓
UI (receives updates via WebSocket)
```

### Testing without Hardware

The mock implementations provide realistic test data without any hardware:

```bash
# Run core with mock HALs (default)
./build/core/crankshaft-core --port 8080

# Run UI to visualize mock data
./build/ui/crankshaft-ui

# UI displays simulated vehicle data from mock HAL
```

---

## Development Workflow

### 1. Create a Feature Branch

```bash
git checkout -b feature/my-feature
```

### 2. Make Changes

Edit files in `core/`, `ui/`, or `tests/`


### 3. Build and Test

```bash
./scripts/build.sh Debug
./scripts/run_tests.sh
```

### 4. Format and Lint

```bash
./scripts/format.sh
./scripts/lint.sh
```

### 5. Commit and Push

```bash
git add .
git commit -m "feat: add my feature"
git push origin feature/my-feature
```

### 6. Create Pull Request

Open a PR on GitHub for review

---

## Adding New Features

### Add Core Backend Service

1. Create `core/MyService.h` and `core/MyService.cpp`
2. Add to `core/CMakeLists.txt`:
   ```cmake
   set(SOURCES
     main.cpp
     EventBus.cpp
     WebSocketServer.cpp
     ConfigService.cpp
     Logger.cpp
     MyService.cpp  # Add here
   )
   ```
3. Subscribe to EventBus in your service:
   ```cpp
   connect(EventBus::instance(), &EventBus::messagePublished, this,
       [](const QString& topic, const QVariantMap& payload) {
           // Handle events
       });
   ```
4. Publish events:
   ```cpp
   EventBus::instance()->publish("myservice/event", payload);
   ```

### Add UI Screen

1. Create `ui/qml/MyScreen.qml`
2. Add to `ui/CMakeLists.txt`:
   ```cmake
   qt_add_qml_module(crankshaft-ui
     URI Crankshaft
     VERSION 1.0
     QML_FILES
       qml/Main.qml
       qml/HomeScreen.qml
       qml/SettingsScreen.qml
       qml/MyScreen.qml  # Add here
   )
   ```
3. Navigate to screen:
   ```qml
   Button {
       text: "Open My Screen"
       onClicked: stackView.push("MyScreen.qml")
   }
   ```
4. Subscribe to events:
   ```qml
   Connections {
       target: wsClient
       function onEventReceived(topic, payload) {
           if (topic === "myservice/event") {
               // Handle event
           }
       }
   }
   
   Component.onCompleted: {
       wsClient.subscribe("myservice/*")
   }
   ```

### Add Tests

1. Create `tests/test_myfeature.cpp`
2. Add to `tests/CMakeLists.txt`:
   ```cmake
   add_executable(test_myfeature
     test_myfeature.cpp
     ../core/MyService.cpp
   )
   
   target_link_libraries(test_myfeature PRIVATE
     Catch2::Catch2
     Qt6::Core
   )
   
   add_test(NAME MyFeatureTest COMMAND test_myfeature)
   ```
3. Write tests using Catch2:
   ```cpp
   #define CATCH_CONFIG_MAIN
   #include <catch2/catch.hpp>
   
   TEST_CASE("My feature works", "[myfeature]") {
       REQUIRE(1 + 1 == 2);
   }
   ```

---

## Configuration

### Default Configuration

`config/crankshaft.json`:

```json
{
  "core": {
    "websocket": {
      "port": 8080,
      "host": "0.0.0.0"
    },
    "logging": {
      "level": "info",
      "file": "/var/log/crankshaft/core.log"
    }
  },
  "ui": {
    "theme": {
      "mode": "light"
    },
    "language": {
      "default": "en-GB"
    }
  }
}
```

### Access Configuration

**C++:**
```cpp
ConfigService* config = ConfigService::instance();
config->load("config/crankshaft.json");
int port = config->get("core.websocket.port").toInt();
```

**QML:**
```qml
// Published as event when changed
Connections {
    target: wsClient
    function onEventReceived(topic, payload) {
        if (topic === "config/updated") {
            console.log(payload.key, "=", payload.value)
        }
    }
}
```

---

## Debugging

### Core Backend (CLI)

```bash
./build/core/crankshaft-core --config config/crankshaft.json
```

Watch logs for WebSocket connections and events.

### UI Frontend (Qt Creator)

1. Open `crankshaft-mvp/CMakeLists.txt` in Qt Creator
2. Configure project with Qt6
3. Set breakpoints in C++ code
4. Run with debugger

### QML Debugging

Set environment variables:

```bash
QT_DEBUG_PLUGINS=1 QT_LOGGING_RULES='*=true' ./build/ui/crankshaft-ui
```

Or use Qt Creator's QML debugger.

### Network Debugging

Monitor WebSocket traffic:

```bash
# Install websocat
cargo install websocat

# Connect to server
websocat ws://localhost:8080

# Send subscribe message
{"type":"subscribe","topic":"ui/*"}
```

---

## Packaging

### DEB Package

```bash
cd build
cpack -G DEB
```

Output: `crankshaft-mvp-0.1.0-Linux.deb`

### Install Package

```bash
sudo dpkg -i crankshaft-mvp-0.1.0-Linux.deb
```

### Run Installed Application

```bash
/usr/local/bin/crankshaft-core
/usr/local/bin/crankshaft-ui
```

---

## Troubleshooting

### Build Fails: Qt6 Not Found

Install Qt6 development packages:

```bash
sudo apt install qt6-base-dev qt6-websockets-dev qt6-declarative-dev
```

### Build Fails: AUTOMOC Error

Ensure `AUTOMOC ON` is set in `CMakeLists.txt` for targets with Q_OBJECT.

### UI Fails: Cannot Load QML

Check QML file paths in `qt_add_qml_module`. Paths are relative to `ui/`.

### WebSocket Connection Failed

1. Check Core is running: `ps aux | grep crankshaft-core`
2. Check port: `netstat -an | grep 8080`
3. Check firewall: `sudo ufw status`

### Tests Fail: QEventLoop Timeout

Increase wait time in tests:

```cpp
REQUIRE(spy.wait(5000)); // Wait 5 seconds
```

---

## Contributing

### Code Style

- C++: Google C++ Style Guide, enforced by `.clang-format`
- QML: Qt QML Style Guide
- Commit Messages: Conventional Commits (feat, fix, docs, test, etc.)

### Pull Request Process

1. Fork repository
2. Create feature branch
3. Make changes with tests
4. Format and lint code
5. Ensure all tests pass
6. Push and create PR
7. Address review feedback

### License

All code must include GPL v3 license header (see existing files for template).

---

## Resources

- [Qt6 Documentation](https://doc.qt.io/qt-6/)
- [QML Introduction](https://doc.qt.io/qt-6/qmlfirststeps.html)
- [WebSocket Protocol](https://datatracker.ietf.org/doc/html/rfc6455)
- [Catch2 Documentation](https://github.com/catchorg/Catch2/tree/v2.x)
- [Material Design](https://material.io/design)
- [Google Design for Driving](https://developers.google.com/cars/design)
- [UI Guidelines](./UI_GUIDELINES.md)

---

## UI Design Guidelines

All UI components must follow the **Design for Driving** principles from Google to ensure safety, accessibility, and usability whilst driving. Refer to `docs/UI_GUIDELINES.md` for comprehensive guidelines covering:

- **Interaction Principles:** Keep information glanceable (2-second rule), encourage hands-on driving, prioritise driving tasks, discourage distraction
- **Visual Principles:** Legible text (24dp minimum), large touch targets (76×76dp minimum), consistent UI elements, high contrast (4.5:1 ratio)
- **Typography:** Font scales, sizes, weights, and colour specifications
- **Spacing & Layout:** Grid systems (8dp multiples), padding, margins, responsive layouts
- **Colours:** Light/dark mode colour palettes with high contrast requirements
- **Components:** Button sizing, card layouts, text fields, toggles, and accessibility
- **Accessibility:** Keyboard navigation, screen readers, high contrast modes, voice support

### Key Rules for Automotive UI

1. **Touch Targets:** All interactive elements must be ≥76×76dp
2. **Response Time:** System feedback within 250ms of user input
3. **Text Length:** Limit text to ≤120 characters for glanceability
4. **Contrast:** All text/icons must have ≥4.5:1 contrast ratio
5. **Fonts:** Primary text 24dp+, secondary text 20dp+, avoid thin fonts
6. **Spacing:** Use 8dp grid system for all spacing
7. **No Distraction:** Disable games, auto-scrolling content, and non-essential features during driving
8. **Voice Support:** Implement voice commands for critical functions
9. **Night Mode:** Use negative polarity (light text on dark) for nighttime

### UI Development Workflow

1. Review `docs/UI_GUIDELINES.md` before starting new UI components
2. Ensure all components follow the guidelines checklist
3. Test contrast ratios with WebAIM Contrast Checker
4. Test on automotive displays (day/night, high brightness)
5. Verify response times with QML profiler
6. Validate keyboard and voice interaction support

---

## Support

For issues or questions:

- GitHub Issues: https://github.com/opencardev/crankshaft-mvp/issues
- Documentation: See `docs/` directory
