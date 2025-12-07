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

#include "../../transport/Transport.h"
#include <QTimer>
#include <QQueue>

/**
 * @brief Mock/virtual transport for testing
 * 
 * Simulates a transport layer without requiring actual hardware.
 * Useful for:
 *   - Unit testing functional devices
 *   - Development without hardware
 *   - Simulating multiple devices
 *   - Testing error conditions
 * 
 * Can inject data to simulate received data, and captures
 * written data for verification in tests.
 */
class MockTransport : public Transport {
  Q_OBJECT

 public:
  explicit MockTransport(const QString& name = "Mock", QObject* parent = nullptr);
  ~MockTransport() override;

  TransportType getType() const override { return TransportType::VIRTUAL; }
  QString getName() const override { return m_name; }

  bool open() override;
  void close() override;
  bool isOpen() const override { return m_isOpen; }
  TransportState getState() const override { return m_state; }

  qint64 write(const QByteArray& data) override;
  QByteArray read(qint64 maxSize = 0) override;
  qint64 bytesAvailable() const override;
  void flush() override;

  bool configure(const QString& key, const QVariant& value) override;
  QVariant getConfiguration(const QString& key) const override;

  // Mock-specific methods for testing

  /**
   * @brief Inject data as if received from hardware
   */
  void injectData(const QByteArray& data);

  /**
   * @brief Get data that was written to transport
   */
  QByteArray getWrittenData();

  /**
   * @brief Clear written data buffer
   */
  void clearWrittenData();

  /**
   * @brief Simulate transport error
   */
  void simulateError(const QString& error);

  /**
   * @brief Enable/disable automatic data injection
   * When enabled, periodically injects test data
   */
  void setAutoInject(bool enabled, int intervalMs = 1000);

 private slots:
  void onAutoInjectTimer();

 private:
  QString m_name;
  bool m_isOpen;
  TransportState m_state;
  QQueue<QByteArray> m_receiveQueue;
  QByteArray m_writtenData;
  QVariantMap m_config;
  QTimer* m_autoInjectTimer;
  int m_autoInjectCounter;
};
