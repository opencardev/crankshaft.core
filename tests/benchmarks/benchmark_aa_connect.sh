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

# Android Auto connection performance benchmark for Crankshaft
# Target: ≤15 seconds from device detection to AA projection surface visible

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"

# Configuration
TARGET_AA_CONNECT_MS=15000
CORE_BINARY="${BUILD_DIR}/core/crankshaft-core"
WS_PORT=9002
ITERATIONS="${1:-3}"

# Colours for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}═══════════════════════════════════════════════════════════${NC}"
echo -e "${BLUE}   Crankshaft Android Auto Connection Benchmark${NC}"
echo -e "${BLUE}═══════════════════════════════════════════════════════════${NC}"
echo ""
echo -e "Target:      ${YELLOW}≤15.0 seconds${NC}"
echo -e "Iterations:  ${ITERATIONS}"
echo ""

# Check binaries exist
if [[ ! -f "$CORE_BINARY" ]]; then
    echo -e "${RED}Error: Core binary not found: $CORE_BINARY${NC}"
    echo "Build the project first: cmake --build build"
    exit 1
fi

# Helper function to monitor WebSocket events
wait_for_aa_event() {
    local ws_port=$1
    local timeout=$2
    local event_received=0
    
    # Create a temporary file for WebSocket monitoring
    local temp_log=$(mktemp)
    trap "rm -f $temp_log" RETURN
    
    # Use nc (netcat) to connect and listen for AA events
    # In practice, this would need a proper WebSocket client library
    # For now, we simulate by checking the core logs for AA session event
    
    local elapsed=0
    while ((elapsed < timeout)); do
        # Check if core has logged AA session state change (event published)
        if grep -q "android-auto/status/connected\|SessionState.*ACTIVE" /tmp/crankshaft-core.log 2>/dev/null; then
            event_received=1
            break
        fi
        
        sleep 0.1
        ((elapsed++)) || true
    done
    
    return $((! event_received))
}

# Cleanup function
cleanup() {
    if [[ -n "${CORE_PID:-}" ]] && kill -0 "$CORE_PID" 2>/dev/null; then
        kill "$CORE_PID" 2>/dev/null || true
        wait "$CORE_PID" 2>/dev/null || true
    fi
    rm -f /tmp/crankshaft-core.log
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
    
    # Clear log
    rm -f /tmp/crankshaft-core.log
    
    # Start timer (when core starts listening for devices)
    start_time=$(date +%s%3N)
    
    # Launch core with mock mode for testing
    echo "Starting core daemon..."
    export CRANKSHAFT_WS_PORT=$WS_PORT
    export CRANKSHAFT_AA_MOCK=true  # Use mock AA service for testing
    
    "$CORE_BINARY" --ws-port "$WS_PORT" --aa-mock 2>&1 | tee /tmp/crankshaft-core.log >/dev/null 2>&1 &
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
    
    # Simulate AA device connection by sending device detection event
    # In real scenario, this would be USB device hotplug or mDNS discovery
    echo "Simulating AA device connection..."
    
    # Send mock device connected event via WebSocket or direct API
    # For now, use direct dbus call or HTTP endpoint if available
    # This is a placeholder - actual implementation would use:
    # - DBus method call to ServiceManager
    # - Or HTTP POST to diagnostics endpoint
    # - Or direct socket message to WebSocket
    
    # For testing, we'll simulate by checking for the event in logs
    # The mock AA service will trigger device detection automatically
    
    # Wait for AA session event to appear in logs (simulating event propagation)
    if wait_for_aa_event "$WS_PORT" $((TARGET_AA_CONNECT_MS / 100 + 10)); then
        # Event received - end timer
        end_time=$(date +%s%3N)
        total_connection_ms=$((end_time - start_time))
        
        times+=("$total_connection_ms")
        total_time=$((total_time + total_connection_ms))
        
        # Check against target
        if ((total_connection_ms <= TARGET_AA_CONNECT_MS)); then
            echo -e "${GREEN}✓ AA connect time: ${total_connection_ms}ms (PASS)${NC}"
            ((passed++))
        else
            echo -e "${RED}✗ AA connect time: ${total_connection_ms}ms (FAIL - exceeds ${TARGET_AA_CONNECT_MS}ms target)${NC}"
            ((failed++))
        fi
    else
        echo -e "${RED}✗ AA event not received within timeout${NC}"
        ((failed++))
    fi
    
    # Cleanup before next iteration
    cleanup
    CORE_PID=""
    
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

if ((passed > 0)); then
    avg_time=$((total_time / passed))
    
    # Find min and max from valid results
    if ((${#times[@]} > 0)); then
        min_time=${times[0]}
        max_time=${times[0]}
        for time in "${times[@]}"; do
            ((time < min_time)) && min_time=$time || true
            ((time > max_time)) && max_time=$time || true
        done
        
        echo "Iterations:  $ITERATIONS"
        echo "Valid runs:  $passed"
        echo "Failed runs: $failed"
        echo "Average:     ${avg_time}ms"
        echo "Minimum:     ${min_time}ms"
        echo "Maximum:     ${max_time}ms"
    fi
else
    echo "Iterations:  $ITERATIONS"
    echo "Valid runs:  $passed"
    echo "Failed runs: $failed"
fi

echo ""
echo -e "Target:      ${YELLOW}≤${TARGET_AA_CONNECT_MS}ms${NC}"
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
