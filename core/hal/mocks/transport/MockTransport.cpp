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

#include "MockTransport.h"

#include "../../../services/logging/Logger.h"

MockTransport::MockTransport(const QString& name, QObject* parent)
    : Transport(parent),
      m_name(name),
      m_isOpen(false),
      m_state(TransportState::DISCONNECTED),
      m_autoInjectCounter(0) {
  m_autoInjectTimer = new QTimer(this);
  connect(m_autoInjectTimer, &QTimer::timeout, this, &MockTransport::onAutoInjectTimer);
}

MockTransport::~MockTransport() {
  close();
}

bool MockTransport::open() {
  if (m_isOpen) {
    return true;
  }

  Logger::instance().info(QString("MockTransport(%1): Opening").arg(m_name));
  m_state = TransportState::CONNECTING;
  emit stateChanged(m_state);

  m_isOpen = true;
  m_state = TransportState::CONNECTED;
  emit stateChanged(m_state);
  emit connected();

  Logger::instance().info(QString("MockTransport(%1): Connected").arg(m_name));
  return true;
}

void MockTransport::close() {
  if (!m_isOpen) {
    return;
  }

  Logger::instance().info(QString("MockTransport(%1): Closing").arg(m_name));
  m_autoInjectTimer->stop();
  m_isOpen = false;
  m_state = TransportState::DISCONNECTED;
  emit stateChanged(m_state);
  emit disconnected();
}

qint64 MockTransport::write(const QByteArray& data) {
  if (!m_isOpen) {
    return -1;
  }

  m_writtenData.append(data);
  Logger::instance().info(
      QString("MockTransport(%1): Wrote %2 bytes").arg(m_name).arg(data.size()));
  return data.size();
}

QByteArray MockTransport::read(qint64 maxSize) {
  if (!m_isOpen || m_receiveQueue.isEmpty()) {
    return QByteArray();
  }

  QByteArray data = m_receiveQueue.dequeue();

  if (maxSize > 0 && data.size() > maxSize) {
    // Put remaining data back
    m_receiveQueue.prepend(data.mid(maxSize));
    data = data.left(maxSize);
  }

  Logger::instance().info(QString("MockTransport(%1): Read %2 bytes").arg(m_name).arg(data.size()));
  return data;
}

qint64 MockTransport::bytesAvailable() const {
  qint64 total = 0;
  for (const auto& data : m_receiveQueue) {
    total += data.size();
  }
  return total;
}

void MockTransport::flush() {
  // Mock transport doesn't buffer writes
}

bool MockTransport::configure(const QString& key, const QVariant& value) {
  m_config[key] = value;
  return true;
}

QVariant MockTransport::getConfiguration(const QString& key) const {
  return m_config.value(key);
}

void MockTransport::injectData(const QByteArray& data) {
  if (!m_isOpen) {
    Logger::instance().info(QString("MockTransport(%1): Cannot inject data, not open").arg(m_name));
    return;
  }

  m_receiveQueue.enqueue(data);
  Logger::instance().info(
      QString("MockTransport(%1): Injected %2 bytes").arg(m_name).arg(data.size()));
  emit dataReceived();
}

QByteArray MockTransport::getWrittenData() {
  return m_writtenData;
}

void MockTransport::clearWrittenData() {
  m_writtenData.clear();
}

void MockTransport::simulateError(const QString& error) {
  Logger::instance().info(QString("MockTransport(%1): Simulating error: %2").arg(m_name, error));
  m_state = TransportState::ERROR;
  emit stateChanged(m_state);
  emit errorOccurred(error);
}

void MockTransport::setAutoInject(bool enabled, int intervalMs) {
  if (enabled) {
    m_autoInjectTimer->start(intervalMs);
    Logger::instance().info(QString("MockTransport(%1): Auto-inject enabled, interval %2ms")
                                .arg(m_name)
                                .arg(intervalMs));
  } else {
    m_autoInjectTimer->stop();
    Logger::instance().info(QString("MockTransport(%1): Auto-inject disabled").arg(m_name));
  }
}

void MockTransport::onAutoInjectTimer() {
  // Inject some test data
  // This would be customized based on what device is using the transport

  m_autoInjectCounter++;

  // Example: Inject NMEA sentence for GPS
  QString nmea = QString("$GPGGA,%1,5140.1234,N,00009.5678,W,1,08,0.9,100.0,M,47.0,M,,*47\r\n")
                     .arg(QTime::currentTime().toString("hhmmss.zzz"));

  injectData(nmea.toLatin1());
}
