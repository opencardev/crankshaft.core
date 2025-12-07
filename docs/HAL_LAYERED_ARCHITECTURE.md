# HAL Layered Architecture

## Overview

The HAL (Hardware Abstraction Layer) has been refactored to properly separate **functional device capabilities** from **transport/communication mechanisms**. This follows industry-standard patterns used in Linux kernel, Android HAL, and automotive systems.

## The Problem with Previous Design

The original design conflated "what a device does" (GPS, CAN, Camera) with "how it communicates" (UART, USB, SPI). This created issues:

- **GPS via USB-to-Serial**: Was it a GPS device or a UART device?
- **USB CAN adapter**: Was it CAN or USB?
- **SPI CAN controller**: Was it CAN or SPI?
- **Bluetooth GPS receiver**: Was it GPS or Bluetooth?

Answer: **Both!** A device has a **function** (what it does) and a **transport** (how it communicates).

## New Architecture

```
┌─────────────────────────────────────────────────────────┐
│          Application Layer (Crankshaft UI)              │
│          - Uses functional device interfaces            │
└─────────────────────────────────────────────────────────┘
                         ▼
┌─────────────────────────────────────────────────────────┐
│      Functional Device Layer (WHAT devices do)          │
│      core/hal/functional/                               │
│                                                          │
│      GPSDevice      - Provides location data            │
│      CANDevice      - Provides vehicle bus data         │
│      CameraDevice   - Provides video frames             │
│      IMUDevice      - Provides motion data              │
│      etc.                                               │
└─────────────────────────────────────────────────────────┘
                         ▼ uses
┌─────────────────────────────────────────────────────────┐
│      Transport Layer (HOW data moves)                   │
│      core/hal/transport/                                │
│                                                          │
│      UARTTransport      - Serial communication          │
│      USBTransport       - USB communication             │
│      SPITransport       - SPI bus                       │
│      I2CTransport       - I2C bus                       │
│      BluetoothTransport - Bluetooth                     │
│      CANTransport       - Native CAN (SocketCAN)        │
│      etc.                                               │
└─────────────────────────────────────────────────────────┘
                         ▼
┌─────────────────────────────────────────────────────────┐
│         Hardware/Driver Layer                           │
│         - Platform-specific implementations             │
│         - Linux: /dev/ttyUSB0, SocketCAN, etc.          │
└─────────────────────────────────────────────────────────┘
```

## Folder Structure

```
core/hal/
├── transport/              # Transport layer implementations
│   ├── Transport.h         # Base transport class
│   ├── Transport.cpp
│   ├── UARTTransport.h     # Serial/UART transport
│   ├── UARTTransport.cpp
│   ├── USBTransport.h      # USB transport
│   ├── USBTransport.cpp
│   ├── SPITransport.h      # SPI bus transport
│   ├── SPITransport.cpp
│   ├── I2CTransport.h      # I2C bus transport
│   ├── I2CTransport.cpp
│   └── ...
│
├── functional/             # Functional device implementations
│   ├── FunctionalDevice.h  # Base functional device class
│   ├── FunctionalDevice.cpp
│   ├── GPSDevice.h         # GPS/GNSS receiver
│   ├── GPSDevice.cpp
│   ├── CANDevice.h         # CAN bus interface
│   ├── CANDevice.cpp
│   ├── CameraDevice.h      # Camera device
│   ├── CameraDevice.cpp
│   └── ...
│
└── mocks/                  # Mock implementations for testing
    ├── transport/          # Mock transport implementations
    │   ├── MockTransport.h
    │   ├── MockTransport.cpp
    │   └── ...
    │
    └── functional/         # Mock functional devices (if needed)
        └── ...
```

## Key Concepts

### 1. Transport Layer

**Purpose**: Handle the physical/logical communication channel.

**Responsibilities**:
- Open/close connection
- Read/write raw bytes
- Manage connection state
- Handle transport-specific configuration (baud rate, SPI mode, etc.)

**Independent of**: What data means (GPS sentences, CAN frames, etc.)

### 2. Functional Device Layer

**Purpose**: Provide device-specific functionality.

**Responsibilities**:
- Parse/encode device-specific protocols
- Maintain device state
- Emit device-specific signals
- Expose device-specific APIs

**Independent of**: How data is transported (UART, USB, etc.)

### 3. Composition Pattern

Functional devices **use** (not inherit from) transports:

```cpp
class GPSDevice : public FunctionalDevice {
    Transport* m_transport;  // Uses ANY transport
    
    void parseData() {
        QByteArray data = m_transport->read();
        // Parse NMEA sentences regardless of transport
    }
};
```

## Examples

### GPS Device Over Different Transports

```cpp
// GPS over UART (traditional GPS receiver)
auto uartTransport = new UARTTransport("/dev/ttyUSB0");
uartTransport->configure("baudRate", 9600);
auto gps1 = new GPSDevice(uartTransport);

// GPS over Bluetooth (Bluetooth GPS receiver)
auto btTransport = new BluetoothTransport("00:11:22:33:44:55");
auto gps2 = new GPSDevice(btTransport);

// GPS over USB (USB GPS dongle)
auto usbTransport = new USBTransport("/dev/ttyACM0");
auto gps3 = new GPSDevice(usbTransport);

// GPS over Mock (for testing)
auto mockTransport = new MockTransport("GPS Mock");
mockTransport->setAutoInject(true, 1000);  // Inject test data
auto gps4 = new GPSDevice(mockTransport);
```

All four GPS devices have **identical API** and functionality, only the transport differs!

### CAN Device Over Different Transports

```cpp
// USB CAN adapter (e.g., Lawicel CANUSB, Peak PCAN-USB)
auto usbTransport = new USBTransport("/dev/ttyUSB0");
usbTransport->configure("protocol", "slcan");  // SLCAN protocol
auto can1 = new CANDevice(usbTransport);

// SPI CAN controller (e.g., MCP2515)
auto spiTransport = new SPITransport(0, 0);  // SPI bus 0, CS 0
spiTransport->configure("clockSpeed", 10000000);  // 10 MHz
auto can2 = new CANDevice(spiTransport);

// Native CAN interface (SocketCAN on Linux)
auto canTransport = new CANTransport("can0");
auto can3 = new CANDevice(canTransport);

// Mock CAN (for testing)
auto mockTransport = new MockTransport("CAN Mock");
auto can4 = new CANDevice(mockTransport);
```

Same CAN device API, different transport mechanisms!

### Camera Device Over Different Transports

```cpp
// USB camera (UVC)
auto usbTransport = new USBTransport("/dev/video0");
auto camera1 = new CameraDevice(usbTransport);

// CSI camera (Raspberry Pi Camera)
auto csiTransport = new CSITransport();
auto camera2 = new CameraDevice(csiTransport);

// Network camera (RTSP/HTTP)
auto networkTransport = new NetworkTransport("rtsp://192.168.1.100/stream");
auto camera3 = new CameraDevice(networkTransport);
```

## Benefits

### 1. **Flexibility**
- Same functional device can use different transports
- Easy to support new hardware without changing device logic

### 2. **Testability**
- Mock transports for unit testing
- Inject test data without hardware
- Test error conditions easily

### 3. **Reusability**
- Transport implementations shared across devices
- UART transport used by GPS, debug console, CAN adapter, etc.

### 4. **Maintainability**
- Changes to transport don't affect functional devices
- Changes to device protocol don't affect transport

### 5. **Real-World Alignment**
- Matches how actual hardware works
- Matches Linux kernel architecture (SocketCAN, V4L2, etc.)
- Matches Android HAL patterns

## Migration from Old Design

### Old Design (Wrong)

```cpp
class MockGPSDevice : public DeviceHAL {
    // GPS functionality mixed with mock transport
};
```

### New Design (Correct)

```cpp
// Separate transport and functional device
auto transport = new MockTransport("GPS");
auto gps = new GPSDevice(transport);
```

## Profile Configuration

ProfileManager now supports transport configuration:

```json
{
  "devices": [
    {
      "id": "gps1",
      "type": "GPS",
      "transport": {
        "type": "UART",
        "config": {
          "port": "/dev/ttyUSB0",
          "baudRate": 9600
        }
      }
    },
    {
      "id": "can1",
      "type": "CAN",
      "transport": {
        "type": "USB",
        "config": {
          "port": "/dev/ttyUSB1",
          "protocol": "slcan"
        }
      }
    }
  ]
}
```

## Adding New Devices

### Adding a New Transport

1. Create `MyTransport.h` and `MyTransport.cpp` in `core/hal/transport/`
2. Inherit from `Transport`
3. Implement virtual methods
4. Add to CMakeLists.txt

### Adding a New Functional Device

1. Create `MyDevice.h` and `MyDevice.cpp` in `core/hal/functional/`
2. Inherit from `FunctionalDevice`
3. Accept `Transport*` in constructor
4. Use `m_transport->read()` / `m_transport->write()` for I/O
5. Parse/encode device-specific protocol
6. Add to CMakeLists.txt

### Creating Mock Transport

For testing, use `MockTransport`:

```cpp
auto mock = new MockTransport("Test");
mock->open();

// Inject test data
mock->injectData("$GPGGA,123519,5140.1234,N,00009.5678,W,1,08,0.9,100.0,M,47.0,M,,*47\r\n");

// Verify written data
auto gps = new GPSDevice(mock);
gps->sendCommand("CONFIG", {});
QByteArray written = mock->getWrittenData();
```

## Industry Comparisons

### Linux Kernel

- **Transport Layer**: TTY subsystem, USB stack, SPI framework, I2C subsystem
- **Functional Layer**: GPS daemon (gpsd), CAN subsystem (SocketCAN), V4L2 (cameras)
- **Separation**: `/dev/ttyUSB0` (transport) → `gpsd` (functional GPS)

### Android HAL

- **Transport Layer**: Kernel drivers, Binder IPC
- **Functional Layer**: GNSS HAL, Camera HAL, Sensors HAL
- **Separation**: Hardware module provides function, uses any transport

### Automotive (GENIVI/COVESA)

- **Transport Layer**: CAN, LIN, Ethernet, SOME/IP
- **Functional Layer**: Vehicle Signal Specification (VSS)
- **Separation**: Signal routing configuration maps functions to transports

## Summary

The refactored HAL properly separates:
- **WHAT** devices do (functional layer)
- **HOW** they communicate (transport layer)

This matches real-world hardware architecture, improves testability, and provides flexibility for different hardware configurations.
