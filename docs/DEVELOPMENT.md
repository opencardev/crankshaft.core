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
./scripts/build.sh Debug
```

Or for release build:

```bash
./scripts/build.sh Release
```

### Windows (PowerShell)

```powershell
.\scripts\build.ps1 Debug
```

### Manual Build

```bash
# Configure
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug

# Build
cmake --build build --config Debug -j$(nproc)
```

**Outputs:**
- Core: `build/core/crankshaft-core`
- UI: `build/ui/crankshaft-ui`
- Tests: `build/tests/test_*`

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
│   └── Logger.h/.cpp           # Logging service
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

---

## Support

For issues or questions:

- GitHub Issues: https://github.com/opencardev/crankshaft-mvp/issues
- Documentation: See `docs/` directory
