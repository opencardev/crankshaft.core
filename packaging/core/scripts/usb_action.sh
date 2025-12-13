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

# USB action script for Crankshaft
# Handles detection and removal of Android Auto devices

set -e

ACTION=$1
MODEL=$2
USB_PATH=$3

# Logging directory and file
LOG_DIR="/var/log/crankshaft"
LOG_FILE="${LOG_DIR}/usb_events.log"
DEVICE_FILE="/var/lib/crankshaft/android_device"

# Ensure log directory exists
mkdir -p "${LOG_DIR}"

# Logging function
log_message() {
    local level=$1
    shift
    local message="$@"
    echo "$(date '+%Y-%m-%d %H:%M:%S') [${level}] ${message}" >> "${LOG_FILE}"
    logger -t crankshaft-usb -p "user.${level}" "${message}"
}

# Handle device addition
handle_add() {
    if [ -z "${USB_PATH}" ]; then
        log_message "warn" "Device add event without USB path"
        return 1
    fi
    
    sleep 1
    
    # Store device information
    mkdir -p "$(dirname "${DEVICE_FILE}")"
    echo "${USB_PATH}" > "${DEVICE_FILE}"
    echo "${MODEL}" >> "${DEVICE_FILE}"
    
    log_message "info" "Android Auto device detected - Model: ${MODEL}, Path: ${USB_PATH}"
    
    # Notify crankshaft-core via D-Bus
    if command -v dbus-send > /dev/null 2>&1; then
        dbus-send --system --type=signal \
            /org/opencardev/Crankshaft \
            org.opencardev.Crankshaft.USBDevice \
            string:"connected" string:"${MODEL}" string:"${USB_PATH}" 2>/dev/null || true
    fi
    
    return 0
}

# Handle device removal
handle_remove() {
    if [ -z "${USB_PATH}" ]; then
        log_message "warn" "Device remove event without USB path"
        return 1
    fi
    
    if [ ! -f "${DEVICE_FILE}" ]; then
        return 0
    fi
    
    # Check if removed device matches stored device
    if grep -q "${USB_PATH}" "${DEVICE_FILE}" 2>/dev/null; then
        log_message "info" "Android Auto device disconnected - Model: ${MODEL}, Path: ${USB_PATH}"
        
        rm -f "${DEVICE_FILE}"
        
        # Notify crankshaft-core via D-Bus
        if command -v dbus-send > /dev/null 2>&1; then
            dbus-send --system --type=signal \
                /org/opencardev/Crankshaft \
                org.opencardev.Crankshaft.USBDevice \
                string:"disconnected" string:"${MODEL}" string:"${USB_PATH}" 2>/dev/null || true
        fi
    fi
    
    return 0
}

# Main execution
case "${ACTION}" in
    add)
        handle_add
        ;;
    remove)
        handle_remove
        ;;
    *)
        log_message "error" "Unknown action: ${ACTION}"
        exit 1
        ;;
esac

exit 0
