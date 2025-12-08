# Material Design Icons Integration

This project integrates the complete Material Design Icons (MDI) library, providing access to **7,447+ icons** in the QML user interface.

## Overview

- **Font File**: Material Design Icons v7.4.47 webfont (1.3 MB)
- **Total Icons**: 7,447 icons available
- **License**: Icons are licensed under Apache License 2.0 / SIL Open Font License
- **Source**: [Material Design Icons](https://pictogrammers.com/library/mdi/)

## Architecture

### Components

1. **MaterialDesignIcons.js** - JavaScript library with all 7,447 icon codepoint mappings
2. **Icon.qml** - QML component that renders icons using the MDI font
3. **materialdesignicons-webfont.ttf** - The actual font file (auto-downloaded during build)

### How It Works

1. CMake downloads the MDI font during configuration
2. The font is embedded as a Qt resource at `qrc:/fonts/materialdesignicons-webfont.ttf`
3. `Icon.qml` loads the font using `FontLoader`
4. Icons prefixed with `mdi-` automatically use the MDI font
5. `MaterialDesignIcons.js` maps icon names to Unicode codepoints using `String.fromCodePoint()`

## Usage

### In QML Components

```qml
import Crankshaft 1.0

// Simple usage - just provide the icon name with "mdi-" prefix
Icon {
    name: "mdi-account-cog"
    size: 24
    color: Theme.textPrimary
}

// Available properties
Icon {
    name: "mdi-bluetooth"     // Icon name (required, use "mdi-" prefix for MDI icons)
    size: 48                  // Icon size in pixels (default: 24)
    color: "#FF5722"          // Icon color (default: Theme.textPrimary)
}
```

### Common Icon Examples

```qml
// User & Account
Icon { name: "mdi-account" }
Icon { name: "mdi-account-circle" }
Icon { name: "mdi-account-cog" }

// Navigation
Icon { name: "mdi-menu" }
Icon { name: "mdi-arrow-left" }
Icon { name: "mdi-home" }
Icon { name: "mdi-map-marker" }

// Media
Icon { name: "mdi-play" }
Icon { name: "mdi-pause" }
Icon { name: "mdi-volume-high" }
Icon { name: "mdi-music" }

// Communication
Icon { name: "mdi-phone" }
Icon { name: "mdi-email" }
Icon { name: "mdi-message" }
Icon { name: "mdi-bluetooth" }

// Connectivity
Icon { name: "mdi-wifi" }
Icon { name: "mdi-network" }
Icon { name: "mdi-usb" }
Icon { name: "mdi-ethernet" }

// System
Icon { name: "mdi-cog" }
Icon { name: "mdi-monitor" }
Icon { name: "mdi-harddisk" }
Icon { name: "mdi-memory" }
```

## Finding Icons

### Online Icon Browser

Visit [Material Design Icons](https://pictogrammers.com/library/mdi/) to browse and search all available icons.

1. Search for the icon you need
2. Click on the icon
3. Copy the icon name (e.g., "account-circle")
4. Use it in QML with the "mdi-" prefix: `name: "mdi-account-circle"`

### Common Categories

- **Alert & Status**: alert, information, check, close, help
- **Audio & Video**: play, pause, stop, volume, music, video
- **Communication**: phone, email, message, chat, forum
- **Content**: add, remove, edit, save, delete, archive
- **Device**: bluetooth, usb, wifi, battery, signal
- **File**: folder, file, download, upload, cloud
- **Hardware**: keyboard, mouse, monitor, printer, phone
- **Navigation**: menu, home, arrow-left, arrow-right, navigation
- **Social**: account, share, heart, star, comment
- **Toggle**: checkbox, radio-button, toggle-switch

## Icon Mappings Generation

The icon mappings are **automatically generated** during the CMake build process:

1. **First Build**: If `MaterialDesignIcons.js` doesn't exist, it's automatically generated
2. **Subsequent Builds**: The file is reused (generation skipped for faster builds)
3. **Force Regeneration**: Use `cmake -DFORCE_GENERATE_MDI_MAPPINGS=ON ..` to regenerate

### Manual Generation

You can also run the generator script manually:

```bash
python3 scripts/generate_mdi_mappings.py
```

The script will:
1. Download the latest icon metadata from GitHub
2. Generate MaterialDesignIcons.js with all codepoint mappings
3. Output the total number of icons available

### Updating to a Newer Version

To update to a newer version of Material Design Icons:

1. Edit `MDI_VERSION` in `cmake/DownloadMaterialDesignIcons.cmake`
2. Run: `cmake -DFORCE_GENERATE_MDI_MAPPINGS=ON ..`
3. Rebuild: `cmake --build build`

## Fallback Icons

The `Icon` component also supports non-MDI icons using Unicode characters. These don't require the "mdi-" prefix:

```qml
Icon { name: "home" }        // Uses ⌂
Icon { name: "settings" }    // Uses ⚙
Icon { name: "star" }        // Uses ★
Icon { name: "check" }       // Uses ✓
```

See `Icon.qml` for the full list of fallback icons.

## Technical Details

### Unicode Codepoints

Material Design Icons use the Private Use Area (PUA) of Unicode, starting at U+F0000. These codepoints are above the Basic Multilingual Plane (BMP), so we must use `String.fromCodePoint()` instead of `String.fromCharCode()`.

Example:
- Icon name: `account-cog`
- Codepoint: `0xF0495`
- JavaScript: `String.fromCodePoint(0xF0495)`
- Result: `"󰒕"` (displayed using MDI font)

### Font Loading

```qml
FontLoader {
    id: mdiFont
    source: "qrc:/fonts/materialdesignicons-webfont.ttf"
}

Text {
    text: String.fromCodePoint(0xF0495)  // account-cog icon
    font.family: mdiFont.name             // "Material Design Icons"
}
```

### Performance Considerations

- **Font file size**: 1.3 MB (embedded in binary)
- **Mapping file size**: ~350 KB (compiled with AOT)
- **Load time**: Font loads once on first use
- **Memory**: ~1.5 MB for font + mappings
- **Rendering**: Hardware-accelerated, scales perfectly at any size

## Build Integration

The MDI font is automatically downloaded during CMake configuration:

```cmake
# CMakeLists.txt
include(DownloadMaterialDesignIcons)
download_material_design_icons()
```

The download script tries multiple sources:
1. jsDelivr CDN (fast, reliable)
2. GitHub releases (fallback)
3. GitHub raw content (last resort)

If you have internet restrictions, you can manually place the font file at:
```
build/ui/fonts/materialdesignicons-webfont.ttf
```

## Troubleshooting

### Icons show as "?" or boxes

1. Check that the icon name is correct and has "mdi-" prefix
2. Verify the icon exists: [Browse MDI Icons](https://pictogrammers.com/library/mdi/)
3. Check console for font loading errors
4. Ensure the font file was downloaded during build

### Font file not found

1. Delete the `build` folder
2. Run `cmake ..` to trigger font download
3. Check your internet connection
4. Try manually downloading from: https://cdn.jsdelivr.net/npm/@mdi/font@7.4.47/fonts/materialdesignicons-webfont.ttf

### Icons have wrong size/color

The `Icon` component uses these properties:
- `size`: Pixel size (default: 24)
- `color`: Icon color (default: Theme.textPrimary)

Ensure you're setting these properties correctly.

## License

- **Material Design Icons**: Apache License 2.0 / SIL Open Font License
- **Crankshaft Integration Code**: GPL v3 (see LICENSE)

## References

- [Material Design Icons Website](https://pictogrammers.com/library/mdi/)
- [MDI GitHub Repository](https://github.com/Templarian/MaterialDesign)
- [MDI NPM Package](https://www.npmjs.com/package/@mdi/font)
- [Icon Font Usage Guide](https://pictogrammers.com/docs/library/mdi/getting-started/webfont/)
