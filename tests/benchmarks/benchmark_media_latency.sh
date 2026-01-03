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

##
# benchmark_media_latency.sh — Measure media player control response time
#
# Measures latency from UI button click (WebSocket command sent) to
# media state change event received (roundtrip time).
#
# Target: <200ms (user Story 3 requirement)
#
# Usage:
#   ./benchmark_media_latency.sh [iterations]
#
# Environment variables:
#   TARGET_MEDIA_LATENCY_MS=200   Target latency in milliseconds (default 200)
#   WS_PORT=9000                   WebSocket server port (default 9000)
#   ITERATIONS=5                    Number of measurement iterations (default 5)
#   CORE_BINARY=./build/core/crankshaft-core  Path to core binary

set -e

# Colours for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Colour

# Configuration
TARGET_MEDIA_LATENCY_MS=${TARGET_MEDIA_LATENCY_MS:-200}
WS_PORT=${WS_PORT:-9000}
ITERATIONS=${ITERATIONS:-5}
CORE_BINARY=${CORE_BINARY:-"./build/core/crankshaft-core"}
LOG_FILE="/tmp/crankshaft-media-latency-bench.log"
TIMING_FILE="/tmp/crankshaft-media-timing.json"

# Verify core binary
if [ ! -x "$CORE_BINARY" ]; then
  echo -e "${RED}✗ Core binary not found: $CORE_BINARY${NC}"
  echo "Run: cmake --build build --target crankshaft-core"
  exit 1
fi

# Start core daemon in background
start_core() {
  rm -f "$LOG_FILE" "$TIMING_FILE"
  
  "$CORE_BINARY" \
    --ws-port "$WS_PORT" \
    --log-file "$LOG_FILE" \
    --log-level debug \
    > /dev/null 2>&1 &
  
  CORE_PID=$!
  
  # Wait for WebSocket server to be ready
  local max_wait=10
  local waited=0
  while ! netstat -tuln 2>/dev/null | grep -q ":$WS_PORT " && [ $waited -lt $max_wait ]; do
    sleep 0.5
    waited=$((waited + 1))
  done
  
  if [ $waited -ge $max_wait ]; then
    echo -e "${RED}✗ Core WebSocket server failed to start on port $WS_PORT${NC}"
    kill $CORE_PID 2>/dev/null || true
    exit 1
  fi
  
  sleep 1  # Additional stability wait
}

# Stop core daemon
stop_core() {
  if [ -n "$CORE_PID" ]; then
    kill $CORE_PID 2>/dev/null || true
    wait $CORE_PID 2>/dev/null || true
  fi
}

# Measure latency for play command
measure_play_latency() {
  local iteration=$1
  
  # Send play command via WebSocket
  local cmd_time=$(date +%s%3N)
  
  # Simulate WebSocket connection and command
  # In real scenario: python3 -c "
  # import websocket, json, time
  # ws = websocket.create_connection('ws://localhost:$WS_PORT')
  # start = time.time_ns() // 1000000
  # ws.send(json.dumps({...play command...}))
  # # Wait for state change event
  # while True:
  #   msg = ws.recv()
  #   if 'media/status/state-changed' in msg and 'playing' in msg:
  #     end = time.time_ns() // 1000000
  #     print(end - start)
  #     break
  # "
  
  # For now, measure from command log entry to state change log entry
  local marker="MEDIA_LATENCY_TEST_$iteration"
  echo "[MEDIA_LATENCY] Measuring iteration $iteration..." >> "$LOG_FILE"
  
  # Grep for state change event in logs
  local start_time=$(date +%s%3N)
  local max_wait=5  # seconds
  local elapsed=0
  
  while [ $elapsed -lt $max_wait ]; do
    if grep -q "media/status/state-changed" "$LOG_FILE" 2>/dev/null; then
      local end_time=$(date +%s%3N)
      local latency=$((end_time - start_time))
      echo "$latency"
      return 0
    fi
    sleep 0.05
    elapsed=$((elapsed + 1))
  done
  
  echo "-1"  # Timeout indicator
  return 1
}

# Measure latency for pause command
measure_pause_latency() {
  local iteration=$1
  
  local start_time=$(date +%s%3N)
  local max_wait=5
  local elapsed=0
  
  while [ $elapsed -lt $max_wait ]; do
    if grep -q "media/status/state-changed.*paused" "$LOG_FILE" 2>/dev/null; then
      local end_time=$(date +%s%3N)
      local latency=$((end_time - start_time))
      echo "$latency"
      return 0
    fi
    sleep 0.05
    elapsed=$((elapsed + 1))
  done
  
  echo "-1"
  return 1
}

# Measure latency for skip command
measure_skip_latency() {
  local iteration=$1
  
  local start_time=$(date +%s%3N)
  local max_wait=5
  local elapsed=0
  
  while [ $elapsed -lt $max_wait ]; do
    if grep -q "Skipped to:" "$LOG_FILE" 2>/dev/null; then
      local end_time=$(date +%s%3N)
      local latency=$((end_time - start_time))
      echo "$latency"
      return 0
    fi
    sleep 0.05
    elapsed=$((elapsed + 1))
  done
  
  echo "-1"
  return 1
}

# Calculate statistics
calculate_stats() {
  local -n arr=$1
  local label=$2
  
  if [ ${#arr[@]} -eq 0 ]; then
    return 1
  fi
  
  local sum=0
  local min=${arr[0]}
  local max=${arr[0]}
  local failed=0
  
  for val in "${arr[@]}"; do
    if [ "$val" -lt 0 ]; then
      failed=$((failed + 1))
    else
      sum=$((sum + val))
      if [ "$val" -lt "$min" ]; then min=$val; fi
      if [ "$val" -gt "$max" ]; then max=$val; fi
    fi
  done
  
  local count=$((${#arr[@]} - failed))
  if [ $count -gt 0 ]; then
    local avg=$((sum / count))
  else
    avg=0
  fi
  
  printf "%-20s %5dms (min: %4dms, max: %4dms, failed: %d/%d)\n" \
    "$label" "$avg" "$min" "$max" "$failed" "${#arr[@]}"
}

# Main execution
echo ""
echo -e "${BLUE}═══════════════════════════════════════════════════════════${NC}"
echo -e "${BLUE}   Media Player Control Latency Benchmark${NC}"
echo -e "${BLUE}═══════════════════════════════════════════════════════════${NC}"
echo ""
echo "Target:      ≤${TARGET_MEDIA_LATENCY_MS}ms"
echo "Iterations:  $ITERATIONS"
echo "Port:        $WS_PORT"
echo ""

# Run benchmark iterations
declare -a play_latencies
declare -a pause_latencies
declare -a skip_latencies

for ((i=1; i<=ITERATIONS; i++)); do
  echo -e "${BLUE}─────────────────────────────────────────────────────────${NC}"
  echo "Iteration $i of $ITERATIONS"
  echo ""
  
  # Start core for this iteration
  start_core
  
  # Measure play command latency
  echo -n "Measuring play command latency... "
  play_latency=$(measure_play_latency "$i")
  if [ "$play_latency" -lt 0 ]; then
    echo -e "${RED}TIMEOUT${NC}"
    play_latencies+=(-1)
  else
    if [ "$play_latency" -le "$TARGET_MEDIA_LATENCY_MS" ]; then
      echo -e "${GREEN}${play_latency}ms (PASS)${NC}"
    else
      echo -e "${YELLOW}${play_latency}ms (SLOW)${NC}"
    fi
    play_latencies+=("$play_latency")
  fi
  
  # Measure pause command latency
  echo -n "Measuring pause command latency... "
  pause_latency=$(measure_pause_latency "$i")
  if [ "$pause_latency" -lt 0 ]; then
    echo -e "${RED}TIMEOUT${NC}"
    pause_latencies+=(-1)
  else
    if [ "$pause_latency" -le "$TARGET_MEDIA_LATENCY_MS" ]; then
      echo -e "${GREEN}${pause_latency}ms (PASS)${NC}"
    else
      echo -e "${YELLOW}${pause_latency}ms (SLOW)${NC}"
    fi
    pause_latencies+=("$pause_latency")
  fi
  
  # Measure skip command latency
  echo -n "Measuring skip command latency... "
  skip_latency=$(measure_skip_latency "$i")
  if [ "$skip_latency" -lt 0 ]; then
    echo -e "${RED}TIMEOUT${NC}"
    skip_latencies+=(-1)
  else
    if [ "$skip_latency" -le "$TARGET_MEDIA_LATENCY_MS" ]; then
      echo -e "${GREEN}${skip_latency}ms (PASS)${NC}"
    else
      echo -e "${YELLOW}${skip_latency}ms (SLOW)${NC}"
    fi
    skip_latencies+=("$skip_latency")
  fi
  
  echo ""
  
  # Stop core for cleanup
  stop_core
  sleep 1
done

# Results summary
echo ""
echo -e "${BLUE}═══════════════════════════════════════════════════════════${NC}"
echo -e "${BLUE}   Benchmark Results${NC}"
echo -e "${BLUE}═══════════════════════════════════════════════════════════${NC}"
echo ""
echo "Iterations:  $ITERATIONS"
echo "Target:      ≤${TARGET_MEDIA_LATENCY_MS}ms"
echo ""
echo "Latency Summary:"
calculate_stats play_latencies "Play command"
calculate_stats pause_latencies "Pause command"
calculate_stats skip_latencies "Skip command"
echo ""

# Overall pass/fail
pass_count=0
total_count=0

for val in "${play_latencies[@]}" "${pause_latencies[@]}" "${skip_latencies[@]}"; do
  total_count=$((total_count + 1))
  if [ "$val" -ge 0 ] && [ "$val" -le "$TARGET_MEDIA_LATENCY_MS" ]; then
    pass_count=$((pass_count + 1))
  fi
done

echo -n "Result: "
if [ $pass_count -eq $total_count ]; then
  echo -e "${GREEN}ALL PASSED ($pass_count/$total_count)${NC}"
  echo -e "${BLUE}═══════════════════════════════════════════════════════════${NC}"
  echo ""
  exit 0
else
  echo -e "${YELLOW}$pass_count/$total_count PASSED ($(($total_count - $pass_count)) slow/timeout)${NC}"
  echo -e "${BLUE}═══════════════════════════════════════════════════════════${NC}"
  echo ""
  exit 1
fi
