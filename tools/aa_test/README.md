# Android Auto AOAP Test Tool

A minimal standalone tool for testing Android Auto AOAP (Android Open Accessory Protocol) negotiation without rebuilding the full Crankshaft application.

## Purpose

This tool:
- Enumerates USB devices and finds Google/Android devices
- Attempts AOAP negotiation (GET_PROTOCOL → SEND_STRING × 6 → START)
- Supports verbose USB logging to debug control transfers
- Provides quick iteration for USB debugging

## Build

From the repository root:

```bash
# Configure (first time or after changes)
cmake -S tools/aa_test -B tools/aa_test/build

# Build
cmake --build tools/aa_test/build

# Or use the provided script
cd tools/aa_test
./build.sh
```

## Usage

### Basic usage
```bash
# From repo root
./tools/aa_test/build/aa_test
```

### With verbose USB logging
```bash
# Via environment variable
AASDK_VERBOSE_USB=1 ./tools/aa_test/build/aa_test

# Via command line
./tools/aa_test/build/aa_test --verbose-usb

# Or short form
./tools/aa_test/build/aa_test -v
```

### Run with sudo (if needed for USB permissions)
```bash
# Preserve environment
sudo -E AASDK_VERBOSE_USB=1 ./tools/aa_test/build/aa_test

# Or pass through sudo
sudo AASDK_VERBOSE_USB=1 ./tools/aa_test/build/aa_test
```

## What to Look For

### Success indicators
- "AOAP chain completed successfully!"
- Device re-enumerates with PID 0x2d00 or 0x2d01
- `lsusb | grep 18d1` shows accessory mode

### Failure indicators
- "AOAP chain failed" with error message
- Device stays at PID 0x4ee1 (MTP mode)
- Timeout messages

### Verbose USB logs (when enabled)
- Control setup parameters (bmRequestType, bRequest, wValue, wIndex, wLength)
- Hex dumps of control transfer buffers
- Transfer submit/completion results
- Libusb error codes

## Debugging Steps

1. **Run with verbose logging and capture output:**
   ```bash
   AASDK_VERBOSE_USB=1 ./tools/aa_test/build/aa_test 2>&1 | tee ~/aa_test.log
   ```

2. **Monitor kernel USB events:**
   ```bash
   sudo dmesg -wH | tee ~/dmesg_usb.log
   ```

3. **Check device enumeration before/after:**
   ```bash
   # Before plugging
   lsusb -v > ~/lsusb_before.txt
   
   # After AOAP attempt
   lsusb -v > ~/lsusb_after.txt
   ```

4. **Capture USB control transfers (advanced):**
   ```bash
   sudo modprobe usbmon
   sudo tshark -i usbmon1 -Y 'usb.transfer_type == 0' -w ~/usbmon_control.pcapng
   ```

## Common Issues

### Device not found
- Ensure phone is plugged in
- Check USB cable supports data (not charge-only)
- Verify phone is not in charging-only mode

### AOAP negotiation fails
- Phone may need to be unlocked
- Trust dialog on phone may need acceptance
- Some OEM ROMs may not support AOAP
- Try different USB ports (USB 2.0 vs 3.0)

### Permission denied
- Add udev rules or run with sudo
- Add user to `plugdev` group: `sudo usermod -a -G plugdev $USER`

## Dependencies

- AASDK (built from `external/aasdk`)
- Boost.Asio
- libusb-1.0
- protobuf
- pthreads

## See Also

- Main AASDK logger documentation: `external/aasdk/MODERN_LOGGER.md`
- Full application: `core/` and `ui/`
