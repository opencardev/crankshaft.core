#!/bin/bash
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

set -e

echo "=== Running code quality checks ==="

# Check for required tools
MISSING_TOOLS=()
command -v clang-format >/dev/null 2>&1 || MISSING_TOOLS+=("clang-format")
command -v find >/dev/null 2>&1 || MISSING_TOOLS+=("find")

if [ ${#MISSING_TOOLS[@]} -ne 0 ]; then
    echo "Warning: Missing tools: ${MISSING_TOOLS[*]}"
    echo "Some checks will be skipped"
fi

# 1. Check C++ formatting
echo "=== Checking C++ formatting ==="
if command -v clang-format >/dev/null 2>&1; then
    if find core ui -type f \( -name '*.cpp' -o -name '*.hpp' -o -name '*.h' -o -name '*.cc' \) -print0 2>/dev/null | xargs -0 clang-format --dry-run --Werror 2>&1; then
        echo "✓ C++ formatting check passed"
    else
        echo "✗ C++ formatting check failed"
        echo "Run: find core ui -type f \( -name '*.cpp' -o -name '*.hpp' -o -name '*.h' -o -name '*.cc' \) -print0 | xargs -0 clang-format -i"
        exit 1
    fi
else
    echo "⊘ clang-format not found, skipping formatting check"
fi

# 2. Check for license headers
echo "=== Checking license headers ==="
MISSING_HEADERS=$(find core ui -type f \( -name '*.cpp' -o -name '*.hpp' -o -name '*.h' \) -exec grep -L 'GNU General Public License' {} \; 2>/dev/null || true)
if [ -n "$MISSING_HEADERS" ]; then
    echo "✗ Files missing license headers:"
    echo "$MISSING_HEADERS"
    exit 1
else
    echo "✓ License header check passed"
fi

# 3. Check for common issues
echo "=== Checking for common issues ==="
# Check for CRLF line endings in source files
if find core ui -type f \( -name '*.cpp' -o -name '*.hpp' -o -name '*.h' \) -exec file {} \; 2>/dev/null | grep -q CRLF; then
    echo "✗ CRLF line endings found in source files"
    find core ui -type f \( -name '*.cpp' -o -name '*.hpp' -o -name '*.h' \) -exec file {} \; | grep CRLF
    exit 1
else
    echo "✓ No CRLF line endings found"
fi

echo "=== All code quality checks passed ==="
