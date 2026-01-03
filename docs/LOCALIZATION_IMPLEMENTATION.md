# Localization (i18n) Implementation Guide

**Status**: ✅ COMPLETE  
**Supported Locales**: 5 languages (en_GB, en_US, de_DE, fr_FR, es_ES)  
**Framework**: Qt6 Linguist Tools (LinguistTools)  
**Date**: January 3, 2026

---

## Overview

Crankshaft now supports localisation across five major locales, enabling drivers in different regions to use the infotainment system in their native language. The implementation uses Qt's i18n framework with standard translation files (`.ts` XML format).

---

## Supported Locales

| Code | Name | Language | Region |
|------|------|----------|--------|
| `en_GB` | English (GB) | English | United Kingdom |
| `en_US` | English (US) | English | United States |
| `de_DE` | Deutsch (DE) | German | Germany |
| `fr_FR` | Français (FR) | French | France |
| `es_ES` | Español (ES) | Spanish | Spain |

### Why These Locales?

- **en_GB**: Project default, uses British English spelling/conventions
- **en_US**: North American market (largest automotive market)
- **de_DE**: German-speaking automotive region (quality engineering tradition)
- **fr_FR**: French automotive market, Alpine region
- **es_ES**: Spanish automotive market, Mediterranean region

---

## File Structure

```
ui/
├── i18n/                          # Translation files
│   ├── ui_en_GB.ts               # British English (base)
│   ├── ui_en_US.ts               # American English
│   ├── ui_de_DE.ts               # German
│   ├── ui_fr_FR.ts               # French
│   └── ui_es_ES.ts               # Spanish
├── qml/
│   ├── models/
│   │   └── Strings.qml           # String definitions (qsTr() calls)
│   ├── screens/
│   │   ├── SettingsScreen.qml    # Includes LocaleSelector
│   │   └── ...
│   └── components/
│       └── LocaleSelector.qml    # New: Locale switching UI
└── CMakeLists.txt                # Updated with all .ts files
```

---

## Implementation: Using Translations in QML

### 1. Define Strings with `qsTr()`

In `ui/qml/models/Strings.qml` (or any QML file):

```qml
// ui/qml/models/Strings.qml
import QtQuick

QtObject {
    // App title (translatable)
    readonly property string appTitle: qsTr('Crankshaft')
    
    // Settings labels
    readonly property string settingsTitle: qsTr('Settings')
    readonly property string appearance: qsTr('Appearance')
    readonly property string darkMode: qsTr('Dark Mode')
    readonly property string language: qsTr('Language')
    readonly property string connection: qsTr('Connection')
    readonly property string status: qsTr('Status')
    
    // Tile descriptions
    readonly property string navigationDesc: qsTr('GPS and route planning')
    readonly property string mediaDesc: qsTr('Music and audio')
    readonly property string phoneDesc: qsTr('Calls and contacts')
    
    // Status messages
    readonly property string connected: qsTr('Connected')
    readonly property string disconnected: qsTr('Disconnected')
    readonly property string notConnected: qsTr('Not connected')
}
```

### 2. Use Strings in QML Components

```qml
// ui/qml/screens/HomeScreen.qml
import QtQuick
import QtQuick.Controls
import "../models"

Screen {
    Text {
        text: Strings.appTitle
        // This text will automatically be translated!
    }
    
    Card {
        title: Strings.navigationTitle
        description: Strings.navigationDesc
    }
}
```

---

## Workflow: Adding New Strings

### Step 1: Add `qsTr()` to QML Code

```qml
// ui/qml/screens/MyNewScreen.qml
Text {
    text: qsTr('New Feature Title')  // Mark string as translatable
}
```

### Step 2: Update Translation Files

Run translation tools (automatic via CMake):

```bash
# In build directory
cd crankshaft-mvp/build
cmake ..  # Qt tools automatically extract qsTr() strings
```

This updates all `.ts` files in `ui/i18n/`:
- Adds new strings as `<message>` entries
- Marks them as `type="unfinished"` (awaiting translation)

### Step 3: Translate Strings

Edit the `.ts` file for each locale:

```xml
<!-- ui/i18n/ui_fr_FR.ts -->
<context>
    <name>MyNewScreen</name>
    <message>
        <location filename="../qml/MyNewScreen.qml" line="42"/>
        <source>New Feature Title</source>
        <translation>Titre de la nouvelle fonctionnalité</translation>
    </message>
</context>
```

### Step 4: Mark as Complete

Change the `<translation>` tag status:

```xml
<!-- From -->
<translation type="unfinished">Titre de la nouvelle fonctionnalité</translation>

<!-- To -->
<translation>Titre de la nouvelle fonctionnalité</translation>
```

### Step 5: Rebuild

```bash
cmake --build build
```

Qt tools compile `.ts` files to `.qm` (binary translation files) automatically.

---

## Dynamic Locale Switching

### Using the LocaleSelector Component

Add to Settings Screen:

```qml
// ui/qml/screens/SettingsScreen.qml
import QtQuick
import QtQuick.Controls
import "../components"

Page {
    ColumnLayout {
        // ... existing settings ...
        
        LocaleSelector {
            Layout.fillWidth: true
            
            onLocaleChanged: (newLocale) => {
                // Save preference
                SettingsRegistry.setLocale(newLocale)
                
                // Notify app to reload translations
                Qt.locale.setLocale(newLocale)
            }
        }
    }
}
```

### Manual Locale Switching (C++ Code)

```cpp
// In main.cpp or any C++ code
#include <QLocale>

void setApplicationLocale(const QString& localeCode) {
    // localeCode = "en_GB", "fr_FR", etc.
    QLocale::setDefault(QLocale(localeCode));
    
    // Reload QML with new translations
    // (Qt handles this automatically)
}
```

---

## API Reference

### `qsTr(text, context)`

Marks a string for translation.

```qml
// Basic usage
Text { text: qsTr('Hello') }

// With context (helps translators)
Text { text: qsTr('Back', 'Navigation') }

// With parameters
Text { text: qsTr('Battery: %1%').arg(batteryLevel) }
```

### `QT_TRANSLATE_NOOP(context, text)`

Deferred translation (translates later, not immediately):

```qml
// Use when string is computed at runtime
var messages = [
    qsTranslate('Messages', 'Message 1'),
    qsTranslate('Messages', 'Message 2')
]
```

### `Qt.locale()`

Query or set current locale:

```qml
// Get current locale
console.log(Qt.locale().name)  // Output: "en_GB"

// Set new locale
Qt.locale.setLocale("fr_FR")
```

---

## Building with Translations

### Automatic Build Process

`CMakeLists.txt` configuration:

```cmake
# ui/CMakeLists.txt
set(TS_FILES
  i18n/ui_en_GB.ts
  i18n/ui_en_US.ts
  i18n/ui_de_DE.ts
  i18n/ui_fr_FR.ts
  i18n/ui_es_ES.ts
)

qt_add_translations(crankshaft-ui
  TS_FILES ${TS_FILES}
  RESOURCE_PREFIX "/i18n"
)
```

### Build Steps

1. **Extract Strings**: CMake finds all `qsTr()` calls
2. **Generate .ts Files**: Updates translation files with new strings
3. **Compile Translations**: Converts `.ts` → `.qm` (binary)
4. **Embed Resources**: QRCs include `.qm` files in executable

---

## Troubleshooting

### Problem: Strings Not Being Translated

**Cause**: String not wrapped in `qsTr()`

**Solution**:
```qml
// Wrong
text: "Hello"

// Correct
text: qsTr("Hello")
```

### Problem: Context-Specific Translations Not Working

**Cause**: Missing context parameter

**Solution**:
```qml
// Wrong
text: qsTr("Back")  // Ambiguous context

// Correct
text: qsTr("Back", "Navigation")  // Clear context
```

### Problem: New Strings Not Appearing in Translation Files

**Cause**: CMake cache not updated

**Solution**:
```bash
cd build
rm -rf CMakeCache.txt
cmake ..
cmake --build . --target lrelease
```

### Problem: Locale Selector Not Appearing

**Cause**: Component not imported correctly

**Solution**:
```qml
import "../components"  // Ensure this is present

LocaleSelector {
    // Component should now be available
}
```

---

## Best Practices

### 1. Keep Strings Simple
```qml
// Good
text: qsTr("Battery Low")

// Bad - complex, hard to translate
text: qsTr("Your vehicle's battery level has dropped below 20% and should be charged soon to avoid starting issues")
```

### 2. Use Context for Clarity
```qml
// Good - context disambiguates
qsTr("Close", "Button")
qsTr("Close", "Verb")

// Bad - ambiguous
qsTr("Close")
```

### 3. Parameterize Dynamic Content
```qml
// Good
text: qsTr("Battery: %1%").arg(batteryLevel)

// Bad - hard to translate
text: qsTr("Battery: " + batteryLevel + "%")
```

### 4. Group Related Strings
```qml
// Good - organized context
readonly property string battery: qsTr("Battery", "Status")
readonly property string batteryLow: qsTr("Battery Low", "Status")
readonly property string batteryCharging: qsTr("Charging", "Status")

// Bad - scattered
text: qsTr("Battery")
text: qsTr("Low")
text: qsTr("Charging")
```

### 5. Avoid Concatenation
```qml
// Good
text: qsTr("Hello %1").arg(name)

// Bad - translators can't reorder words
text: qsTr("Hello") + " " + name
```

---

## Translation Coverage

### Current Status (Phase 2025-01-03)

| Locale | Coverage | Status |
|--------|----------|--------|
| en_GB | 100% | ✅ Complete |
| en_US | 100% | ✅ Complete |
| de_DE | 100% | ✅ Complete (existing) |
| fr_FR | 100% | ✅ Complete |
| es_ES | 100% | ✅ Complete |

**Total Translatable Strings**: ~60 (core UI)

### Expanding Translation Coverage

To add more strings to a locale:

1. Edit the `.ts` file directly:
```xml
<!-- ui/i18n/ui_fr_FR.ts -->
<message>
    <source>New String</source>
    <translation>Nouvelle Chaîne</translation>
</message>
```

2. Rebuild:
```bash
cd build && cmake --build . --target lrelease
```

---

## Future Locales

To add a new locale (e.g., Italian):

### Step 1: Create Translation File

Copy an existing file and rename:
```bash
cp ui/i18n/ui_en_GB.ts ui/i18n/ui_it_IT.ts
```

### Step 2: Update CMakeLists.txt

```cmake
set(TS_FILES
  i18n/ui_en_GB.ts
  i18n/ui_en_US.ts
  i18n/ui_de_DE.ts
  i18n/ui_fr_FR.ts
  i18n/ui_es_ES.ts
  i18n/ui_it_IT.ts  # Add new locale
)
```

### Step 3: Translate Strings

Edit `ui_it_IT.ts` with Italian translations.

### Step 4: Update LocaleSelector

```qml
// ui/qml/components/LocaleSelector.qml
property var locales: [
    { code: 'en_GB', name: qsTr('English (GB)') },
    { code: 'en_US', name: qsTr('English (US)') },
    { code: 'de_DE', name: qsTr('Deutsch (DE)') },
    { code: 'fr_FR', name: qsTr('Français (FR)') },
    { code: 'es_ES', name: qsTr('Español (ES)') },
    { code: 'it_IT', name: qsTr('Italiano (IT)') }  # Add new locale
]
```

---

## Command Reference

### Updating Translation Files

```bash
cd build

# Full rebuild (extracts strings and compiles)
cmake ..
cmake --build .

# Just compile translations (after manual .ts edits)
cmake --build . --target lrelease
```

### Checking Translation Status

```bash
# Count untranslated strings
grep 'type="unfinished"' ui/i18n/ui_*.ts | wc -l

# List all untranslated strings
grep -B2 'type="unfinished"' ui/i18n/ui_fr_FR.ts | grep "<source>"
```

---

## Testing Translations

### Manual Testing

1. **Change System Locale**:
   ```bash
   export LANG=de_DE.UTF-8
   ./build/ui/crankshaft-ui
   ```

2. **Verify UI Updates**: Check that all text displays in German

3. **Test LocaleSelector**: Navigate to Settings → Language, select new locale

### Automated Testing

```bash
# Test each locale
for locale in en_GB en_US de_DE fr_FR es_ES; do
    LANG=$locale ./build/ui/crankshaft-ui --headless-test
done
```

---

## Integration with Extension System

Extensions can define their own translatable strings:

```qml
// In extension QML
import QtQuick

QtObject {
    property string title: qsTr('My Extension', 'MyExtension')
    property string description: qsTr('Does something cool', 'MyExtension')
}
```

Extension manifests should list supported locales:

```json
{
    "id": "my-extension",
    "i18n": {
        "default": "en_GB",
        "supported": ["en_GB", "en_US", "de_DE", "fr_FR", "es_ES"]
    }
}
```

---

## Performance Notes

### Translation Overhead

- **Minimal**: `qsTr()` is zero-cost when translations are compiled
- **Compiled Format**: `.qm` files are binary, fast to load
- **Memory**: Translations loaded into memory at startup (~10KB per locale)

### Optimization Tips

1. **Use CONSTANT Properties**: Mark static strings as read-only
2. **Lazy Load**: Only load locale translations when switched
3. **Cache Translations**: Don't repeatedly call `qsTr()`

---

## References

- [Qt Linguist Documentation](https://doc.qt.io/qt-6/qtlinguist-index.html)
- [Qt qsTr() Documentation](https://doc.qt.io/qt-6/qml-qtqml-qt.html#qsTr-method)
- [CMake qt_add_translations](https://doc.qt.io/qt-6/qt-add-translations.html)
- [Locale Codes (IETF BCP 47)](https://en.wikipedia.org/wiki/IETF_language_tag)

---

## Summary

| Aspect | Status |
|--------|--------|
| Framework | ✅ Qt Linguist (native Qt i18n) |
| Languages | ✅ 5 supported (en_GB, en_US, de_DE, fr_FR, es_ES) |
| Strings | ✅ 60+ translatable UI strings |
| Components | ✅ LocaleSelector for dynamic switching |
| Build Integration | ✅ CMake automated translation compilation |
| Documentation | ✅ Complete with examples |
| Testing | ✅ Manual and automated approaches |

---

**Next Steps**:
1. Deploy to production with 5 locales
2. Gather user feedback on translations
3. Add community contributions for additional locales
4. Integrate with extension translation framework

**Completion Date**: January 3, 2026  
**Maintainer**: Crankshaft Development Team
