# Project: Crankshaft
# This file is part of Crankshaft project.
# Copyright (C) 2025 OpenCarDev Team
#
#  Crankshaft is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 3 of the License, or
#  (at your option) any later version.
#
#  Crankshaft is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with Crankshaft. If not, see <http://www.gnu.org/licenses/>.

"""
Generate Material Design Icons codepoint mappings for QML.
Downloads the official metadata and creates a JavaScript/QML compatible mapping file.
"""

import json
import urllib.request
import sys
from pathlib import Path


def download_mdi_metadata(version="7.4.47"):
    """Download MDI metadata from CDN."""
    # Try meta.json first as it's the most reliable source
    try:
        url = f"https://raw.githubusercontent.com/Templarian/MaterialDesign/master/meta.json"
        print(f"Downloading MDI metadata from GitHub...")
        with urllib.request.urlopen(url, timeout=30) as response:
            meta = json.loads(response.read().decode('utf-8'))
        return {icon['name']: icon['codepoint'] for icon in meta}
    except Exception as e:
        print(f"Error downloading GitHub metadata: {e}", file=sys.stderr)
        
        # Fallback to CSS parsing
        try:
            url = f"https://cdn.jsdelivr.net/npm/@mdi/font@{version}/css/materialdesignicons.css"
            print(f"Trying CSS metadata v{version}...")
            with urllib.request.urlopen(url, timeout=30) as response:
                css_content = response.read().decode('utf-8')
            return parse_css_codepoints(css_content)
        except Exception as e2:
            print(f"Error downloading CSS metadata: {e2}", file=sys.stderr)
            return None


def parse_css_codepoints(css_content):
    """Parse CSS content to extract icon name to codepoint mappings."""
    import re
    
    mappings = {}
    # Match patterns like: .mdi-account:before { content: "\F0001"; }
    pattern = r'\.mdi-([a-z0-9-]+):before\s*{\s*content:\s*"\\([A-F0-9]+)"'
    
    for match in re.finditer(pattern, css_content, re.IGNORECASE):
        icon_name = match.group(1)
        codepoint = match.group(2)
        mappings[icon_name] = codepoint
    
    return mappings


def generate_qml_mapping_file(mappings, output_path):
    """Generate QML-compatible JavaScript mapping file."""
    
    header = '''/*
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

// AUTO-GENERATED FILE - DO NOT EDIT MANUALLY
// Generated from Material Design Icons metadata
// To regenerate: python scripts/generate_mdi_mappings.py

.pragma library

'''
    
    with open(output_path, 'w', encoding='utf-8') as f:
        f.write(header)
        f.write('// Material Design Icons codepoint mappings\n')
        f.write('// Reference: https://pictogrammers.com/library/mdi/\n\n')
        f.write('function getMaterialIcon(iconName) {\n')
        f.write('    const mdiMap = {\n')
        
        # Sort by icon name for consistent output
        sorted_icons = sorted(mappings.items())
        
        for i, (icon_name, codepoint) in enumerate(sorted_icons):
            # Convert hex string to integer for String.fromCodePoint
            codepoint_int = int(codepoint, 16)
            comma = ',' if i < len(sorted_icons) - 1 else ''
            f.write(f'        "{icon_name}": String.fromCodePoint(0x{codepoint}){comma}\n')
        
        f.write('    };\n\n')
        f.write('    return mdiMap[iconName] || "?";\n')
        f.write('}\n')
    
    print(f"Generated {len(mappings)} icon mappings to {output_path}")


def main():
    """Main entry point."""
    script_dir = Path(__file__).parent
    project_root = script_dir.parent
    output_path = project_root / "ui" / "qml" / "components" / "MaterialDesignIcons.js"
    
    print("Material Design Icons Mapping Generator")
    print("=" * 50)
    print("This script is automatically run during CMake configuration.")
    print("To force regeneration: cmake -DFORCE_GENERATE_MDI_MAPPINGS=ON ..")
    print()
    
    mappings = download_mdi_metadata()
    
    if not mappings:
        print("Failed to download metadata. Using fallback set.", file=sys.stderr)
        # Fallback to basic set
        mappings = {
            "account-cog": "F0495",
            "stethoscope": "F0A4E",
            "text-box": "F060E",
            "network": "F06F3",
            "monitor": "F0379",
            "volume-high": "F057E",
            "usb": "F0553",
            "bluetooth": "F00AF",
            "speedometer": "F0505",
            "harddisk": "F02CA",
            "update": "F06A0",
            "information": "F02FD"
        }
    
    generate_qml_mapping_file(mappings, output_path)
    print("\nDone!")
    print(f"\nTotal icons available: {len(mappings)}")
    print("\nUsage in QML:")
    print('  import "components/MaterialDesignIcons.js" as MDI')
    print('  text: MDI.getMaterialIcon("account-cog")')


if __name__ == "__main__":
    main()
