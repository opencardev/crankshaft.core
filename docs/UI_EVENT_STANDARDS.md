# UI Event Handling Standards

## Event Publishing Guidelines

### When to Publish Events

**DO Publish Events For:**
1. **User Actions** - Button clicks, toggle switches, slider changes
   - Example: `wsClient.publish("audio/volume", { "value": 75 })`
   - Example: `wsClient.publish("wifi/enabled", { "enabled": true })`

2. **User-Initiated State Changes** - User explicitly changes a setting
   - Example: `wsClient.publish("ui/theme/changed", { "mode": "dark" })`
   - Example: `wsClient.publish("bluetooth/connect", { "device": "..." })`

3. **User Commands** - Explicit user requests to the backend
   - Example: `wsClient.publish("androidauto/disconnect", {})`
   - Example: `wsClient.publish("bluetooth/discover", {})`

**DO NOT Publish Events For:**
1. **Component Lifecycle** - Component loading, unloading, or initialization
   - ❌ DON'T: `Component.onCompleted: { wsClient.publish("screen/loaded", {}) }`
   - ✅ DO: Backend should track screen state via navigation events if needed

2. **Navigation Intent** - When card/button is just for navigation
   - ❌ DON'T: `onClicked: { wsClient.publish("ui/navigation/opened", {}) }`
   - ✅ DO: `onClicked: { stack.push(navigationScreen) }`

3. **Redundant Status Queries** - Automatic polling/status checks
   - ❌ DON'T: Auto-query status on every screen load
   - ✅ DO: Backend pushes status changes via WebSocket events

### Event Naming Convention

Format: `<domain>/<action>`

**Domains:**
- `ui/*` - UI state changes (theme, language, etc.)
- `audio/*` - Audio system controls
- `video/*` - Video/display settings
- `wifi/*` - WiFi connectivity
- `bluetooth/*` - Bluetooth connectivity
- `androidauto/*` - Android Auto interactions

**Actions:**
- Use past tense for state changes: `changed`, `enabled`, `disabled`
- Use present tense for commands: `connect`, `disconnect`, `discover`
- Use nouns for data: `volume`, `brightness`, `resolution`

### Payload Structure

Always use JSON objects with descriptive keys:

```qml
// Good
wsClient.publish("audio/volume", {
    "value": volumeSlider.value
})

// Good
wsClient.publish("wifi/connect", {
    "ssid": "MyNetwork",
    "password": "secret"
})

// Acceptable for simple toggles
wsClient.publish("audio/mute", {
    "muted": muteSwitch.checked
})
```

### Screen-Specific Patterns

#### HomeScreen
- Cards for unimplemented features: Just log to console with TODO
- Cards for implemented features: Navigate using `stack.push()`
- No automatic event publishing on load

#### AndroidAutoScreen
- Touch events: Publish normalized coordinates
- Control buttons: HOME button publishes, back button navigates
- No status query on Component.onCompleted

#### Settings Screens
- Changes immediately publish events (no "Save" button pattern)
- Toggle switches: Publish on `onCheckedChanged`
- Sliders: Publish on `onValueChanged` (may want debouncing for performance)
- ComboBox: Publish on `onActivated`
- Back button: Navigate via `stack.pop()`, don't publish events

### Backend Event Handling

The backend should:
1. Listen for UI events on subscribed topics
2. Process the events and update internal state
3. Broadcast state changes back to UI via WebSocket
4. UI reacts to incoming events via `Connections { target: wsClient }`

### Example Patterns

**User Action → Backend → State Update:**
```qml
// User changes theme
Switch {
    onToggled: {
        Theme.toggleTheme()
        wsClient.publish("ui/theme/changed", {
            "mode": Theme.isDark ? "dark" : "light"
        })
    }
}

// Backend broadcasts confirmation
Connections {
    target: wsClient
    function onEventReceived(topic, payload) {
        if (topic === "ui/theme/changed") {
            Theme.isDark = payload.mode === "dark"
        }
    }
}
```

**Navigation (No Events):**
```qml
// Just navigate - backend doesn't need to know
AppButton {
    onClicked: {
        if (stack) {
            stack.push(settingsScreen, { stack: stack })
        }
    }
}
```

**User Command:**
```qml
// User explicitly requests action
Button {
    text: "Disconnect"
    onClicked: {
        wsClient.publish("androidauto/disconnect", {})
    }
}
```

## Migration Checklist

When adding new screens or features:
- [ ] Remove `Component.onCompleted` event publishing unless absolutely necessary
- [ ] Navigation cards/buttons should use `stack.push()` not `wsClient.publish()`
- [ ] Only publish events for actual user actions, not lifecycle events
- [ ] Use consistent domain/action naming
- [ ] Include meaningful payload data with descriptive keys
- [ ] Document expected backend response events
