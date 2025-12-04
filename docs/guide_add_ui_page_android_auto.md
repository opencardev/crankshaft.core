# Add a New UI Page and Integrate it into HomeScreen (Android Auto Example)

This guide explains how to add a new QML UI page to Crankshaft and integrate it into the HomeScreen navigation, using an "Android Auto" page as the example. It follows the project standards (Qt 6, C++20, British English, i18n-ready UI) and assumes you are building on Linux via WSL from Windows.

## Prerequisites
- Buildable workspace (CMake, Qt 6, Linguist tools). 
- The UI app runs (e.g., via the provided VS Code tasks). 
- Basic familiarity with QML and the existing structure under `ui/qml/`.

  title: qsTr("Android Auto")
  description: qsTr("Connect your phone")
  icon: "phone" // Icon component maps known names; use "phone" for Android Auto
  onClicked: {
    // Pass the StackView so the page can navigate back reliably
    stackView.push(androidautoScreen, { stack: stackView })
  }
## Quick Checklist

1. Create the page: add `ui/qml/AndroidAutoScreen.qml` with `qsTr()` for all visible strings and use `Theme` styles.
2. Register the page: add `qml/AndroidAutoScreen.qml` under `QML_FILES` in `ui/CMakeLists.txt` `qt_add_qml_module(...)`.
3. Add card in HomeScreen: use supported properties (`title`, `description`, `icon`), and push with the stack: `stackView.push(androidautoScreen, { stack: stackView })`.
4. Implement Back on the page: 
   ```qml
   Page {
     property var stack: null
     AppButton {
       text: qsTr("← Back")
       onClicked: {
         if (stack) { stack.pop() }
         else if (StackView.view) { StackView.view.pop() }
         else { console.warn("No stack available to pop") }
       }
     }
   }
   ```
5. Update translations: add new strings to `ui/i18n/ui_en_GB.ts` and `ui/i18n/ui_de_DE.ts`; build with the `Build Translations` task or WSL command.
6. Rebuild and run:
   ```powershell
   wsl bash -lc "cd /mnt/c/Users/matth/install/repos/opencardev/oct_2025/crankshaft-mvp; cmake --build build --target crankshaft-ui -j"
   wsl bash -lc "cd /mnt/c/Users/matth/install/repos/opencardev/oct_2025/crankshaft-mvp; ./build/ui/crankshaft-ui"
   ```

## 1) Create the Android Auto Page (QML)
Create a new file `ui/qml/AndroidAutoScreen.qml`.

Ensure `stackView` is accessible in `HomeScreen.qml` (this is typically set in `Main.qml` where the `StackView` is created and `HomeScreen` is the initial item). If not, expose `stackView` via an `id` or context property.

### Navigation best practices
- Prefer passing the `StackView` reference when pushing pages: `stackView.push(androidautoScreen, { stack: stackView })`.
- In the pushed page (`AndroidAutoScreen.qml`), add:

```qml
Page {
  property var stack: null
  AppButton {
    text: qsTr("← Back")
    onClicked: {
      if (stack) {
        stack.pop()
      } else if (StackView.view) {
        StackView.view.pop()
      } else {
        console.warn("AndroidAutoScreen: No stack available to pop")
      }
    }
  }
}
```

### Card component properties
Use only supported properties provided by `Components/Card.qml`:
- `title`, `description`, `icon`

Do not use `subtitle` or `iconName` — they are not defined and will cause errors like:
`Type HomeScreen unavailable ... Cannot assign to non-existent property "iconName"`.

## 2.2) Register the page in ui/CMakeLists.txt
To make `AndroidAutoScreen` available as a type, add it to the QML module in `ui/CMakeLists.txt`:

```cmake
qt_add_qml_module(crankshaft-ui
  URI Crankshaft
  VERSION 1.0
  QML_FILES
    qml/Main.qml
    qml/HomeScreen.qml
    qml/SettingsScreen.qml
    qml/AndroidAutoScreen.qml
    qml/Components/AppButton.qml
    qml/Components/Card.qml
    qml/Components/Icon.qml
  RESOURCES
    assets/icons/mdi.svg
)
```

Then rebuild the UI target:

```powershell
wsl bash -lc "cd /mnt/c/Users/matth/install/repos/opencardev/oct_2025/crankshaft-mvp; cmake --build build --target crankshaft-ui -j"
```

If you see `AndroidAutoScreen is not a type`, it means the file is not included in the QML module — revisit the CMake changes above.
/*
 * Project: Crankshaft
 * This file is part of Crankshaft project.
 * Copyright (C) 2025 OpenCarDev Team
 *
 *  Crankshaft is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Crankshaft is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Crankshaft. If not, see <http://www.gnu.org/licenses/>.
*/
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "Components"

Page {
  id: root

  background: Rectangle { color: Theme.background }

  header: Rectangle {
    width: parent.width
    height: 80
    color: Theme.surface
    RowLayout {
      anchors.fill: parent
      anchors.margins: Theme.spacingMd
      AppButton {
        text: qsTr("← Back")
        onClicked: stackView.pop()
      }
      Text {
        text: qsTr("Android Auto")
        font.pixelSize: Theme.fontSizeHeading2
        font.bold: true
        color: Theme.textPrimary
        Layout.fillWidth: true
      }
    }
  }

  ScrollView {
    anchors.fill: parent
    contentWidth: availableWidth
    ColumnLayout {
      width: parent.width
      spacing: Theme.spacingMd
      Rectangle {
        Layout.fillWidth: true
        Layout.margins: Theme.spacingMd
        height: contentCol.height + Theme.spacingLg * 2
        color: Theme.surface
        radius: Theme.radiusMd
        ColumnLayout {
          id: contentCol
          anchors.centerIn: parent
          width: parent.width - Theme.spacingLg * 2
          spacing: Theme.spacingMd
          Text {
            text: qsTr("Status")
            font.pixelSize: Theme.fontSizeHeading3
            font.bold: true
            color: Theme.textPrimary
          }
          Text {
            text: qsTr("Not connected")
            font.pixelSize: Theme.fontSizeBody
            color: Theme.textSecondary
          }
          RowLayout {
            Layout.fillWidth: true
            spacing: Theme.spacingMd
            AppButton {
              text: qsTr("Start Android Auto")
              onClicked: {
                // Example: Publish an event to core to start AASDK/OpenAuto
                wsClient.publish("androidauto/start", { })
              }
            }
            AppButton {
              text: qsTr("Stop Android Auto")
              onClicked: {
                wsClient.publish("androidauto/stop", { })
              }
            }
          }
        }
      }
    }
  }
}
```

Notes:
- Use `qsTr()` for all visible strings (i18n-ready).
- Use `Theme` palette and spacing.
- Use `wsClient.publish()` to communicate with core over WebSockets.

## 2) Add Navigation from HomeScreen
Open `ui/qml/HomeScreen.qml` and add a new tile/card that pushes `AndroidAutoScreen.qml` onto the `StackView`.

Example snippet (place within the grid/list of cards):

```qml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "Components"

// ... inside the existing layout where other cards are defined
Card {
  title: qsTr("Android Auto")
  subtitle: qsTr("Connect your phone")
  iconName: "icons/android_auto.svg" // optional, add an icon into ui/assets/icons if desired
  onClicked: {
    stackView.push("AndroidAutoScreen.qml")
  }
}
```

Ensure `stackView` is accessible in `HomeScreen.qml` (this is typically set in `Main.qml` where the `StackView` is created and `HomeScreen` is the initial item). If not, expose `stackView` via an `id` or context property.

## 2.1) Ensure Main.qml can resolve the new page
Open `ui/qml/Main.qml` and verify two things so the new page loads correctly:

- The `StackView` exists with an `id` (e.g., `id: stackView`) and is in scope for `HomeScreen`.
- The QML import path includes the folder containing your new page, or you reference it by relative path.

Example minimal `Main.qml` structure:

```qml
import QtQuick
import QtQuick.Controls

ApplicationWindow {
  id: app
  visible: true
  width: 1024; height: 600

  StackView {
    id: stackView
    anchors.fill: parent
    initialItem: HomeScreen {}
  }
}
```

Referencing the page by filename (`stackView.push("AndroidAutoScreen.qml")`) works because the repository’s UI CMake rules copy `ui/qml` into the build and bundle it into resources. Keep the new screen under `ui/qml/` so it’s included by the existing copy/resource steps.

## 3) Wire Events to Core (Optional)
In core, you may choose to subscribe to the topics used by the page:
- `androidauto/start`
- `androidauto/stop`

Implement the Android Auto functionality using AASDK/OpenAuto in a dedicated extension or core module. Keep the core slim; integrate via the event bus and WebSockets.

## 4) i18n: Add and Build Translations (Detailed)
All user-visible strings must use `qsTr()` in QML. The project compiles `.ts` files to `.qm` and embeds them under the resource prefix `:/i18n`.

### 4.1) Where translation sources live
- English (GB): `ui/i18n/ui_en_GB.ts`
- German (DE): `ui/i18n/ui_de_DE.ts`

Add entries for new strings such as "Android Auto", "Start Android Auto", "Stop Android Auto", "Status", and "Not connected".

### 4.2) Update the `.ts` files
You can run `lupdate` via CMake targets or invoke it manually. The repo provides a translations target:

Using VS Code task:
- Run `Build Translations`

Using terminal (WSL):

```bash
wsl bash -lc "cmake --build build --target translations -j || ninja -C build translations"
```

This will:
- Scan QML for `qsTr()` strings
- Update `ui_en_GB.ts` and `ui_de_DE.ts`
- Compile to `ui_en_GB.qm` and `ui_de_DE.qm` (placed under `build/ui/`)
- Embed them into the UI binary with resource aliases `:/i18n/ui_en_GB.qm` and `:/i18n/ui_de_DE.qm`

### 4.3) Language codes and file naming
- The runtime language codes in the app use hyphens (e.g., `en-GB`, `de-DE`).
- The compiled translation filenames use underscores (e.g., `ui_en_GB.qm`, `ui_de_DE.qm`).
- The UI loader converts hyphens to underscores automatically when resolving the resource path, e.g., `de-DE` → `ui_de_DE.qm`.

### 4.4) Runtime retranslation
Language switching is handled in `ui/main.cpp` by listening to the `ui/language/changed` event. When the language changes, the app:
- Removes the old translator
- Loads the new file from `:/i18n/ui_<lang>.qm`
- Updates the context property `currentLanguage`
- Calls `engine.retranslate()` so all `qsTr()` strings refresh immediately

If strings do not update, verify:
- You are using `qsTr()` for all visible text
- Translations were rebuilt and embedded (see `build/ui/*.qm` and `build/ui/.qt/rcc/crankshaft-ui_translations.qrc`)
- The language code sent over WebSocket matches expected codes (e.g., `de-DE`)

## 5) Build and Run
Use the provided tasks in VS Code:
- `Build (Debug)`
- `Run Application (VNC Debug)` to start with a virtual display (e.g., `-platform vnc:size=1024x600,port=5900`).

Or run from terminal:

```powershell
wsl bash -lc "cd /mnt/c/Users/matth/install/repos/opencardev/oct_2025/crankshaft-mvp; cmake --build build --config Debug -j"
wsl bash -lc "cd /mnt/c/Users/matth/install/repos/opencardev/oct_2025/crankshaft-mvp; QT_DEBUG_PLUGINS=1 QT_LOGGING_RULES='*=true' ./build/CrankshaftReborn -platform vnc:size=1024x600,port=5900"
```

## 6) UI/UX Guidelines Recap
- Modern, clean UI; follow `Theme` colours, spacing, radii.
- Provide a light/dark mode friendly layout.
- Use `AppButton`, `Card`, and other components for consistency.
- Keep page logic minimal; communicate with core over WebSockets.

## 7) Testing Tips
- Verify navigation from HomeScreen to Android Auto page and back.
- Check language switching updates all new strings.
- Watch logs for `wsClient.publish(...)` being received and handled by core.

## 8) Optional: Add an Icon
If desired, place an icon under `ui/assets/icons/` and reference it in the card. Ensure the asset is included by the UI CMake rules (the repo already copies assets via `crankshaft-ui_copy_res`).

---

By following these steps, you can add new pages (like Android Auto) to the UI, wire them into the HomeScreen, and route interactions through the event system while keeping i18n and styling consistent.

## Troubleshooting

- Error: `QQmlApplicationEngine failed to load component` / `AndroidAutoScreen is not a type`
  - Cause: The new page is not registered in the QML module.
  - Fix: Add `qml/AndroidAutoScreen.qml` to `qt_add_qml_module(... QML_FILES ...)` in `ui/CMakeLists.txt`, then rebuild the `crankshaft-ui` target.

- Error: `Type HomeScreen unavailable` / `Cannot assign to non-existent property "iconName"`
  - Cause: Using properties that `Components/Card.qml` does not define.
  - Fix: Use only supported properties: `title`, `description`, `icon`. Do not use `subtitle` or `iconName`.

- Error: `TypeError: Cannot call method 'pop' of null`
  - Cause: Page attempts to call `pop()` without a valid reference to the hosting `StackView`.
  - Fix: Pass the stack on push: `stackView.push(androidautoScreen, { stack: stackView })` and in the page use a robust Back handler:
    ```qml
    Page {
      property var stack: null
      AppButton {
        text: qsTr("← Back")
        onClicked: {
          if (stack) { stack.pop() }
          else if (StackView.view) { StackView.view.pop() }
          else { console.warn("No stack available to pop") }
        }
      }
    }
    ```

- Translations do not load or change at runtime
  - Symptoms: `Failed to load translation: "de-DE"` or strings not updating after language switch.
  - Causes & Fixes:
    - Filenames use underscores, language codes use hyphens. Convert when loading, e.g. `QString(lang).replace('-', '_')` → `:/i18n/ui_de_DE.qm`.
    - Rebuild translations: `Build Translations` task or
      ```bash
      wsl bash -lc "cmake --build build --target translations -j || ninja -C build translations"
      ```
    - Ensure all visible strings use `qsTr()` and the app calls `engine.retranslate()` after installing the new translator.
    - Verify the compiled `.qm` files exist: `build/ui/ui_en_GB.qm`, `build/ui/ui_de_DE.qm`.

- Quick sanity commands
  ```powershell
  wsl bash -lc "cd /mnt/c/Users/matth/install/repos/opencardev/oct_2025/crankshaft-mvp; cmake --build build --target crankshaft-ui -j"
  wsl bash -lc "cd /mnt/c/Users/matth/install/repos/opencardev/oct_2025/crankshaft-mvp; ./build/ui/crankshaft-ui"
  wsl bash -lc "cd /mnt/c/Users/matth/install/repos/opencardev/oct_2025/crankshaft-mvp; find build/ui -name '*.qm'"
  ```