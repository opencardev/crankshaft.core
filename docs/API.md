# Crankshaft MVP - API Reference

## WebSocket API

### Connection

Connect to the Core backend via WebSocket:

```
ws://localhost:8080
```

The port can be configured in `config/crankshaft.json` or via CLI argument `--port`.

### Message Format

All messages are JSON objects. The `type` field determines the message purpose.

---

## Client → Server Messages

### Subscribe to Topic

Subscribe to receive events for a specific topic or pattern.

**Request:**
```json
{
  "type": "subscribe",
  "topic": "ui/theme/*"
}
```

**Fields:**
- `type` (string): Must be `"subscribe"`
- `topic` (string): Topic or pattern to subscribe to

**Wildcards:**
- `*` at end of topic matches all sub-topics
- Example: `"ui/*"` matches `"ui/theme"`, `"ui/navigation"`, etc.

**Response:** None (subscription is confirmed by receiving events)

---

### Unsubscribe from Topic

Stop receiving events for a topic.

**Request:**
```json
{
  "type": "unsubscribe",
  "topic": "ui/theme/*"
}
```

**Fields:**
- `type` (string): Must be `"unsubscribe"`
- `topic` (string): Topic to unsubscribe from

**Response:** None

---

### Publish Event

Publish an event to the Core backend.

**Request:**
```json
{
  "type": "publish",
  "topic": "ui/button/clicked",
  "payload": {
    "button": "settings",
    "timestamp": 1234567890
  }
}
```

**Fields:**
- `type` (string): Must be `"publish"`
- `topic` (string): Event topic
- `payload` (object): Event data (any valid JSON object)

**Response:** None (event is broadcasted to subscribers)

---

## Server → Client Messages

### Event Broadcast

Receive an event from the Core backend.

**Message:**
```json
{
  "type": "event",
  "topic": "ui/theme/changed",
  "payload": {
    "mode": "dark"
  }
}
```

**Fields:**
- `type` (string): Always `"event"`
- `topic` (string): Event topic
- `payload` (object): Event data

**Trigger:** Sent when a subscribed topic receives an event

---

## EventBus API (C++)

Internal API for Core backend services.

### Subscribe to Events

Use Qt signals/slots to receive events:

```cpp
#include "EventBus.h"

EventBus* bus = EventBus::instance();
connect(bus, &EventBus::messagePublished, this, 
    [](const QString& topic, const QVariantMap& payload) {
        qDebug() << "Event:" << topic << payload;
    });
```

### Publish Events

```cpp
EventBus* bus = EventBus::instance();
QVariantMap payload;
payload["key"] = "value";
bus->publish("my/topic", payload);
```

**Thread Safety:** `publish()` is thread-safe and can be called from any thread.

---

## ConfigService API (C++)

Manage application configuration.

### Get Configuration Value

```cpp
#include "ConfigService.h"

ConfigService* config = ConfigService::instance();
config->load("config/crankshaft.json");

int port = config->get("core.websocket.port").toInt();
QString theme = config->get("ui.theme.mode").toString();
```

### Set Configuration Value

```cpp
config->set("ui.theme.mode", "dark");
config->save("config/crankshaft.json");
```

**Key Format:** Dot-notation for nested keys (e.g., `"parent.child.grandchild"`)

### Listen for Changes

```cpp
connect(config, &ConfigService::configChanged, this,
    [](const QString& key, const QVariant& value) {
        qDebug() << "Config changed:" << key << "=" << value;
    });
```

---

## Logger API (C++)

Centralized logging service.

### Log Messages

```cpp
#include "Logger.h"

Logger* logger = Logger::instance();
logger->log(Logger::Info, "Application started");
logger->log(Logger::Warning, "Low memory detected");
logger->log(Logger::Error, "Failed to connect to server");
```

**Levels:**
- `Logger::Debug` - Detailed debug information
- `Logger::Info` - General informational messages
- `Logger::Warning` - Warning messages (non-critical)
- `Logger::Error` - Error messages (critical)

### Configure Logger

```cpp
logger->setLogFile("/var/log/crankshaft.log");
logger->setMinLevel(Logger::Info); // Only log Info and above
```

---

## WebSocketClient API (QML)

QML-accessible WebSocket client.

### Subscribe to Topic

```qml
wsClient.subscribe("ui/theme/*")
```

### Unsubscribe from Topic

```qml
wsClient.unsubscribe("ui/theme/*")
```

### Publish Event

```qml
wsClient.publish("ui/button/clicked", {
    "button": "settings",
    "timestamp": Date.now()
})
```

### Handle Events

```qml
Connections {
    target: wsClient
    
    function onEventReceived(topic, payload) {
        console.log("Event:", topic, JSON.stringify(payload))
        
        if (topic === "ui/theme/changed") {
            Theme.isDark = payload.mode === "dark"
        }
    }
    
    function onErrorOccurred(error) {
        console.error("WebSocket error:", error)
    }
}
```

### Connection Status

```qml
Text {
    text: wsClient.connected ? "Connected" : "Disconnected"
    color: wsClient.connected ? "green" : "red"
}
```

---

## Theme API (QML)

QML Singleton for consistent styling.

### Access Theme Properties

```qml
import QtQuick

Rectangle {
    color: Theme.surface
    radius: Theme.radiusMd
    
    Text {
        text: "Hello World"
        font.pixelSize: Theme.fontSizeH2
        color: Theme.textPrimary
    }
}
```

### Toggle Theme

```qml
Button {
    text: "Toggle Theme"
    onClicked: Theme.toggleTheme()
}
```

### Available Properties

**Colors:**
- `background`, `surface`, `surfaceVariant`
- `primary`, `primaryVariant`, `secondary`, `secondaryVariant`
- `error`, `success`, `warning`
- `textPrimary`, `textSecondary`, `textDisabled`
- `divider`

**Typography:**
- `fontSizeH1` (32px), `fontSizeH2` (24px), `fontSizeH3` (20px)
- `fontSizeBody` (16px), `fontSizeCaption` (14px), `fontSizeSmall` (12px)

**Spacing:**
- `spacingXs` (4px), `spacingSm` (8px), `spacingMd` (16px)
- `spacingLg` (24px), `spacingXl` (32px)

**Shape:**
- `radiusSm` (4px), `radiusMd` (8px), `radiusLg` (16px), `radiusXl` (24px)

**Animation:**
- `animationDuration` (200ms), `animationDurationSlow` (300ms)

**State:**
- `isDark` (bool) - Current theme mode

---

## Common Topics

### UI Topics

- `ui/theme/changed` - Theme mode changed
  - Payload: `{ "mode": "dark" | "light" }`

- `ui/language/changed` - UI language changed
  - Payload: `{ "language": "en-GB" | "de-DE" }`

- `ui/navigation/opened` - Navigation feature opened
  - Payload: `{}`

- `ui/media/opened` - Media player opened
  - Payload: `{}`

- `ui/phone/opened` - Phone feature opened
  - Payload: `{}`

### System Topics

- `system/startup` - Application started
  - Payload: `{ "version": "0.1.0" }`

- `system/shutdown` - Application shutting down
  - Payload: `{}`

- `system/error` - System error occurred
  - Payload: `{ "message": "Error description" }`

### Config Topics

- `config/updated` - Configuration changed
  - Payload: `{ "key": "path.to.key", "value": "new value" }`

---

## Error Handling

### WebSocket Errors

The `errorOccurred` signal provides error information:

```qml
Connections {
    target: wsClient
    
    function onErrorOccurred(error) {
        // Handle error (e.g., show notification)
        console.error("Connection error:", error)
    }
}
```

### JSON Parsing Errors

Invalid JSON messages are logged and ignored. Check Core logs:

```
[Warning] Invalid JSON message received
```

### Topic Mismatch

Subscribing to non-existent topics has no effect. Events are only received if published.

---

## Rate Limiting

No rate limiting in MVP. For production, consider:

- Max subscriptions per client
- Max message rate per client
- Max payload size

---

## Example Integration

### Publishing from C++ Extension

```cpp
#include "EventBus.h"

class MyExtension : public QObject {
public:
    void doSomething() {
        EventBus* bus = EventBus::instance();
        
        QVariantMap payload;
        payload["status"] = "complete";
        payload["result"] = 42;
        
        bus->publish("extension/myextension/complete", payload);
    }
};
```

### Receiving in QML UI

```qml
Connections {
    target: wsClient
    
    function onEventReceived(topic, payload) {
        if (topic === "extension/myextension/complete") {
            console.log("Extension result:", payload.result)
            resultText.text = "Result: " + payload.result
        }
    }
}

Component.onCompleted: {
    wsClient.subscribe("extension/myextension/*")
}
```
