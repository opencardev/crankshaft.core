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

#include "Transport.h"

/**
 * @brief UART/Serial transport implementation
 *
 * Provides serial communication for devices that use UART/RS232.
 * Can be used by: GPS receivers, CAN adapters, debug consoles, etc.
 *
 * Configuration keys:
 *   - "port": Serial port path (e.g., "/dev/ttyUSB0")
 *   - "baudRate": Baud rate (e.g., 9600, 115200)
 *   - "dataBits": Data bits (5, 6, 7, 8)
 *   - "parity": Parity ("none", "even", "odd")
 *   - "stopBits": Stop bits (1, 2)
 *   - "flowControl": Flow control ("none", "hardware", "software")
 */
class UARTTransport : public Transport {
  Q_OBJECT

 public:
  explicit UARTTransport(const QString& portName, QObject* parent = nullptr);
  ~UARTTransport() override;

  TransportType getType() const override {
    return TransportType::UART;
  }
  QString getName() const override;

  bool open() override;
  void close() override;
  bool isOpen() const override;
  TransportState getState() const override;

  qint64 write(const QByteArray& data) override;
  QByteArray read(qint64 maxSize = 0) override;
  qint64 bytesAvailable() const override;
  void flush() override;

  bool configure(const QString& key, const QVariant& value) override;
  QVariant getConfiguration(const QString& key) const override;

  // UART-specific configuration helpers
  bool setBaudRate(qint32 baudRate);
  bool setDataBits(quint8 dataBits);
  bool setParity(const QString& parity);
  bool setStopBits(quint8 stopBits);
  bool setFlowControl(const QString& flowControl);

 private:
  QString m_portName;
  TransportState m_state;
  QVariantMap m_config;

  // Platform-specific serial port handle would go here
  // void* m_serialHandle;
};
