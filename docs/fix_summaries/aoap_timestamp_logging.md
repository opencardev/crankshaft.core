# AOAP Timestamp Logging Implementation

## Overview
Ported timestamp logging and timeout improvements from `tools/aa_test` standalone tool to the production `RealAndroidAutoService` implementation. This enables better debugging of Android Auto AOAP (Android Open Accessory Protocol) negotiation timing on Raspberry Pi.

## Problem Addressed
Previously, AOAP negotiation logs lacked timestamps, making it difficult to:
- Track precise timing of AOAP device detection and negotiation attempts
- Identify whether the service was hanging or progressing
- Correlate AOAP events with system time for debugging

## Changes Made

### 1. Added Timestamp Helper Functions (`RealAndroidAutoService.cpp`, lines 57-86)

Added three static helper functions after the `#include` directives:

- `getTimestamp()`: Returns current system time with millisecond precision in ISO 8601 format (`YYYY-MM-DDTHH:MM:SS.mmm`)
- `logInfo(const std::string& msg)`: Logs info messages with `[TIMESTAMP] INFO:` prefix
- `logError(const std::string& msg)`: Logs error messages with `[TIMESTAMP] ERROR:` prefix

#### Headers Added
```cpp
#include <chrono>
#include <iomanip>
#include <sstream>
```

### 2. Instrumented `checkForConnectedDevices()` Method

Added timestamp logging to key points in device enumeration and AOAP negotiation:

- Device discovery: `[TIMESTAMP] INFO: [RealAndroidAutoService] Found Google device: vid=0xXXXX pid=0xXXXX`
- Device opened: `[TIMESTAMP] INFO: [RealAndroidAutoService] Opened device for AOAP negotiation`
- Chain creation: `[TIMESTAMP] INFO: [RealAndroidAutoService] Creating AccessoryModeQueryChain...`
- Chain started: `[TIMESTAMP] INFO: [RealAndroidAutoService] AOAP chain started successfully`
- AOAP errors: `[TIMESTAMP] ERROR: [RealAndroidAutoService] AOAP chain error (attempt N): <error details>`
- Timeout handling: `[TIMESTAMP] INFO: [RealAndroidAutoService] AOAP timeout - checking if device re-enumerated...`

### 3. Instrumented `startSearching()` Method

Added timestamp logging to USB search initiation:

- Start of USB search: `[TIMESTAMP] INFO: [RealAndroidAutoService] Starting USB device search`
- Timer activation: `[TIMESTAMP] INFO: Started periodic device detection timer (fallback for hotplug)`
- Search start: `[TIMESTAMP] INFO: Started searching for Android Auto devices`

### 4. Connection Promises

Updated success/error callbacks in USB hub promise handlers to use timestamp logging:

- Device connected: Timestamped log message in device connection handler
- USB hub error: Timestamped error reporting with specific error messages

## Benefits

1. **Precise Timing**: Each log entry now includes millisecond-precision timestamp for exact temporal correlation
2. **Debugging**: Timeline of AOAP negotiation attempts is clearly visible
3. **Timeout Detection**: Easy to spot when the 8-second AOAP timeout occurs
4. **Retry Pattern Analysis**: Can track retry delays and attempt counts with exact timing
5. **Consistency**: Same logging pattern as proven `aa_test` standalone tool

## Example Log Output

```
[2025-01-10T14:32:45.123] INFO: [RealAndroidAutoService] Starting USB device search
[2025-01-10T14:32:46.456] INFO: [RealAndroidAutoService] Found Google device: vid=0x18d1 pid=0x4ee1
[2025-01-10T14:32:46.457] INFO: [RealAndroidAutoService] Opened device for AOAP negotiation
[2025-01-10T14:32:46.458] INFO: [RealAndroidAutoService] Creating AccessoryModeQueryChain...
[2025-01-10T14:32:46.459] INFO: [RealAndroidAutoService] Starting AOAP query chain...
[2025-01-10T14:32:46.460] INFO: [RealAndroidAutoService] AOAP chain started successfully
[2025-01-10T14:32:54.461] INFO: [RealAndroidAutoService] AOAP timeout - checking if device re-enumerated...
[2025-01-10T14:32:55.789] INFO: [RealAndroidAutoService] Found Google device: vid=0x18d1 pid=0x2d00
[2025-01-10T14:32:55.790] INFO: [RealAndroidAutoService] ✓ Device in AOAP mode! Forwarding to USBHub...
```

## Testing

1. Build core with debug symbols: `./scripts/build.sh --component core --build-type Debug`
2. Deploy to Raspberry Pi: `sudo cmake --install build`
3. Run application and observe logs with timestamps
4. Connect Android Auto device and observe AOAP negotiation timeline

## Files Modified

- `core/services/android_auto/RealAndroidAutoService.cpp`: Added helper functions and instrumented methods

## Backward Compatibility

✅ No breaking changes. All new logging is additive and uses existing Logger interface.

## Related Issues

- Original issue: "AA still not loading" (AOAP negotiation failing)
- Related tool: `tools/aa_test/main.cpp` (source of timestamp pattern)
