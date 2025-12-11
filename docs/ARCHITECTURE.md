# Crankshaft MVP - Architecture

## Overview

Crankshaft MVP is an automotive infotainment system designed around a modular, event-driven architecture. The system consists of two main processes that communicate via WebSockets:

1. **Core Backend** - Manages business logic, event routing, and configuration
2. **UI Frontend** - Provides the graphical user interface

## System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                     UI Frontend (Qt/QML)                     │
│  ┌────────────┐  ┌──────────────┐  ┌──────────────────┐    │
│  │   Main.qml │  │  HomeScreen  │  │ SettingsScreen   │    │
│  │            │  │              │  │                  │    │
│  └─────┬──────┘  └──────┬───────┘  └────────┬─────────┘    │
│        │                │                    │              │
│        └────────────────┴────────────────────┘              │
│                         │                                   │
│                  ┌──────▼────────┐                          │
│                  │ WebSocketClient│                          │
│                  └───────┬────────┘                          │
└──────────────────────────┼───────────────────────────────────┘
                           │ WebSocket (JSON)
                           │ ws://localhost:8080
┌──────────────────────────┼───────────────────────────────────┐
│                  ┌───────▼────────┐                          │
│                  │WebSocketServer │                          │
│                  └───────┬────────┘                          │
│                          │                                   │
│              ┌───────────┴────────────┐                      │
│              │      EventBus          │                      │
│              │   (Pub/Sub Pattern)    │                      │
│              └───────────┬────────────┘                      │
│                          │                                   │
│         ┌────────────────┼────────────────┐                 │
│         │                │                │                 │
│  ┌──────▼──────┐  ┌──────▼──────┐  ┌──────▼──────┐        │
│  │ConfigService│  │   Logger    │  │   Future    │        │
│  │             │  │             │  │  Extensions │        │
│  └─────────────┘  └─────────────┘  └─────────────┘        │
│                                                             │
│                  Core Backend (C++)                        │
└─────────────────────────────────────────────────────────────┘
```

## Components

### Core Backend

#### EventBus
- **Purpose**: Central message broker for internal communication
- **Pattern**: Singleton, Pub/Sub
- **Thread Safety**: QMutex-protected for multi-threaded access
- **API**:
  - `publish(topic, payload)` - Emit an event
  - Signal: `messagePublished(topic, payload)` - Receive events

#### WebSocketServer
- **Purpose**: WebSocket server for UI communication
- **Port**: Configurable (default: 8080)
- **Protocol**: JSON messages
- **Features**:
  - Client connection management
  - Per-client topic subscriptions (with wildcard support)
  - Broadcast events to subscribed clients
  - Auto-reconnect on disconnect

#### ConfigService
- **Purpose**: Configuration management with JSON persistence
- **Pattern**: Singleton
- **Features**:
  - Load/save JSON configuration files
  - Dot-notation key access (e.g., "core.websocket.port")
  - Signal: `configChanged` for reactive updates

#### Logger
- **Purpose**: Centralized logging
- **Levels**: Debug, Info, Warning, Error
- **Outputs**: Console (qDebug) and optional file
- **Format**: ISO 8601 timestamp + level + message

### UI Frontend

#### WebSocketClient
- **Purpose**: Connect UI to Core backend
- **Features**:
  - Auto-reconnect with 2-second delay
  - Subscribe/unsubscribe to topics
  - Publish messages to backend
  - Q_INVOKABLE methods for QML access
- **Signals**:
  - `eventReceived(topic, payload)` - New event from backend
  - `connectedChanged()` - Connection status changed
  - `errorOccurred(error)` - Connection error

#### Theme System
- **Purpose**: Centralized styling for QML UI
- **Pattern**: QML Singleton
- **Features**:
  - Light/Dark mode toggle
  - Material Design color palette
  - Typography scale (H1, H2, H3, Body, Caption, Small)
  - Spacing and border radius constants
  - Animation duration settings

#### QML Components
- **Main.qml**: Application window, StackView navigation, connection status indicator
- **HomeScreen.qml**: Main dashboard with navigation cards
- **SettingsScreen.qml**: Theme toggle, language selector, connection info
- **AppButton.qml**: Themed button component
- **Card.qml**: Interactive card with ripple effect
- **Icon.qml**: Icon display (placeholder for MDI)

#### Internationalization (i18n)
- **Languages**: English (GB), German (DE)
- **Format**: Qt Linguist .ts/.qm files
- **Runtime**: QTranslator loads language at startup

## WebSocket Protocol

All messages are JSON objects with a `type` field:

### Subscribe to Topic
```json
{
  "type": "subscribe",
  "topic": "ui/*"
}
```

### Unsubscribe from Topic
```json
{
  "type": "unsubscribe",
  "topic": "ui/theme"
}
```

### Publish Event
```json
{
  "type": "publish",
  "topic": "ui/button/clicked",
  "payload": {
    "button": "settings"
  }
}
```

### Event Broadcast (Server → Client)
```json
{
  "type": "event",
  "topic": "ui/theme/changed",
  "payload": {
    "mode": "dark"
  }
}
```

## Topic Conventions

Topics use hierarchical naming with `/` separator:

- `ui/*` - UI-related events
- `ui/theme/*` - Theme changes
- `ui/navigation/*` - Navigation events
- `system/*` - System-level events
- `config/*` - Configuration changes

Wildcards:
- `ui/*` - All topics starting with `ui/`
- `*` - All topics (use sparingly)

## Data Flow Examples

### Theme Toggle
1. User clicks dark mode switch in SettingsScreen.qml
2. QML calls `Theme.toggleTheme()`
3. QML publishes via `wsClient.publish("ui/theme/changed", {"mode": "dark"})`
4. WebSocketServer receives message, publishes to EventBus
5. EventBus broadcasts to all subscribers
6. WebSocketServer broadcasts to all subscribed clients
7. Other UI instances receive event via `wsClient.eventReceived`

### Configuration Update
1. Extension modifies configuration via ConfigService
2. ConfigService emits `configChanged` signal
3. Core publishes to EventBus: `config/updated`
4. WebSocketServer broadcasts to subscribed UI clients
5. UI updates display

## Threading Model

- **Core Backend**: QCoreApplication event loop, single-threaded with Qt signals/slots
- **EventBus**: Thread-safe with QMutex, callable from any thread
- **UI Frontend**: QGuiApplication event loop, QML runs on main thread
- **WebSocketServer**: Qt network thread handles socket I/O

## Security Considerations

- WebSocket server binds to localhost by default (no external access)
- No authentication in MVP (add for production)
- Message validation: JSON parsing with error handling
- Topic filtering: Clients can only subscribe, not broadcast arbitrarily

## Extension Points

The MVP architecture supports future extensions:

1. **New Backend Services**: Subscribe to EventBus, publish events
2. **New UI Screens**: Create QML files, stack navigation
3. **External Integrations**: Connect to EventBus, communicate via WebSocket
4. **Plugin System**: Load dynamic libraries, register with EventBus

## Build System

- **CMake 3.16+**: Multi-target build
- **Subdirectories**: `core/`, `ui/`, `tests/`
- **Qt6 Modules**: Core, Network, WebSockets, Gui, Qml, Quick
- **Testing**: Catch2 v2.13.10 via FetchContent
- **Packaging**: CPack generates .deb packages

## Configuration Files

- `config/crankshaft.json` - Main configuration
  - WebSocket port
  - Logging level and file
  - UI theme and language defaults
