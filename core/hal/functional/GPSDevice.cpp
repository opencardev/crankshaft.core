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

#include "GPSDevice.h"

#include "../../services/logging/Logger.h"

GPSDevice::GPSDevice(Transport* transport, QObject* parent)
    : FunctionalDevice(transport, parent), m_state(DeviceState::OFFLINE) {
  // Initialize location with invalid values
  m_currentLocation.latitude = 0.0;
  m_currentLocation.longitude = 0.0;
  m_currentLocation.altitude = 0.0;
  m_currentLocation.speed = 0.0;
  m_currentLocation.heading = 0.0;
  m_currentLocation.satellites = 0;
  m_currentLocation.fixType = "none";
  m_currentLocation.hdop = 99.9;
  m_currentLocation.vdop = 99.9;

  // Connect to transport signals
  if (m_transport) {
    connect(m_transport, &Transport::dataReceived, this, &GPSDevice::onTransportDataReceived);
  }
}

GPSDevice::~GPSDevice() {
  shutdown();
}

bool GPSDevice::initialize() {
  if (m_state == DeviceState::ONLINE) {
    return true;
  }

  Logger::instance().info("GPSDevice: Initializing GPS device");

  if (!m_transport) {
    Logger::instance().info("GPSDevice: No transport configured");
    m_state = DeviceState::ERROR;
    emit stateChanged(m_state);
    return false;
  }

  m_state = DeviceState::INITIALIZING;
  emit stateChanged(m_state);

  // Open transport if not already open
  if (!m_transport->isOpen()) {
    if (!m_transport->open()) {
      Logger::instance().info("GPSDevice: Failed to open transport");
      m_state = DeviceState::ERROR;
      emit stateChanged(m_state);
      return false;
    }
  }

  m_state = DeviceState::ONLINE;
  emit stateChanged(m_state);
  Logger::instance().info("GPSDevice: Initialization complete");
  return true;
}

void GPSDevice::shutdown() {
  if (m_state == DeviceState::OFFLINE) {
    return;
  }

  Logger::instance().info("GPSDevice: Shutting down");
  m_state = DeviceState::OFFLINE;
  emit stateChanged(m_state);
}

bool GPSDevice::setConfig(const QString& key, const QVariant& value) {
  m_config[key] = value;
  return true;
}

QVariant GPSDevice::getConfig(const QString& key) const {
  return m_config.value(key);
}

GPSLocation GPSDevice::getCurrentLocation() const {
  QMutexLocker locker(&m_mutex);
  return m_currentLocation;
}

quint8 GPSDevice::getSatelliteCount() const {
  QMutexLocker locker(&m_mutex);
  return m_currentLocation.satellites;
}

void GPSDevice::onTransportDataReceived() {
  if (!m_transport) {
    return;
  }

  // Read available data from transport
  QByteArray data = m_transport->read();
  if (data.isEmpty()) {
    return;
  }

  // Append to buffer and parse
  m_buffer.append(data);
  parseNMEA(m_buffer);
}

void GPSDevice::parseNMEA(const QByteArray& data) {
  // Look for complete NMEA sentences (lines ending with \r\n)
  int start = 0;
  int end = m_buffer.indexOf("\r\n", start);

  while (end != -1) {
    QByteArray sentence = m_buffer.mid(start, end - start);

    // Process NMEA sentence
    QString line = QString::fromLatin1(sentence).trimmed();
    if (line.startsWith("$GPGGA") || line.startsWith("$GNGGA")) {
      // Parse GGA sentence for location and satellite count
      // TODO: Implement NMEA parsing
      Logger::instance().info(QString("GPSDevice: Received GGA: %1").arg(line));

      // Example: Update location (simplified)
      QMutexLocker locker(&m_mutex);
      m_currentLocation.satellites = 8;  // Would parse from sentence
      m_currentLocation.fixType = "3D";
      m_currentLocation.timestamp = QDateTime::currentDateTime();

      emit locationUpdated(m_currentLocation);
      emit satellitesChanged(m_currentLocation.satellites);
    }

    // Move to next sentence
    start = end + 2;  // Skip \r\n
    end = m_buffer.indexOf("\r\n", start);
  }

  // Remove processed data from buffer
  if (start > 0) {
    m_buffer.remove(0, start);
  }

  // Prevent buffer from growing indefinitely
  if (m_buffer.size() > 4096) {
    Logger::instance().info("GPSDevice: Buffer overflow, clearing");
    m_buffer.clear();
  }
}
