# Extensible Settings System (VSCode-Style)

## Overview

The Crankshaft settings system uses a simple, VSCode-style split view that's easy to extend. Settings are defined in a data model that can be modified at runtime by extensions.

## Architecture

### Components

1. **SettingsModel** (QML)
   - Data model containing categories and settings
   - Provides functions to add categories and settings dynamically
   - Simple JavaScript object structure

2. **SettingsScreen** (QML)
   - VSCode-style split view (category list | settings detail)
   - Automatic rendering based on setting types
   - Deep linking support
   - No C++ required for basic usage

## Adding Settings

### Method 1: Modify SettingsModel.qml Directly

Edit `ui/qml/SettingsModel.qml` and add to the `categories` array:

```qml
{
    id: "myservice",
    name: "My Service",
    icon: "🔧",
    settings: [
        {
            key: "myservice.enabled",
            label: "Enable My Service",
            description: "Turn the service on or off",
            type: "toggle",
            value: true,
            onChange: function(val) {
                wsClient.publish("myservice/enabled", { "value": val })
            }
        }
    ]
}
```

### Method 2: Add Category Dynamically (Extension)

From any QML component with access to settingsData:

```qml
Component.onCompleted: {
    settingsData.addCategory({
        id: "myextension",
        name: "My Extension",
        icon: "🎨",
        settings: [
            {
                key: "myext.setting1",
                label: "Setting One",
                description: "First setting",
                type: "toggle",
                value: false,
                onChange: function(val) {
                    console.log("Setting changed:", val)
                }
            }
        ]
    })
}
```

### Method 3: Add Settings to Existing Category

```qml
settingsData.addSettingsToCategory("system", [
    {
        key: "mysetting",
        label: "My New Setting",
        type: "text",
        value: "default",
        onChange: function(val) {
            wsClient.publish("settings/changed", { "key": "mysetting", "value": val })
        }
    }
])

### Setting Types

#### Toggle (Boolean)
```qml
{
    type: "toggle",
    defaultValue: false,
    currentValue: false
}
```

#### Select (Dropdown)
```qml
{
    type: "select",
    defaultValue: "option1",
    currentValue: "option1",
    options: {
        values: ["option1", "option2", "option3"]
    }
}
```

#### Text (Text Input)
```qml
{
    type: "text",
    defaultValue: "",
    currentValue: "",
    options: {
        placeholder: "Enter text..."
    }
}
```

#### Number (SpinBox)
```qml
{
    type: "number",
    defaultValue: 50,
    currentValue: 50,
    options: {
        min: 0,
        max: 100
    }
}
```

#### Slider
```qml
{
    type: "slider",
    defaultValue: 50,
    currentValue: 50,
    options: {
        min: 0,
        max: 100,
        step: 1
    }
}
```

## Custom Settings Pages

For complex settings that need custom UI, register a page with a QML component:

```qml
SettingsRegistry.registerPage(
    "androidauto",
    "androidauto.settings",
    "Android Auto",
    "🚗",
    "qrc:/qt/qml/Crankshaft/qml/AndroidAutoSettingsScreen.qml",  // Custom component
    SettingsRegistry.Basic,
    10
)
```

The custom QML component will receive:
- `stack` property for navigation
- Access to `wsClient` for publishing changes
- Access to `Theme` and `Strings`

## Deep Linking

Navigate directly to a specific settings page:

```qml
// From anywhere in the app
stack.push(settingsScreen, {
    stack: stack,
    deepLinkPageId: "androidauto.settings"
})
```

The settings screen will automatically:
1. Select the appropriate category
2. Navigate to the specified page

## Event Handling

When settings are changed, they publish WebSocket events:

```javascript
{
    topic: "settings/changed",
    payload: {
        key: "ui.theme.mode",
        value: true
    }
}
```

Services should subscribe to `settings/*` to receive updates.

## Default Categories

Pre-registered categories:
- **general** (order 10): General system settings
- **appearance** (order 20): Theme, language, display
- **connectivity** (order 30): Network, Bluetooth, Wi-Fi
- **media** (order 40): Audio, video, media player
- **androidauto** (order 50): Android Auto settings
- **system** (order 90): System information, logs
- **about** (order 100): About, version, credits

## Extension Example

Extensions can register settings in their initialization:

```qml
// In extension's settings initializer
QtObject {
    Component.onCompleted: {
        // Register page
        SettingsRegistry.registerPage(
            "media",
            "media.myextension",
            "My Media Extension",
            "🎵",
            "",
            SettingsRegistry.Advanced,
            20
        )
        
        // Register settings
        SettingsRegistry.registerSetting("media.myextension", {
            key: "myextension.enabled",
            label: "Enable Extension",
            description: "Turn the extension on or off",
            type: "toggle",
            defaultValue: true,
            currentValue: true,
            complexityLevel: SettingsRegistry.Advanced,
            order: 10
        })
    }
}
```

## Best Practices

1. **Use namespaced keys**: `service.category.setting`
2. **Set appropriate complexity levels**: Don't overwhelm basic users
3. **Provide descriptions**: Help users understand settings
4. **Use meaningful order values**: Group related settings
5. **Subscribe to setting changes**: Listen for `settings/*` events
6. **Validate input**: Check values in your service
7. **Provide defaults**: Always set sensible default values
8. **Use deep linking**: Let users jump directly to relevant settings

## Migration from Old Settings

The old SettingsScreen has been replaced. To migrate:

1. Remove hard-coded settings UI
2. Register pages and settings via SettingsRegistry
3. Subscribe to `settings/changed` events
4. Update navigation to use deep linking if needed

## Future Enhancements

- Settings import/export
- Settings search
- Settings profiles (quick switch between configurations)
- Settings validation and constraints
- Settings groups within pages
- Settings reset to defaults
- Settings change history/undo
