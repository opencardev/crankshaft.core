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

#include <QDateTime>

#include "FunctionalDevice.h"

/**
 * @brief GPS location data structure
 */
struct GPSLocation {
  double latitude;      // Degrees (-90 to +90)
  double longitude;     // Degrees (-180 to +180)
  double altitude;      // Metres above sea level
  double speed;         // Speed in m/s
  double heading;       // Heading in degrees (0-360)
  quint8 satellites;    // Number of satellites in view
  QString fixType;      // Fix type: "none", "2D", "3D"
  double hdop;          // Horizontal dilution of precision
  double vdop;          // Vertical dilution of precision
  QDateTime timestamp;  // Time of fix
};

/**
 * @brief GPS device class
 *
 * Provides GPS/GNSS location functionality. Transport-agnostic:
 * can use UART, USB, Bluetooth, or any other transport that
 * provides NMEA sentences or binary GPS protocol data.
 *
 * Examples:
 *   // GPS over UART (traditional GPS receiver)
 *   auto uart = new UARTTransport("/dev/ttyUSB0");
 *   auto gps = new GPSDevice(uart);
 *
 *   // GPS over Bluetooth (Bluetooth GPS receiver)
 *   auto bt = new BluetoothTransport("00:11:22:33:44:55");
 *   auto gps = new GPSDevice(bt);
 *
 *   // GPS over USB (USB GPS dongle)
 *   auto usb = new USBTransport("/dev/ttyACM0");
 *   auto gps = new GPSDevice(usb);
 */
class GPSDevice : public FunctionalDevice {
  Q_OBJECT

 public:
  explicit GPSDevice(Transport* transport, QObject* parent = nullptr);
  ~GPSDevice() override;

  FunctionalDeviceType getType() const override {
    return FunctionalDeviceType::GPS;
  }
  QString getName() const override {
    return "GPS Receiver";
  }
  QString getDescription() const override {
    return "GPS/GNSS location receiver";
  }

  bool initialize() override;
  void shutdown() override;
  DeviceState getState() const override {
    return m_state;
  }
  bool isOnline() const override {
    return m_state == DeviceState::ONLINE;
  }

  bool setConfig(const QString& key, const QVariant& value) override;
  QVariant getConfig(const QString& key) const override;

  /**
   * @brief Get current GPS location
   */
  GPSLocation getCurrentLocation() const;

  /**
   * @brief Get number of satellites in view
   */
  quint8 getSatelliteCount() const;

 signals:
  /**
   * @brief Emitted when location updates
   */
  void locationUpdated(const GPSLocation& location);

  /**
   * @brief Emitted when satellite count changes
   */
  void satellitesChanged(quint8 count);

 private slots:
  void onTransportDataReceived();

 private:
  void parseNMEA(const QByteArray& data);

  DeviceState m_state;
  GPSLocation m_currentLocation;
  QByteArray m_buffer;  // Buffer for incomplete NMEA sentences
  QVariantMap m_config;
};
