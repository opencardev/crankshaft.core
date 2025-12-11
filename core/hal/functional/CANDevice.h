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
 * @brief CAN message structure
 */
struct CANMessage {
  quint32 id;           // CAN identifier (11-bit or 29-bit)
  QByteArray data;      // Data payload (0-8 bytes for CAN 2.0, up to 64 for CAN FD)
  bool extended;        // Extended frame format (29-bit ID)
  bool rtr;             // Remote transmission request
  bool fd;              // CAN FD frame
  QDateTime timestamp;  // Reception/transmission time
};

/**
 * @brief CAN bus device class
 *
 * Provides CAN bus communication functionality. Transport-agnostic:
 * can use USB-CAN adapter, SPI-CAN controller, native CAN interface,
 * or any other transport that provides CAN frame data.
 *
 * Examples:
 *   // USB CAN adapter (e.g., Lawicel CANUSB, Peak PCAN-USB)
 *   auto usb = new USBTransport("/dev/ttyUSB0");
 *   auto can = new CANDevice(usb);
 *
 *   // SPI CAN controller (e.g., MCP2515)
 *   auto spi = new SPITransport(0, 0);  // SPI bus 0, chip select 0
 *   auto can = new CANDevice(spi);
 *
 *   // Native CAN interface (e.g., SocketCAN on Linux)
 *   auto native = new CANTransport("can0");
 *   auto can = new CANDevice(native);
 */
class CANDevice : public FunctionalDevice {
  Q_OBJECT

 public:
  explicit CANDevice(Transport* transport, QObject* parent = nullptr);
  ~CANDevice() override;

  FunctionalDeviceType getType() const override {
    return FunctionalDeviceType::VEHICLE_CAN;
  }
  QString getName() const override {
    return "CAN Bus";
  }
  QString getDescription() const override {
    return "Vehicle CAN bus interface";
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
   * @brief Send CAN message
   */
  bool sendMessage(const CANMessage& message);

  /**
   * @brief Set CAN bus bit rate
   */
  bool setBitRate(quint32 bitRate);

  /**
   * @brief Get current bit rate
   */
  quint32 getBitRate() const;

 signals:
  /**
   * @brief Emitted when CAN message received
   */
  void messageReceived(const CANMessage& message);

  /**
   * @brief Emitted on bus error
   */
  void busError(const QString& error);

  /**
   * @brief Emitted when bus status changes
   */
  void busStatusChanged(bool active);

 private slots:
  void onTransportDataReceived();

 private:
  void parseCANData(const QByteArray& data);

  DeviceState m_state;
  quint32 m_bitRate;
  QByteArray m_buffer;
  QVariantMap m_config;
};
