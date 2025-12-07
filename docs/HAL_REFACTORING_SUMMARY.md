# HAL Refactoring Summary

## Changes Made

### 1. Architecture Refactoring ✅

**Problem Identified**: The original HAL conflated device functionality (GPS, CAN, Camera) with transport mechanisms (UART, USB, SPI). This is architecturally incorrect.

**Solution**: Separated into two distinct layers:
- **Transport Layer**: HOW data moves (UART, USB, SPI, Bluetooth, etc.)
- **Functional Device Layer**: WHAT devices do (GPS, CAN, Camera, etc.)

### 2. New Folder Structure ✅

```
core/hal/
├── transport/              # NEW: Transport layer
│   ├── Transport.h         # Base transport class
│   ├── Transport.cpp
│   ├── UARTTransport.h     # UART/Serial transport
│   └── UARTTransport.cpp
│
├── functional/             # NEW: Functional devices
│   ├── FunctionalDevice.h  # Base functional device
│   ├── FunctionalDevice.cpp
│   ├── GPSDevice.h         # GPS receiver (transport-agnostic)
│   ├── GPSDevice.cpp
│   ├── CANDevice.h         # CAN bus (transport-agnostic)
│   └── CANDevice.cpp
│
└── mocks/
    ├── transport/          # NEW: Mock transports
    │   ├── MockTransport.h
    │   └── MockTransport.cpp
    │
    ├── functional/         # NEW: Mock functional devices
    │
    └── [legacy mocks]      # LEGACY: Old mock structure (to migrate)
```

### 3. Files Created ✅

#### Transport Layer (3 classes)
- `core/hal/transport/Transport.h` - Base transport class
- `core/hal/transport/Transport.cpp`
- `core/hal/transport/UARTTransport.h` - UART transport implementation
- `core/hal/transport/UARTTransport.cpp`

#### Functional Device Layer (3 classes)
- `core/hal/functional/FunctionalDevice.h` - Base functional device
- `core/hal/functional/FunctionalDevice.cpp`
- `core/hal/functional/GPSDevice.h` - GPS device implementation
- `core/hal/functional/GPSDevice.cpp`
- `core/hal/functional/CANDevice.h` - CAN device implementation
- `core/hal/functional/CANDevice.cpp`

#### Mock Layer (1 class)
- `core/hal/mocks/transport/MockTransport.h` - Mock transport for testing
- `core/hal/mocks/transport/MockTransport.cpp`

#### Documentation (2 files)
- `docs/HAL_LAYERED_ARCHITECTURE.md` - Comprehensive architecture guide
- `docs/HAL_REFACTORING_SUMMARY.md` - This summary

#### Scripts (1 file)
- `scripts/refactor_hal_to_layered.sh` - Refactoring automation script

**Total**: 15 new files created

### 4. CMakeLists.txt Updated ✅

Updated to include:
- Transport layer sources and headers
- Functional device sources and headers
- Mock transport sources and headers
- Clear separation with comments
- Legacy files marked for future migration

## Architecture Benefits

### Before (Wrong)
```cpp
class MockGPSDevice : public DeviceHAL {
    // Mixed: GPS functionality + mock transport
};
```

Problem: Can't have GPS over USB, Bluetooth, or different transports.

### After (Correct)
```cpp
// GPS functionality (WHAT)
class GPSDevice : public FunctionalDevice {
    Transport* m_transport;  // Uses ANY transport
};

// Transport layer (HOW)
class UARTTransport : public Transport { };
class BluetoothTransport : public Transport { };

// Composition
auto uart = new UARTTransport("/dev/ttyUSB0");
auto gps = new GPSDevice(uart);  // GPS over UART

auto bt = new BluetoothTransport("00:11:22:33:44:55");
auto gps2 = new GPSDevice(bt);  // Same GPS, different transport!
```

## Real-World Examples

### GPS Device
```cpp
// Traditional GPS receiver (UART)
auto uart = new UARTTransport("/dev/ttyUSB0");
auto gps1 = new GPSDevice(uart);

// Bluetooth GPS receiver
auto bt = new BluetoothTransport("00:11:22:33:44:55");
auto gps2 = new GPSDevice(bt);

// USB GPS dongle
auto usb = new USBTransport("/dev/ttyACM0");
auto gps3 = new GPSDevice(usb);

// Mock GPS for testing
auto mock = new MockTransport("GPS");
mock->setAutoInject(true, 1000);
auto gps4 = new GPSDevice(mock);
```

All four have **identical GPS API**, only transport differs!

### CAN Device
```cpp
// USB CAN adapter (Lawicel CANUSB, Peak PCAN)
auto usb = new USBTransport("/dev/ttyUSB0");
auto can1 = new CANDevice(usb);

// SPI CAN controller (MCP2515)
auto spi = new SPITransport(0, 0);
auto can2 = new CANDevice(spi);

// Native CAN (SocketCAN)
auto native = new CANTransport("can0");
auto can3 = new CANDevice(native);
```

Same CAN API, different hardware!

## Testing Improvements

### Mock Transport Benefits

```cpp
// Create mock transport
auto mock = new MockTransport("Test GPS");
mock->open();

// Inject test NMEA data
mock->injectData("$GPGGA,123519,5140.1234,N,00009.5678,W,1,08,0.9,100.0,M,47.0,M,,*47\r\n");

// Create GPS with mock transport
auto gps = new GPSDevice(mock);
gps->initialize();

// Verify GPS parses data correctly
auto location = gps->getCurrentLocation();
REQUIRE(location.satellites == 8);

// Verify GPS sends commands
gps->sendCommand("CONFIG", {});
QByteArray written = mock->getWrittenData();
// Assert written data is correct
```

No hardware needed for testing!

## Industry Alignment

This architecture matches:

### Linux Kernel
- **Transport**: TTY subsystem, USB stack, SPI framework
- **Functional**: gpsd (GPS), SocketCAN (CAN), V4L2 (camera)

### Android HAL
- **Transport**: Kernel drivers, Binder IPC
- **Functional**: GNSS HAL, Camera HAL, Sensors HAL

### Automotive (GENIVI/COVESA)
- **Transport**: CAN, LIN, Ethernet, SOME/IP
- **Functional**: Vehicle Signal Specification (VSS)

## Migration Plan

### Phase 1: Foundation ✅ COMPLETE
- Created base Transport class
- Created base FunctionalDevice class
- Created example implementations (UART, GPS, CAN)
- Created MockTransport for testing
- Updated CMakeLists.txt
- Created comprehensive documentation

### Phase 2: Expand Transport Layer 🔄 NEXT
- [ ] Create USBTransport
- [ ] Create SPITransport
- [ ] Create I2CTransport
- [ ] Create BluetoothTransport
- [ ] Create WiFiTransport
- [ ] Create CANTransport (native SocketCAN)

### Phase 3: Expand Functional Devices 🔄 NEXT
- [ ] Migrate existing devices to new architecture
  - [ ] CameraDevice
  - [ ] IMUDevice
  - [ ] LIDARDevice
  - [ ] RADARDevice
  - [ ] DisplayDevice
  - [ ] etc.

### Phase 4: Update ProfileManager 🔄 FUTURE
- [ ] Add transport configuration to profiles
- [ ] Support device + transport combinations
- [ ] Migration tool for old profiles

### Phase 5: Deprecate Legacy 🔄 FUTURE
- [ ] Mark old device classes as deprecated
- [ ] Migrate all usage to new architecture
- [ ] Remove legacy classes

## Breaking Changes

### For Developers

**Old way**:
```cpp
auto gps = new MockGPSDevice();
gps->initialize();
```

**New way**:
```cpp
auto transport = new MockTransport("GPS");
auto gps = new GPSDevice(transport);
gps->initialize();
```

### For Configuration

**Old profile**:
```json
{
  "devices": [
    {"type": "GPS", "mock": true}
  ]
}
```

**New profile**:
```json
{
  "devices": [
    {
      "type": "GPS",
      "transport": {
        "type": "UART",
        "config": {"port": "/dev/ttyUSB0", "baudRate": 9600}
      }
    }
  ]
}
```

## Build Status

✅ CMakeLists.txt updated
✅ New files added to build
⚠️ Legacy files still present (for backward compatibility)
🔄 Build verification needed

## Testing

### Unit Tests Needed

1. **Transport Layer Tests**
   - MockTransport injection/capture
   - UART configuration
   - Connection lifecycle

2. **Functional Device Tests**
   - GPS NMEA parsing
   - CAN frame encoding/decoding
   - Error handling

3. **Integration Tests**
   - GPS + MockTransport
   - CAN + MockTransport
   - Multiple devices, multiple transports

## Documentation

✅ `docs/HAL_LAYERED_ARCHITECTURE.md` - Complete architecture guide
✅ `docs/HAL_REFACTORING_SUMMARY.md` - This summary
📋 TODO: Update main README.md with new architecture
📋 TODO: Add migration guide for existing code

## Next Steps

1. **Build and Test** ✅ IMMEDIATE
   ```bash
   cd /mnt/c/Users/matth/install/repos/opencardev/oct_2025/crankshaft-mvp
   ./scripts/refactor_hal_to_layered.sh
   cmake -S . -B build
   cmake --build build
   ```

2. **Create Additional Transports** 🔄 HIGH PRIORITY
   - USB, SPI, I2C, Bluetooth, WiFi

3. **Migrate Existing Devices** 🔄 HIGH PRIORITY
   - Convert Camera, IMU, etc. to new architecture

4. **Update Tests** 🔄 MEDIUM PRIORITY
   - Create transport unit tests
   - Create functional device tests

5. **Update ProfileManager** 🔄 MEDIUM PRIORITY
   - Support transport configuration
   - Device + transport instantiation

## Questions & Answers

**Q: Why separate transport and functional device?**
A: Real hardware works this way. GPS can use UART, USB, or Bluetooth. CAN can use USB adapter, SPI controller, or native interface. Separation provides flexibility and matches reality.

**Q: What about devices that don't need complex transport?**
A: They still use transport layer, but it might be simple (e.g., GPIOTransport for GPIO pins). The pattern remains consistent.

**Q: How do I test without hardware?**
A: Use MockTransport! Inject test data, verify device behavior, no hardware needed.

**Q: Is this overkill for simple devices?**
A: No. The pattern scales from simple (GPIO) to complex (Camera). Consistency is valuable.

**Q: When to use which layer?**
A: **Transport**: "How do I read/write bytes?" **Functional**: "How do I interpret those bytes as GPS/CAN/Camera data?"

## References

- Linux Kernel Driver Model
- Android Hardware Abstraction Layer (HAL)
- GENIVI/COVESA Vehicle Signal Specification
- Automotive Ethernet (SOME/IP)
- SocketCAN documentation
