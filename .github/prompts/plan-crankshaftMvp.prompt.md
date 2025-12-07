# 🚗 Crankshaft - Complete Specification for Copilot

## What to Build

A minimal automotive infotainment system with **separate Core (backend) and UI (frontend)** processes communicating via **WebSockets** and **EventBus**, fully buildable, testable, and lint-compliant.

---

## Architecture

```
┌─────────────────┐         WebSocket          ┌─────────────────┐
│  Core (C++/Qt6) │◄──────────────────────────►│   UI (QML/Qt6)  │
│                 │    JSON Messages            │                 │
│  - EventBus     │                             │  - WS Client    │
│  - WS Server    │                             │  - Theme System │
│  - Services     │                             │  - i18n Support │
└─────────────────┘                             └─────────────────┘
```

### Core Application
- **Tech Stack**: C++17, Qt6 (Core, Network, WebSockets)
- **Port**: 8080 (configurable)
- **Components**:
  - `EventBus`: Topic-based pub/sub for internal messaging
  - `WebSocketServer`: Broadcasts EventBus messages to UI clients
  - `ConfigService`: JSON config management
  - `Logger`: Console + file logging

### UI Application
- **Tech Stack**: QML (Qt6 Quick, Qt6 WebSockets)
- **Components**:
  - `WebSocketClient`: Connects to Core, subscribes to topics
  - `Theme Singleton`: Light/dark mode with Material Design colors
  - `i18n`: en-GB, de-DE translations
  - `HomeScreen`, `SettingsScreen`: Demo UI
  - Material Design Icons integration

---

## WebSocket Protocol (JSON)

```json
{
  "type": "publish|subscribe|event",
  "topic": "ui/theme/changed",
  "payload": { "mode": "dark" },
  "timestamp": 1701475200
}
```

**Message Types**:
- `subscribe`: UI → Core (subscribe to topic)
- `publish`: UI → Core (publish event)
- `event`: Core → UI (broadcast EventBus event)

---

## File Structure

```
crankshaft-mvp/
├── CMakeLists.txt                 # Top-level build
├── README.md                      # Quick start guide
├── .clang-format                  # Google style, 100 char limit
├── .clang-tidy                    # modernize, bugprone, performance
├── .gitignore
│
├── core/
│   ├── CMakeLists.txt
│   ├── main.cpp                   # Core entry point
│   ├── EventBus.h / .cpp          # Pub/sub implementation
│   ├── WebSocketServer.h / .cpp   # WS server + JSON protocol
│   ├── ConfigService.h / .cpp     # Config management
│   └── Logger.h / .cpp            # Logging utility
│
├── ui/
│   ├── CMakeLists.txt
│   ├── main.cpp                   # UI entry point
│   ├── WebSocketClient.h / .cpp   # WS client
│   ├── qml/
│   │   ├── Main.qml               # Application window
│   │   ├── HomeScreen.qml         # Home screen
│   │   ├── SettingsScreen.qml     # Theme + language settings
│   │   ├── Theme.qml              # Singleton for theme
│   │   ├── qmldir
│   │   └── Components/
│   │       ├── AppButton.qml
│   │       ├── Card.qml
│   │       └── Icon.qml           # MDI icon helper
│   ├── i18n/
│   │   ├── ui_en_GB.ts
│   │   └── ui_de_DE.ts
│   ├── assets/
│   │   ├── icons/ (MDI)
│   │   └── ui.qrc                 # Qt resource file
│   └── CMakeLists.txt
│
├── tests/
│   ├── CMakeLists.txt
│   ├── test_eventbus.cpp          # EventBus unit tests
│   ├── test_websocket.cpp         # WebSocket integration tests
│   └── catch2.hpp (FetchContent)
│
├── scripts/
│   ├── build.sh                   # Linux/WSL build script
│   ├── build.ps1                  # PowerShell wrapper
│   ├── lint.sh                    # Run clang-format check
│   ├── format.sh                  # Apply formatting
│   └── run_tests.sh               # Run ctest
│
├── config/
│   └── crankshaft.json            # Default config
│
├── .github/
│   └── workflows/
│       └── ci.yml             # CI: build, test, lint, package
│
└── docs/
    ├── ARCHITECTURE.md
    ├── API.md (WebSocket protocol)
    └── DEVELOPMENT.md
```

---

## MVP Features Checklist

### Core Features
- ✅ EventBus with thread-safe pub/sub
- ✅ WebSocket server (Qt WebSockets)
- ✅ JSON message protocol (subscribe, publish, event)
- ✅ Broadcast EventBus → WebSocket clients
- ✅ ConfigService (read/write JSON)
- ✅ CLI args: `--port`, `--config`

### UI Features
- ✅ WebSocket client with auto-reconnect
- ✅ Subscribe to topics from Core
- ✅ Theme singleton (light/dark mode)
- ✅ Material Design color palette
- ✅ i18n with qsTr() (en-GB, de-DE)
- ✅ HomeScreen with demo widgets
- ✅ SettingsScreen (theme toggle, language selector)
- ✅ Material Design Icons (MDI)

### Build & Quality
- ✅ CMake build with separate core/ui/tests
- ✅ Catch2 tests via FetchContent
- ✅ `.clang-format` + `.clang-tidy`
- ✅ `qmllint` for QML files
- ✅ GitHub Actions CI (build, test, lint)
- ✅ CPack .deb package generation
- ✅ Works on Linux, WSL, Raspberry Pi OS

---

## Implementation Phases

### Phase 1: Core Backend (2 days)
1. CMake setup for `core/`
2. Implement `EventBus` (pub/sub with QObject signals)
3. Write `test_eventbus.cpp` (Catch2)
4. Implement `WebSocketServer` (listen, accept, broadcast)
5. JSON message parsing (QJsonDocument)
6. `ConfigService` + `Logger` utilities
7. `main.cpp` with CLI args

### Phase 2: UI Frontend (2 days)
1. CMake setup for `ui/` with QML resources
2. Implement `WebSocketClient` (connect, send, receive)
3. Create `Main.qml` (ApplicationWindow + StackView)
4. Create `Theme.qml` singleton (Material colors)
5. Add i18n (.ts files, QTranslator)
6. `HomeScreen.qml` + `SettingsScreen.qml`
7. MDI icon integration

### Phase 3: Integration (1 day)
1. Connect UI to Core via WebSocket
2. Test theme change: UI → Core → EventBus → UI
3. Test language change
4. Write `test_websocket.cpp`
5. Multi-client WebSocket test

### Phase 4: CI & Packaging (1 day)
1. GitHub Actions workflow (`.github/workflows/mvp-ci.yml`)
2. Install Qt6, build, run tests
3. clang-format check, clang-tidy, qmllint
4. CPack DEB generation
5. Upload artifacts

---

## Code Examples

### EventBus.h (Core)
```cpp
#pragma once
#include <QObject>
#include <QVariantMap>
#include <QMutex>

class EventBus : public QObject {
  Q_OBJECT
public:
  static EventBus& instance();
  void publish(const QString &topic, const QVariantMap &payload);
  
signals:
  void messagePublished(const QString &topic, const QVariantMap &payload);

private:
  EventBus() = default;
  QMutex m_mutex;
};
```

### WebSocketServer.h (Core)
```cpp
#pragma once
#include <QObject>
#include <QWebSocketServer>
#include <QWebSocket>

class WebSocketServer : public QObject {
  Q_OBJECT
public:
  explicit WebSocketServer(quint16 port, QObject *parent = nullptr);
  void broadcastEvent(const QString &topic, const QVariantMap &payload);

private slots:
  void onNewConnection();
  void onTextMessageReceived(const QString &message);
  void onClientDisconnected();

private:
  QWebSocketServer *m_server;
  QList<QWebSocket*> m_clients;
};
```

### WebSocketClient.h (UI)
```cpp
#pragma once
#include <QObject>
#include <QWebSocket>

class WebSocketClient : public QObject {
  Q_OBJECT
public:
  explicit WebSocketClient(const QUrl &url, QObject *parent = nullptr);
  Q_INVOKABLE void subscribe(const QString &topic);
  Q_INVOKABLE void publish(const QString &topic, const QVariantMap &payload);

signals:
  void eventReceived(const QString &topic, const QVariantMap &payload);
  void connected();
  void disconnected();

private slots:
  void onTextMessageReceived(const QString &message);
};
```

### Theme.qml (UI Singleton)
```qml
pragma Singleton
import QtQuick 2.15

QtObject {
  property bool isDark: false
  
  property color background: isDark ? "#1e1e1e" : "#ffffff"
  property color surface: isDark ? "#2d2d2d" : "#f5f5f5"
  property color primary: "#1976d2"
  property color text: isDark ? "#ffffff" : "#000000"
  
  property int spacing: 16
  property int radius: 8
}
```

---

## Build Commands

### Linux / WSL
```bash
# Configure
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Build
cmake --build build -j

# Run Core (terminal 1)
./build/core/crankshaft-core --port 8080

# Run UI (terminal 2)
./build/ui/crankshaft-ui --server ws://localhost:8080

# Run Tests
ctest --test-dir build --output-on-failure

# Format Check
find core ui tests -name '*.cpp' -o -name '*.h' | xargs clang-format --dry-run --Werror

# Lint
clang-tidy -p build core/*.cpp ui/*.cpp

# Package
cd build && cpack -G DEB
```

### Windows PowerShell (using WSL)
```powershell
# Build
wsl bash -lc "cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug && cmake --build build -j"

# Run Core
wsl bash -lc "./build/core/crankshaft-core"

# Run UI
wsl bash -lc "export DISPLAY=:0; ./build/ui/crankshaft-ui"
```

---

## GitHub Actions CI Workflow

```yaml
name: MVP CI

on: [push, pull_request]

jobs:
  build-and-test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      
      - name: Install Qt6
        uses: jurplel/install-qt-action@v3
        with:
          version: '6.5.3'
          modules: 'qtwebsockets'
      
      - name: Install Dependencies
        run: sudo apt-get install -y cmake ninja-build clang-format clang-tidy
      
      - name: Configure
        run: cmake -S . -B build -GNinja -DCMAKE_BUILD_TYPE=Debug
      
      - name: Build
        run: cmake --build build
      
      - name: Test
        run: ctest --test-dir build --output-on-failure
      
      - name: Lint (clang-format)
        run: bash scripts/lint.sh
      
      - name: Package
        run: cd build && cpack -G DEB
      
      - name: Upload Artifact
        uses: actions/upload-artifact@v4
        with:
          name: crankshaft-mvp-deb
          path: build/*.deb
```

---

## Success Criteria

✅ **Core runs standalone** (./crankshaft-core --port 8080)  
✅ **UI connects to Core** via WebSocket  
✅ **EventBus works**: Core publishes → UI receives  
✅ **Theme switching**: UI sends command → Core broadcasts → UI updates  
✅ **Language switching**: Works with qsTr() + QTranslator  
✅ **All tests pass**: EventBus, WebSocket, integration  
✅ **Lint passes**: clang-format, clang-tidy, qmllint  
✅ **CI builds and tests** on GitHub Actions  
✅ **.deb package** installs on Raspberry Pi 4  

---

## What's NOT in MVP

❌ Extension framework  
❌ Extension store/marketplace  
❌ AndroidAuto integration  
❌ Media player, radio, bluetooth  
❌ Navigation  
❌ Security sandboxing  
❌ OTA updates  

**MVP = Architecture proof-of-concept with working Core ↔ UI communication**

---

## Copilot Prompt to Build This

> "Create a CMake-based Qt6 C++/QML project with:
> 1. `core/` folder: C++ backend with EventBus (pub/sub), WebSocketServer (Qt WebSockets), ConfigService, Logger. Main accepts --port CLI arg.
> 2. `ui/` folder: QML frontend with WebSocketClient, Theme singleton (light/dark Material Design), i18n (en-GB, de-DE), HomeScreen, SettingsScreen, MDI icons.
> 3. `tests/` folder: Catch2 tests for EventBus and WebSocket using FetchContent.
> 4. WebSocket JSON protocol: {type, topic, payload} for subscribe/publish/event messages.
> 5. Scripts: build.sh, lint.sh (clang-format check), format.sh.
> 6. GitHub Actions CI: build on Ubuntu, run tests, check formatting, generate .deb.
> 7. .clang-format (Google style), .clang-tidy (modernize,bugprone,performance).
> 8. All files have GPL v3 license headers.
> 9. README with quick start commands.
> Follow the file structure and architecture in MVP_SPECIFICATION.md."

---

This specification gives you everything needed to ask Copilot to build the complete MVP from scratch. The system will have proper separation of concerns, real-time communication, theme/i18n support, and full CI/CD pipeline.
