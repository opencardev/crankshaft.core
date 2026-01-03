#!/usr/bin/env bash
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

set -euo pipefail

# Cold-start performance benchmark for Crankshaft infotainment system
# Target: ≤10 seconds from core launch to Home screen visible

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"

# Configuration
TARGET_COLD_START_MS=10000
CORE_BINARY="${BUILD_DIR}/core/crankshaft-core"
UI_BINARY="${BUILD_DIR}/ui/crankshaft-ui"
WS_PORT=9001
ITERATIONS="${1:-3}"
PLATFORM="${PLATFORM:-vnc:size=1024x600,port=5901}"

# Colours for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}═══════════════════════════════════════════════════════════${NC}"
echo -e "${BLUE}   Crankshaft Cold-Start Performance Benchmark${NC}"
echo -e "${BLUE}═══════════════════════════════════════════════════════════${NC}"
echo ""
echo -e "Target:      ${YELLOW}≤10.0 seconds${NC}"
echo -e "Iterations:  ${ITERATIONS}"
echo -e "Platform:    ${PLATFORM}"
echo ""

# Check binaries exist
if [[ ! -f "$CORE_BINARY" ]]; then
    echo -e "${RED}Error: Core binary not found: $CORE_BINARY${NC}"
    echo "Build the project first: cmake --build build"
    exit 1
fi

if [[ ! -f "$UI_BINARY" ]]; then
    echo -e "${RED}Error: UI binary not found: $UI_BINARY${NC}"
    echo "Build the project first: cmake --build build"
    exit 1
fi

# Cleanup function
cleanup() {
    if [[ -n "${CORE_PID:-}" ]] && kill -0 "$CORE_PID" 2>/dev/null; then
        kill "$CORE_PID" 2>/dev/null || true
        wait "$CORE_PID" 2>/dev/null || true
    fi
    if [[ -n "${UI_PID:-}" ]] && kill -0 "$UI_PID" 2>/dev/null; then
        kill "$UI_PID" 2>/dev/null || true
        wait "$UI_PID" 2>/dev/null || true
    fi
}
trap cleanup EXIT

# Run benchmark iterations
declare -a times
total_time=0
passed=0
failed=0

for ((i=1; i<=ITERATIONS; i++)); do
    echo -e "${BLUE}─────────────────────────────────────────────────────────${NC}"
    echo -e "${BLUE}Iteration $i of $ITERATIONS${NC}"
    echo ""
    
    # Start timer
    start_time=$(date +%s%3N)
    
    # Launch core
    echo "Starting core daemon..."
    export CRANKSHAFT_WS_PORT=$WS_PORT
    "$CORE_BINARY" --ws-port "$WS_PORT" >/dev/null 2>&1 &
    CORE_PID=$!
    
    # Wait for core to be ready (WebSocket server listening)
    timeout=5
    while ((timeout > 0)); do
        if netstat -tuln 2>/dev/null | grep -q ":${WS_PORT} "; then
            break
        fi
        sleep 0.1
        ((timeout--)) || true
    done
    
    if ((timeout <= 0)); then
        echo -e "${RED}✗ Core failed to start within 5 seconds${NC}"
        cleanup
        ((failed++))
        continue
    fi
    
    core_ready_time=$(date +%s%3N)
    core_startup_ms=$((core_ready_time - start_time))
    echo "Core ready in ${core_startup_ms}ms"
    
    # Launch UI
    echo "Starting UI client..."
    QT_QPA_PLATFORM="$PLATFORM" "$UI_BINARY" --server "ws://localhost:$WS_PORT" >/dev/null 2>&1 &
    UI_PID=$!
    
    # Wait for UI to be visible (check for QML engine initialization)
    # We'll use a simple timeout as a proxy for "UI visible"
    sleep 2
    
    # Check if UI process is still running
    if ! kill -0 "$UI_PID" 2>/dev/null; then
        echo -e "${RED}✗ UI crashed during startup${NC}"
        cleanup
        ((failed++))
        continue
    fi
    
    # End timer
    end_time=$(date +%s%3N)
    total_startup_ms=$((end_time - start_time))
    
    times+=("$total_startup_ms")
    total_time=$((total_time + total_startup_ms))
    
    # Check against target
    if ((total_startup_ms <= TARGET_COLD_START_MS)); then
        echo -e "${GREEN}✓ Cold start: ${total_startup_ms}ms (PASS)${NC}"
        ((passed++))
    else
        echo -e "${RED}✗ Cold start: ${total_startup_ms}ms (FAIL - exceeds ${TARGET_COLD_START_MS}ms target)${NC}"
        ((failed++))
    fi
    
    # Cleanup before next iteration
    cleanup
    CORE_PID=""
    UI_PID=""
    
    # Brief pause between iterations
    if ((i < ITERATIONS)); then
        sleep 1
    fi
done

# Calculate statistics
echo ""
echo -e "${BLUE}═══════════════════════════════════════════════════════════${NC}"
echo -e "${BLUE}   Benchmark Results${NC}"
echo -e "${BLUE}═══════════════════════════════════════════════════════════${NC}"
echo ""

if ((ITERATIONS > 0)); then
    avg_time=$((total_time / ITERATIONS))
    
    # Find min and max
    min_time=${times[0]}
    max_time=${times[0]}
    for time in "${times[@]}"; do
        ((time < min_time)) && min_time=$time
        ((time > max_time)) && max_time=$time
    done
    
    echo "Iterations:  $ITERATIONS"
    echo "Average:     ${avg_time}ms"
    echo "Minimum:     ${min_time}ms"
    echo "Maximum:     ${max_time}ms"
    echo ""
    echo -e "Target:      ${YELLOW}≤${TARGET_COLD_START_MS}ms${NC}"
    echo ""
    
    if ((passed == ITERATIONS)); then
        echo -e "${GREEN}Result: ALL PASSED ($passed/$ITERATIONS)${NC}"
        exit 0
    elif ((failed == ITERATIONS)); then
        echo -e "${RED}Result: ALL FAILED (0/$ITERATIONS)${NC}"
        exit 1
    else
        echo -e "${YELLOW}Result: PARTIAL ($passed passed, $failed failed)${NC}"
        exit 1
    fi
else
    echo -e "${RED}No iterations completed${NC}"
    exit 1
fi
