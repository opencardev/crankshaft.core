/*
 * Project: Crankshaft
 * This file is part of Crankshaft project.
 * Copyright (C) 2025 OpenCarDev Team
 *
 *  Crankshaft is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Crankshaft is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Crankshaft. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <QMutex>
#include <QObject>
#include <QString>
#include <QVariant>

#include "../transport/Transport.h"

/**
 * @brief Functional device types
 *
 * Represents WHAT the device does, independent of HOW it communicates.
 * A GPS device provides location data regardless of whether it uses
 * UART, USB, Bluetooth, or any other transport.
 */
enum class FunctionalDeviceType {
  /* Vehicle Communication */
  VEHICLE_CAN,  // CAN bus for vehicle data
  VEHICLE_LIN,  // LIN bus for vehicle data

  /* Position/Navigation */
  GPS,      // GPS/GNSS receiver
  IMU,      // Inertial Measurement Unit
  COMPASS,  // Magnetometer/compass

  /* Imaging */
  CAMERA,  // Camera/video capture
  LIDAR,   // LIDAR sensor
  RADAR,   // RADAR sensor

  /* Environmental Sensors */
  THERMOMETER,  // Temperature sensor
  HUMIDITY,     // Humidity sensor
  PRESSURE,     // Pressure/barometric sensor

  /* Motion Sensors */
  ACCELEROMETER,  // Accelerometer
  GYROSCOPE,      // Gyroscope

  /* Display/Output */
  DISPLAY,   // Display/screen
  LED,       // LED indicators
  SPEAKER,   // Audio output
  VIBRATOR,  // Haptic feedback

  /* Control */
  GPIO,  // General purpose I/O
  PWM,   // PWM control (motors, servos)

  /* Power */
  POWER_SUPPLY,  // Power supply monitoring
  BATTERY,       // Battery management

  /* Connectivity */
  BLUETOOTH,  // Bluetooth radio
  WIFI,       // WiFi radio
  CELLULAR    // Cellular modem
};

/**
 * @brief Device state enumeration
 */
enum class DeviceState {
  OFFLINE,       // Not connected or not available
  INITIALIZING,  // Initialization in progress
  ONLINE,        // Connected and operational
  ERROR          // Error state
};

/**
 * @brief Functional device base class
 *
 * Abstract base class for functional devices. Each device provides
 * specific functionality (GPS location, CAN messages, camera frames)
 * and uses a Transport to actually communicate with hardware.
 *
 * Architecture:
 *   FunctionalDevice (WHAT: GPS, CAN, Camera)
 *       ↓ uses
 *   Transport (HOW: UART, USB, SPI, Bluetooth)
 *       ↓
 *   Hardware
 *
 * Example:
 *   GPSDevice gps(uartTransport);  // GPS over UART
 *   GPSDevice gps(bluetoothTransport);  // Same GPS device, different transport
 */
class FunctionalDevice : public QObject {
  Q_OBJECT

 public:
  explicit FunctionalDevice(Transport* transport, QObject* parent = nullptr);
  virtual ~FunctionalDevice() = default;

  /**
   * @brief Get functional device type
   */
  virtual FunctionalDeviceType getType() const = 0;

  /**
   * @brief Get device name
   */
  virtual QString getName() const = 0;

  /**
   * @brief Get device description
   */
  virtual QString getDescription() const = 0;

  /**
   * @brief Initialize the device
   * @return true if successful
   */
  virtual bool initialize() = 0;

  /**
   * @brief Shutdown the device
   */
  virtual void shutdown() = 0;

  /**
   * @brief Get current device state
   */
  virtual DeviceState getState() const = 0;

  /**
   * @brief Check if device is operational
   */
  virtual bool isOnline() const = 0;

  /**
   * @brief Get the transport used by this device
   */
  Transport* getTransport() const {
    return m_transport;
  }

  /**
   * @brief Set device configuration
   */
  virtual bool setConfig(const QString& key, const QVariant& value) = 0;

  /**
   * @brief Get device configuration
   */
  virtual QVariant getConfig(const QString& key) const = 0;

 signals:
  /**
   * @brief Emitted when device state changes
   */
  void stateChanged(DeviceState state);

  /**
   * @brief Emitted on error
   */
  void errorOccurred(const QString& error);

 protected:
  Transport* m_transport;
  mutable QMutex m_mutex;
};
