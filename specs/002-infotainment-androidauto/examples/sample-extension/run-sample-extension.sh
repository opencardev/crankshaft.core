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
set -u

# Sample Extension Entrypoint
# This script demonstrates a minimal extension that connects to Crankshaft core
# via WebSocket and exposes a simple media source capability.

EXTENSION_ID="com.opencardev.sample"
WEBSOCKET_HOST="${CRANKSHAFT_WEBSOCKET_HOST:-127.0.0.1}"
WEBSOCKET_PORT="${CRANKSHAFT_WEBSOCKET_PORT:-9001}"
WEBSOCKET_URL="ws://${WEBSOCKET_HOST}:${WEBSOCKET_PORT}"

# Logging helper
log() {
    echo "[$(date +'%Y-%m-%d %H:%M:%S')] [${EXTENSION_ID}] $@" >&2
}

cleanup() {
    log "Shutting down gracefully..."
    exit 0
}

trap cleanup SIGTERM SIGINT

log "Starting Sample Extension"
log "Connecting to core at ${WEBSOCKET_URL}"

# Connect to Crankshaft core via WebSocket
# In a real extension, you would use a language-specific WebSocket library
# This example uses Python with websocket-client if available

python3 << 'PYTHON_SCRIPT'
import json
import sys
import time
import os
from datetime import datetime

def log(msg):
    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    print(f"[{timestamp}] [com.opencardev.sample] {msg}", file=sys.stderr)

try:
    import websocket
    
    extension_id = "com.opencardev.sample"
    ws_url = os.environ.get("CRANKSHAFT_WEBSOCKET_URL", "ws://127.0.0.1:9001")
    
    log(f"Attempting to connect to {ws_url}")
    
    ws = websocket.WebSocket()
    try:
        ws.connect(ws_url, timeout=5)
        log("Connected to core")
    except Exception as e:
        log(f"Failed to connect: {e}. Running in offline mode.")
        # Run in offline mode - demonstrate extension lifecycle
        while True:
            time.sleep(1)
    
    # Register as media source extension
    register_msg = {
        "type": "extension.register",
        "extension_id": extension_id,
        "capabilities": ["media.source"],
        "version": "1.0.0"
    }
    
    log(f"Registering extension: {json.dumps(register_msg)}")
    ws.send(json.dumps(register_msg))
    
    # Listen for incoming commands
    log("Listening for commands from core...")
    while True:
        try:
            msg = ws.recv(timeout=1)
            if msg:
                log(f"Received: {msg}")
                data = json.loads(msg)
                
                # Echo back acknowledgment
                ack = {
                    "type": "extension.ack",
                    "message_id": data.get("id"),
                    "status": "ok"
                }
                ws.send(json.dumps(ack))
        except websocket.WebSocketTimeoutException:
            pass
        except Exception as e:
            log(f"Error: {e}")
            break
    
    ws.close()

except ImportError:
    log("websocket-client not available, running in offline mode")
    # Offline mode: just demonstrate extension lifecycle
    import time
    log("Extension initialized successfully")
    log("Extension running. Press Ctrl+C to exit")
    
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        log("Extension shutting down")

PYTHON_SCRIPT

log "Sample Extension stopped"
exit 0
