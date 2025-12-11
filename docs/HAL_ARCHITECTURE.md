# Hardware Abstraction Layer (HAL) Architecture

## Overview

The Crankshaft HAL system provides a flexible, production-inspired architecture for vehicle, host, and device hardware abstraction. Inspired by Android Automotive's HAL design patterns, the three-tier abstraction allows seamless integration of mock devices for development and testing while remaining open for real hardware implementations.

### Design Principles

- **Three-Tier Abstraction**: Separation of vehicle hardware, host/system hardware, and device interfaces
- **Mock-First Development**: Comprehensive mock implementations with realistic simulated data for development without hardware
- **Easy Hardware Extension**: Clear base class interfaces requiring only 5-6 method overrides for new hardware implementations
- **Thread-Safe**: All HALs use QMutex protection for concurrent property access
- **Event-Driven**: Signal-based architecture integrates seamlessly with the existing EventBus and WebSocket server
- **Extensible Device Registry**: Multiple devices of the same type supported with unique identification

## Three-Tier Architecture

### 1. Vehicle HAL (`VehicleHAL`)

Abstract base class representing the vehicle's hardware state and control.

**Responsibilities:**
- Manage vehicle properties (speed, fuel, temperature, gears, lights, climate, etc.)
- Handle vehicle property subscriptions and change notifications
- Provide read/write access to vehicle state

**Key Properties (60+ types):**

| Category | Properties |
|----------|-----------|
| **Engine** | speed, rpm, coolant temperature, oil temperature, oil pressure, throttle position, idle speed |
| **Drivetrain** | gear status, parking brake, ABS status, stability control, transmission type |
| **Doors/Windows** | lock status (all doors), window position (all windows), sunroof position |
| **Lights** | headlights on/off, high beams on/off, fog lights on/off, brake lights on/off, turn signals |
| **Climate** | AC on/off, HVAC mode, target temperature, actual temperature, air recirculation |
| **Fuel** | level, consumption rate, range, fuel door status |
| **Safety** | ABS active, stability control active, airbag ready, passenger presence |
| **Position** | latitude, longitude, altitude, heading, ground speed, trip distance |
| **Interior** | odometer, cabin air quality, cabin noise level |

**Base Class Interface:**

```cpp
class VehicleHAL : public QObject {
    Q_OBJECT
public:
    enum class PropertyType {
        // 60+ property types defined
    };

    // Pure virtual methods to override in subclasses
    virtual void initialize() = 0;
    virtual void shutdown() = 0;
    virtual QVariant getProperty(PropertyType type) = 0;
    virtual bool setProperty(PropertyType type, const QVariant &value) = 0;
    virtual bool isPropertyWritable(PropertyType type) = 0;
    virtual void subscribe(PropertyType type, QObject *subscriber) = 0;
    virtual void unsubscribe(PropertyType type, QObject *subscriber) = 0;

    // Helper methods
    static QString propertyTypeToString(PropertyType type);
    static PropertyType propertyTypeFromString(const QString &name);

signals:
    void propertyChanged(PropertyType type, const QVariant &value);
    void errorOccurred(const QString &message);
};
```

### 2. Host HAL (`HostHAL`)

Abstract base class representing the host system's hardware and software properties.

**Responsibilities:**
- Manage system/host properties (time, device info, resources, connectivity, sensors)
- Track system state and hardware capabilities
- Provide system diagnostics and status information

**Key Properties (50+ types):**

| Category | Properties |
|----------|-----------|
| **System Time** | current time, date, timezone, system uptime, boot time |
| **Device Info** | model, manufacturer, serial number, firmware version, hardware revision |
| **Resources** | CPU temperature, memory used, storage used, battery health, charging status |
| **Display** | brightness, backlight on/off, resolution, DPI, orientation |
| **Audio** | volume, mute, output device, input device, sample rate |
| **Connectivity** | WiFi enabled/connected, Bluetooth enabled/connected, cellular connected, GPS enabled/fix |
| **Power** | system on/off, battery percentage, charging enabled, USB connected, power mode |
| **Sensors** | accelerometer (x, y, z), gyroscope, compass, light level, proximity, pressure |

**Base Class Interface:**

```cpp
class HostHAL : public QObject {
    Q_OBJECT
public:
    enum class PropertyType {
        // 50+ property types defined
    };

    // Pure virtual methods
    virtual void initialize() = 0;
    virtual void shutdown() = 0;
    virtual QVariant getProperty(PropertyType type) = 0;
    virtual bool setProperty(PropertyType type, const QVariant &value) = 0;
    virtual bool isPropertyWritable(PropertyType type) = 0;
    virtual void subscribe(PropertyType type, QObject *subscriber) = 0;
    virtual void unsubscribe(PropertyType type, QObject *subscriber) = 0;

    // Helper methods
    static QString propertyTypeToString(PropertyType type);
    static PropertyType propertyTypeFromString(const QString &name);

signals:
    void propertyChanged(PropertyType type, const QVariant &value);
    void errorOccurred(const QString &message);
};
```

### 3. Device HAL (`DeviceHAL`)

Abstract base class for hardware device interfaces (CAN bus, I2C, GPIO, UART, etc.).

**Responsibilities:**
- Manage hardware device connections and state
- Handle device data send/receive
- Support command execution and configuration

**Supported Device Types:**

| Interface Type | Purpose | Properties |
|---|---|---|
| **CAN** | Vehicle CAN bus communication | Speed (kbps), ID filtering, error counters |
| **LIN** | Local Interconnect Network | Baud rate, nodes, scheduling |
| **I2C** | Inter-IC communication | Speed (kHz), address, timeout |
| **SPI** | Serial Peripheral Interface | Speed, mode, chip select |
| **UART** | Serial communication | Baud rate, parity, stop bits, flow control |
| **Ethernet** | Network communication | IP address, port, MAC address |
| **Bluetooth** | Wireless communication | Device name, MAC, power |
| **USB** | USB communication | Speed, descriptor, endpoints |
| **GPIO** | General Purpose I/O | Pin number, direction, state |
| **PWM** | Pulse Width Modulation | Frequency, duty cycle, channels |
| **Camera** | Video capture | Resolution, frame rate, codec |
| **LIDAR** | Distance measurement | Range, scan rate, points per frame |
| **RADAR** | Motion detection | Frequency, range, resolution |
| **Thermometer** | Temperature sensor | Range, accuracy, update rate |
| **GPS** | Global positioning | Accuracy, update rate, DGPS support |

**Device State Machine:**

```
[Offline] → [Connecting] → [Online]
    ↑           ↓            ↓
    └───────────[Error]──────┘
```

**Base Class Interface:**

```cpp
class DeviceHAL : public QObject {
    Q_OBJECT
public:
    enum class Type { CAN, I2C, GPIO, UART, GPS, /* ... */ };
    enum class State { Offline, Connecting, Online, Error };

    // Pure virtual methods
    virtual Type getType() const = 0;
    virtual QString getName() const = 0;
    virtual QString getDescription() const = 0;
    virtual void initialize() = 0;
    virtual void shutdown() = 0;
    virtual State getState() const = 0;
    virtual bool isConnected() const = 0;
    virtual bool sendData(const QByteArray &data) = 0;
    virtual bool sendCommand(const QString &command, const QVariantMap &args) = 0;
    virtual QVariantMap getStatus() const = 0;
    virtual bool setConfig(const QString &key, const QVariant &value) = 0;
    virtual QVariant getConfig(const QString &key) const = 0;

signals:
    void stateChanged(State newState);
    void dataReceived(const QByteArray &data);
    void errorOccurred(const QString &message);
    void connected();
    void disconnected();
};
```

## HAL Manager

The `HALManager` singleton orchestrates all three HAL types at the application level.

**Responsibilities:**
- Register and manage Vehicle, Host, and Device HALs
- Provide unified property access interface
- Route property change signals to EventBus
- Maintain device registry
- Provide diagnostics and status reporting

**Architecture:**

```
Application
    ↓
┌─────────────────────┐
│   HALManager        │ (Singleton)
│  (Orchestrator)     │
└─────────────────────┘
    ↓       ↓       ↓
    │       │       └─→ [Device HALs] (0..N)
    │       │
    │       └─→ [HostHAL]
    │
    └─→ [VehicleHAL]
         ↓
    ┌────────────────┐
    │   EventBus     │ (Property changes broadcast)
    └────────────────┘
         ↓
    [WebSocket Clients]
```

### Initialization

```cpp
// Get singleton instance
HALManager &halManager = HALManager::instance();

// Initialize with default mock HALs
halManager.initialize(true);

// Or initialize without default devices (for custom setup)
halManager.initialize(false);
auto vehicleHal = std::make_shared<CustomVehicleHAL>();
auto hostHal = std::make_shared<CustomHostHAL>();
halManager.setVehicleHAL(vehicleHal);
halManager.setHostHAL(hostHal);
```

### Property Access

```cpp
// Get vehicle property
QVariant speed = halManager.getVehicleProperty(VehicleHAL::PropertyType::Speed);

// Set vehicle property (if writable)
halManager.setVehicleProperty(VehicleHAL::PropertyType::HeadlightsOn, true);

// Subscribe to property changes
QObject::connect(&halManager, &HALManager::vehiclePropertyChanged,
    [](const VehicleHAL::PropertyType &type, const QVariant &value) {
        qDebug() << "Vehicle property changed:" << value;
    });
```

### Device Management

```cpp
// List all devices
QList<std::shared_ptr<DeviceHAL>> devices = halManager.getDevices();

// Get device by name
auto canBus = halManager.getDevice("CAN0");

// Get devices by type
auto gpioDevices = halManager.getDevicesByType(DeviceHAL::Type::GPIO);

// Register custom device
auto customDevice = std::make_shared<CustomDeviceHAL>();
halManager.registerDeviceHAL(customDevice);
```

## Mock Implementations

### MockVehicleHAL

Provides realistic simulated vehicle data for testing and development without physical hardware.

**Features:**

- **Driving Simulation**: Realistic speed profiles with 25-second acceleration/deceleration cycles
- **Fuel Consumption**: Linked to vehicle speed (0.001% per second at max speed)
- **Location Tracking**: Simulated circular driving pattern with latitude/longitude updates
- **Engine Temperature**: Realistic variations (±1°C) within operating range (85-110°C)
- **Odometer/Trip**: Tracked in real-time based on speed and elapsed time

**Simulation Parameters:**

```cpp
// Adjust simulation speed (1.0 = real-time)
mockVehicle->setSimulationSpeed(2.0);  // 2x faster simulation

// Toggle driving simulation
mockVehicle->setSimulateDriving(true);  // Enable realistic driving patterns
```

**Example Usage:**

```cpp
auto vehicleHal = std::make_shared<MockVehicleHAL>();
vehicleHal->initialize();

// Speed gradually increases from 0 to 100 km/h over ~12 seconds
// Then decreases back to 0 over ~12 seconds in a cycle
while (running) {
    QVariant speed = vehicleHal->getProperty(VehicleHAL::PropertyType::Speed);
    QVariant fuel = vehicleHal->getProperty(VehicleHAL::PropertyType::FuelLevel);
    QVariant odometer = vehicleHal->getProperty(VehicleHAL::PropertyType::Odometer);
    // Use simulated data...
}
```

### MockHostHAL

Simulates host/system properties with realistic variations.

**Features:**

- **System Time**: Updates every second from QDateTime
- **Memory**: Realistic ±10 MB variations (1024-3500 MB range)
- **CPU Temperature**: ±2°C variations (50-85°C range)
- **Connectivity**: WiFi, Bluetooth, GPS, cellular simulation
- **Power State**: Battery level, charging status simulation

**Example Usage:**

```cpp
auto hostHal = std::make_shared<MockHostHAL>();
hostHal->initialize();

// Monitor system properties
QVariant cpuTemp = hostHal->getProperty(HostHAL::PropertyType::CPUTemperature);
QVariant memory = hostHal->getProperty(HostHAL::PropertyType::MemoryUsed);
QVariant wifiConnected = hostHal->getProperty(HostHAL::PropertyType::WiFiConnected);
```

### MockDeviceHAL

Simulates hardware device interfaces with realistic state transitions and data buffering.

**Features:**

- **State Management**: Offline → Connecting (500ms) → Online state transitions
- **Data Buffering**: Circular RX buffer for received data simulation
- **Command Execution**: Supports get_status, reset, test, echo commands
- **Configuration Storage**: Per-device configuration persistence

**Example Usage:**

```cpp
auto canDevice = std::make_shared<MockDeviceHAL>(DeviceHAL::Type::CAN, "CAN0");
canDevice->initialize();

// Wait for connection
while (canDevice->getState() != DeviceHAL::State::Online) {
    QThread::msleep(100);
}

// Send/receive data
QByteArray data = QByteArray::fromHex("123456");
canDevice->sendData(data);

// Execute commands
QVariantMap status = canDevice->getStatus();
qDebug() << "Device connected:" << status["connected"];
```

## Integration with EventBus

The HAL system integrates seamlessly with Crankshaft's EventBus for property change broadcasting:

### Signal Routing

In `core/main.cpp`:

```cpp
// Connect vehicle property changes to EventBus
QObject::connect(&halManager, &HALManager::vehiclePropertyChanged,
    [](const VehicleHAL::PropertyType &type, const QVariant &value) {
        QString propertyName = VehicleHAL::propertyTypeToString(type);
        EventBus::instance().publish("hal/vehicle/" + propertyName, value);
    });

// Connect host property changes to EventBus
QObject::connect(&halManager, &HALManager::hostPropertyChanged,
    [](const HostHAL::PropertyType &type, const QVariant &value) {
        QString propertyName = HostHAL::propertyTypeToString(type);
        EventBus::instance().publish("hal/host/" + propertyName, value);
    });
```

### WebSocket Broadcasting

Property changes automatically broadcast to all connected WebSocket clients via the existing EventBus→WebSocket routing:

```
HALManager propertyChanged signal
    ↓
EventBus publish("hal/vehicle/Speed", 65.5)
    ↓
WebSocketServer broadcastEvent()
    ↓
All connected clients receive: { topic: "hal/vehicle/Speed", data: 65.5 }
```

### UI Integration Example

In QML:

```qml
import QtQuick
import Qt.WebSockets

WebSocket {
    id: socket
    url: "ws://localhost:8080"

    onTextMessageReceived: (message) => {
        let event = JSON.parse(message);
        if (event.topic === "hal/vehicle/Speed") {
            speedometer.value = event.data;
        }
    }
}
```

## Extending the HAL System

### Implementing Custom Vehicle HAL

For example, implementing a CAN-based vehicle interface:

```cpp
#include "hal/VehicleHAL.h"

class CANVehicleHAL : public VehicleHAL {
public:
    void initialize() override {
        // Open CAN bus device
        canDevice = std::make_shared<CANDevice>("/dev/can0");
        canDevice->open();
        
        // Start reading CAN frames
        connect(canDevice.get(), &CANDevice::frameReceived,
                this, &CANVehicleHAL::onCANFrameReceived);
    }

    void shutdown() override {
        if (canDevice) canDevice->close();
    }

    QVariant getProperty(PropertyType type) override {
        QMutexLocker lock(&propertyMutex);
        return properties[type];
    }

    bool setProperty(PropertyType type, const QVariant &value) override {
        if (!isPropertyWritable(type)) return false;
        
        // Send CAN frame to set property
        CANFrame frame = createCANFrame(type, value);
        canDevice->sendFrame(frame);
        return true;
    }

    bool isPropertyWritable(PropertyType type) override {
        // Headlights, climate, locks are writable
        return type == PropertyType::HeadlightsOn ||
               type == PropertyType::TargetTemperature ||
               // ... other writable properties
               false;
    }

    void subscribe(PropertyType type, QObject *subscriber) override {
        QMutexLocker lock(&subscriberMutex);
        subscribers[type].append(subscriber);
    }

    void unsubscribe(PropertyType type, QObject *subscriber) override {
        QMutexLocker lock(&subscriberMutex);
        subscribers[type].removeAll(subscriber);
    }

private:
    void onCANFrameReceived(const CANFrame &frame) {
        PropertyType type = parseCANFrame(frame);
        QVariant value = extractValue(frame);
        
        {
            QMutexLocker lock(&propertyMutex);
            properties[type] = value;
        }
        
        emit propertyChanged(type, value);
    }

    std::shared_ptr<CANDevice> canDevice;
    QMutex propertyMutex;
    QMutex subscriberMutex;
    QMap<PropertyType, QVariant> properties;
    QMap<PropertyType, QList<QObject *>> subscribers;
};
```

### Steps to Add Custom HAL

1. **Create header file** (e.g., `core/hal/CANVehicleHAL.h`)
   - Inherit from appropriate base class (VehicleHAL, HostHAL, or DeviceHAL)
   - Implement all pure virtual methods

2. **Implement methods**
   - Override initialize() to setup hardware
   - Implement getProperty/setProperty for property access
   - Implement subscribe/unsubscribe for change notifications
   - Emit signals on property changes

3. **Register with HALManager**
   ```cpp
   auto customHal = std::make_shared<CANVehicleHAL>();
   customHal->initialize();
   HALManager::instance().setVehicleHAL(customHal);
   ```

4. **Add to CMakeLists.txt**
   ```cmake
   set(SOURCES ${SOURCES}
       core/hal/CANVehicleHAL.cpp
   )
   ```

5. **Test thoroughly**
   - Verify property changes are emitted correctly
   - Check thread safety with concurrent access
   - Validate EventBus signal routing

## Diagnostics and Debugging

### HALManager Diagnostics

```cpp
// Get comprehensive diagnostics
QVariantMap diagnostics = halManager.getDiagnostics();

// Get human-readable status report
QString report = halManager.getStatusReport();
qDebug() << report;
```

**Example Output:**

```
==== HAL Status Report ====
Vehicle HAL: MockVehicleHAL
- Speed: 65.5 km/h
- RPM: 3275
- Fuel Level: 67.2%
- Engine Temperature: 95.5°C

Host HAL: MockHostHAL
- System Time: 2025-01-15 14:32:45
- CPU Temperature: 72.3°C
- Memory Used: 2048 MB
- Battery: 85% (Not charging)

Devices: 3 registered
1. CAN0 (CAN Bus) - Online
2. I2C0 (I2C) - Offline
3. GPS0 (GPS) - Online
==========================
```

### Property Change Logging

Enable detailed logging of HAL property changes:

```cpp
// In main.cpp or configuration
Logger::instance().setLevel(Logger::Level::Debug);

// All property changes will be logged to debug level
// Vehicle property updated: Speed
// Vehicle property updated: RPM
// Host property updated: CPUTemperature
```

## Performance Considerations

### Raspberry Pi 4 Optimization

For resource-constrained environments like Raspberry Pi 4:

1. **Mock Implementation Updates**
   - Adjust update timers: Vehicle HAL defaults to 500ms updates
   - Host HAL updates every 1 second
   - Device HALs update on-demand

2. **Property Subscription**
   - Only subscribe to properties being actively used
   - Unsubscribe when monitoring is complete
   - Avoids unnecessary signal emission

3. **Memory Management**
   - Mock implementations are lightweight (~1-2 MB)
   - Device registry cleared on shutdown
   - Property caches are minimal

4. **CPU Impact**
   - Mock vehicle driving simulation: ~2% CPU on RPi4
   - Mock host system monitoring: <1% CPU
   - Device I/O operations are event-driven, not polling

## Testing Strategy

### Unit Tests

Located in `tests/test_vehicle_hal.cpp`, `tests/test_host_hal.cpp`, `tests/test_device_hal.cpp`:

- Property get/set operations
- Read-only vs. writable properties
- Subscription/notification mechanism
- Mock implementation data updates
- HALManager singleton lifecycle
- Thread safety with concurrent access

### Integration Tests

- HAL signals properly routed to EventBus
- EventBus messages received by WebSocket clients
- UI correctly displays property values from WebSocket
- Multiple property changes processed sequentially

### Example Test

```cpp
#include <catch2/catch_test_macros.hpp>
#include "hal/MockVehicleHAL.h"

TEST_CASE("MockVehicleHAL speed simulation") {
    auto hal = std::make_shared<MockVehicleHAL>();
    hal->initialize();

    // Speed should start at 0
    REQUIRE(hal->getProperty(VehicleHAL::PropertyType::Speed).toDouble() == 0.0);

    // After 2 seconds, speed should be increasing
    QThread::sleep(2000);
    double speed = hal->getProperty(VehicleHAL::PropertyType::Speed).toDouble();
    REQUIRE(speed > 0.0);
    REQUIRE(speed < 120.0);  // Max simulation speed

    hal->shutdown();
}
```

## References

- [Android Automotive HAL Documentation](https://source.android.com/docs/automotive/hal)
- Qt Signal/Slot Documentation
- Crankshaft EventBus Architecture (`docs/ARCHITECTURE.md`)
- Design for Driving Guidelines (`docs/UI_GUIDELINES.md`)

## Future Enhancements

1. **HAL Persistence**: Save/restore HAL state between sessions
2. **Recording/Playback**: Record real HAL data, replay for testing
3. **Property History**: Maintain time-series history of property changes
4. **Remote HAL**: Network-based HAL for testing on different machines
5. **Hardware Profiling**: Performance metrics and diagnostics for custom HALs
6. **Extension HALs**: Allow extensions to register custom HAL implementations
