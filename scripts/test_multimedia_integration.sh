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

# Android Auto Multimedia Integration Verification Script

set -e  # Exit on error

echo "=========================================="
echo "Android Auto Multimedia Integration Test"
echo "=========================================="
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Test results
TESTS_PASSED=0
TESTS_FAILED=0

# Function to print test result
test_result() {
    if [ $1 -eq 0 ]; then
        echo -e "${GREEN}✓ PASS${NC}: $2"
        ((TESTS_PASSED++))
    else
        echo -e "${RED}✗ FAIL${NC}: $2"
        ((TESTS_FAILED++))
    fi
}

echo "1. Checking GStreamer installation..."
if gst-inspect-1.0 --version &>/dev/null; then
    GST_VERSION=$(gst-inspect-1.0 --version | grep version | awk '{print $2}')
    test_result 0 "GStreamer installed (version $GST_VERSION)"
else
    test_result 1 "GStreamer not found"
fi

echo ""
echo "2. Checking GStreamer H.264 decoders..."

# Check for hardware decoders
DECODERS_FOUND=""

if gst-inspect-1.0 vaapih264dec &>/dev/null; then
    DECODERS_FOUND="${DECODERS_FOUND}VA-API "
    test_result 0 "VA-API decoder (vaapih264dec) available"
else
    echo -e "${YELLOW}⚠ INFO${NC}: VA-API decoder not available (Intel/AMD only)"
fi

if gst-inspect-1.0 omxh264dec &>/dev/null; then
    DECODERS_FOUND="${DECODERS_FOUND}OMX "
    test_result 0 "OMX decoder (omxh264dec) available"
else
    echo -e "${YELLOW}⚠ INFO${NC}: OMX decoder not available (Raspberry Pi only)"
fi

if gst-inspect-1.0 nvh264dec &>/dev/null; then
    DECODERS_FOUND="${DECODERS_FOUND}NVDEC "
    test_result 0 "NVDEC decoder (nvh264dec) available"
else
    echo -e "${YELLOW}⚠ INFO${NC}: NVDEC decoder not available (NVIDIA only)"
fi

# Check for software decoder (should always be available)
if gst-inspect-1.0 avdec_h264 &>/dev/null; then
    DECODERS_FOUND="${DECODERS_FOUND}Software "
    test_result 0 "Software decoder (avdec_h264) available"
else
    test_result 1 "Software decoder not found (required)"
fi

echo ""
echo "3. Checking build artifacts..."

if [ -f "build/core/crankshaft-core" ]; then
    test_result 0 "Core executable exists"
else
    test_result 1 "Core executable not found"
fi

if [ -f "build/ui/crankshaft-ui" ]; then
    test_result 0 "UI executable exists"
else
    test_result 1 "UI executable not found"
fi

echo ""
echo "4. Checking source files..."

FILES=(
    "core/hal/multimedia/IVideoDecoder.h"
    "core/hal/multimedia/GStreamerVideoDecoder.h"
    "core/hal/multimedia/GStreamerVideoDecoder.cpp"
    "core/hal/multimedia/IAudioMixer.h"
    "core/hal/multimedia/AudioMixer.h"
    "core/hal/multimedia/AudioMixer.cpp"
    "core/services/android_auto/ProtocolHelpers.h"
    "core/services/android_auto/ProtocolHelpers.cpp"
)

for file in "${FILES[@]}"; do
    if [ -f "$file" ]; then
        test_result 0 "$(basename $file) exists"
    else
        test_result 1 "$(basename $file) not found"
    fi
done

echo ""
echo "5. Checking integration in RealAndroidAutoService..."

if grep -q "IVideoDecoder\* m_videoDecoder" core/services/android_auto/RealAndroidAutoService.h; then
    test_result 0 "Video decoder member declared"
else
    test_result 1 "Video decoder member not found"
fi

if grep -q "IAudioMixer\* m_audioMixer" core/services/android_auto/RealAndroidAutoService.h; then
    test_result 0 "Audio mixer member declared"
else
    test_result 1 "Audio mixer member not found"
fi

if grep -q "GStreamerVideoDecoder" core/services/android_auto/RealAndroidAutoService.cpp; then
    test_result 0 "GStreamer decoder instantiated"
else
    test_result 1 "GStreamer decoder not instantiated"
fi

if grep -q "AudioMixer" core/services/android_auto/RealAndroidAutoService.cpp; then
    test_result 0 "Audio mixer instantiated"
else
    test_result 1 "Audio mixer not instantiated"
fi

if grep -q "crankshaft::protocol" core/services/android_auto/RealAndroidAutoService.cpp; then
    test_result 0 "Protocol helpers used"
else
    test_result 1 "Protocol helpers not used"
fi

echo ""
echo "6. Checking documentation..."

DOCS=(
    "docs/MULTIMEDIA_HAL.md"
    "docs/MULTIMEDIA_QUICKSTART.md"
    "docs/fix_summaries/multimedia_implementation.md"
    "docs/INTEGRATION_COMPLETE.md"
)

for doc in "${DOCS[@]}"; do
    if [ -f "$doc" ]; then
        test_result 0 "$(basename $doc) exists"
    else
        test_result 1 "$(basename $doc) not found"
    fi
done

echo ""
echo "=========================================="
echo "Test Summary"
echo "=========================================="
echo -e "Passed: ${GREEN}${TESTS_PASSED}${NC}"
echo -e "Failed: ${RED}${TESTS_FAILED}${NC}"
echo ""

if [ -n "$DECODERS_FOUND" ]; then
    echo "Available hardware decoders: $DECODERS_FOUND"
else
    echo -e "${YELLOW}Warning: No hardware decoders available${NC}"
    echo "Software decoding will be used (higher CPU usage)"
fi

echo ""

if [ $TESTS_FAILED -eq 0 ]; then
    echo -e "${GREEN}✓ All tests passed!${NC}"
    echo ""
    echo "Integration is complete and ready for device testing."
    echo "Next steps:"
    echo "  1. Connect Android phone via USB"
    echo "  2. Run: ./build/core/crankshaft-core"
    echo "  3. Test video, audio, touch, and key inputs"
    echo ""
    echo "For troubleshooting, see: docs/MULTIMEDIA_HAL.md"
    exit 0
else
    echo -e "${RED}✗ Some tests failed${NC}"
    echo ""
    echo "Please review the failures above and:"
    echo "  - Install missing dependencies"
    echo "  - Rebuild the project"
    echo "  - Check integration in RealAndroidAutoService"
    exit 1
fi
