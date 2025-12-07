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

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QVariantMap>

/**
 * @brief Transport layer types
 * 
 * Represents the physical/logical communication channels used to
 * transfer data between functional devices and hardware.
 */
enum class TransportType {
  UART,           // Serial/UART transport
  USB,            // USB transport
  SPI,            // SPI bus transport
  I2C,            // I2C bus transport
  BLUETOOTH,      // Bluetooth transport
  WIFI,           // WiFi/Network transport
  ETHERNET,       // Ethernet transport
  CAN,            // Native CAN bus transport
  LIN,            // LIN bus transport
  VIRTUAL,        // Virtual/mock transport for testing
  MEMORY          // Shared memory transport
};

/**
 * @brief Transport state enumeration
 */
enum class TransportState {
  DISCONNECTED,   // Not connected
  CONNECTING,     // Connection in progress
  CONNECTED,      // Connected and ready
  ERROR           // Error state
};

/**
 * @brief Transport base class
 * 
 * Abstract base class for all transport layer implementations.
 * Handles the physical/logical communication channel independent
 * of the functional device using it.
 * 
 * Examples:
 *   - UART transport can be used by GPS, CAN adapter, or any serial device
 *   - USB transport can carry GPS data, CAN frames, camera streams, etc.
 *   - SPI transport can interface with CAN controllers, displays, sensors
 */
class Transport : public QObject {
  Q_OBJECT

 public:
  explicit Transport(QObject* parent = nullptr);
  virtual ~Transport() = default;

  /**
   * @brief Get transport type
   */
  virtual TransportType getType() const = 0;

  /**
   * @brief Get transport name
   */
  virtual QString getName() const = 0;

  /**
   * @brief Open/connect the transport
   * @return true if successful
   */
  virtual bool open() = 0;

  /**
   * @brief Close/disconnect the transport
   */
  virtual void close() = 0;

  /**
   * @brief Check if transport is open and ready
   */
  virtual bool isOpen() const = 0;

  /**
   * @brief Get current transport state
   */
  virtual TransportState getState() const = 0;

  /**
   * @brief Write data to transport
   * @param data Data to write
   * @return Number of bytes written, -1 on error
   */
  virtual qint64 write(const QByteArray& data) = 0;

  /**
   * @brief Read data from transport
   * @param maxSize Maximum bytes to read (0 = all available)
   * @return Data read, empty if no data or error
   */
  virtual QByteArray read(qint64 maxSize = 0) = 0;

  /**
   * @brief Check if data is available to read
   * @return Number of bytes available
   */
  virtual qint64 bytesAvailable() const = 0;

  /**
   * @brief Flush write buffer
   */
  virtual void flush() = 0;

  /**
   * @brief Configure transport-specific settings
   * @param key Configuration key
   * @param value Configuration value
   * @return true if successful
   */
  virtual bool configure(const QString& key, const QVariant& value) = 0;

  /**
   * @brief Get transport-specific configuration
   * @param key Configuration key
   * @return Configuration value
   */
  virtual QVariant getConfiguration(const QString& key) const = 0;

 signals:
  /**
   * @brief Emitted when data is received
   */
  void dataReceived();

  /**
   * @brief Emitted when transport state changes
   */
  void stateChanged(TransportState state);

  /**
   * @brief Emitted on error
   */
  void errorOccurred(const QString& error);

  /**
   * @brief Emitted when transport connects
   */
  void connected();

  /**
   * @brief Emitted when transport disconnects
   */
  void disconnected();
};
