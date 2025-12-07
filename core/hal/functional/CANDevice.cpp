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

#include "CANDevice.h"
#include "../../Logger.h"

CANDevice::CANDevice(Transport* transport, QObject* parent)
    : FunctionalDevice(transport, parent),
      m_state(DeviceState::OFFLINE),
      m_bitRate(500000) {  // Default 500 kbps

  // Connect to transport signals
  if (m_transport) {
    connect(m_transport, &Transport::dataReceived, this, &CANDevice::onTransportDataReceived);
  }
}

CANDevice::~CANDevice() {
  shutdown();
}

bool CANDevice::initialize() {
  if (m_state == DeviceState::ONLINE) {
    return true;
  }

  Logger::instance()->log("CANDevice: Initializing CAN bus");

  if (!m_transport) {
    Logger::instance()->log("CANDevice: No transport configured");
    m_state = DeviceState::ERROR;
    emit stateChanged(m_state);
    return false;
  }

  m_state = DeviceState::INITIALIZING;
  emit stateChanged(m_state);

  // Open transport if not already open
  if (!m_transport->isOpen()) {
    if (!m_transport->open()) {
      Logger::instance()->log("CANDevice: Failed to open transport");
      m_state = DeviceState::ERROR;
      emit stateChanged(m_state);
      return false;
    }
  }

  // Configure bit rate based on transport type
  // USB-CAN adapters typically need AT commands
  // SPI-CAN controllers need register configuration
  // Native CAN uses ioctl or netlink
  setBitRate(m_bitRate);

  m_state = DeviceState::ONLINE;
  emit stateChanged(m_state);
  emit busStatusChanged(true);
  Logger::instance()->log(QString("CANDevice: Initialization complete, bit rate %1").arg(m_bitRate));
  return true;
}

void CANDevice::shutdown() {
  if (m_state == DeviceState::OFFLINE) {
    return;
  }

  Logger::instance()->log("CANDevice: Shutting down");
  m_state = DeviceState::OFFLINE;
  emit stateChanged(m_state);
  emit busStatusChanged(false);
}

bool CANDevice::setConfig(const QString& key, const QVariant& value) {
  m_config[key] = value;

  if (key == "bitRate") {
    return setBitRate(value.toUInt());
  }

  return true;
}

QVariant CANDevice::getConfig(const QString& key) const {
  return m_config.value(key);
}

bool CANDevice::sendMessage(const CANMessage& message) {
  if (!isOnline() || !m_transport) {
    return false;
  }

  // Encode CAN message based on transport protocol
  // USB-CAN adapters might use ASCII commands (e.g., "t1230112233445566\r")
  // SPI-CAN controllers use binary register writes
  // Native CAN uses SocketCAN frame format

  QByteArray encoded;
  // TODO: Implement encoding based on transport type
  // For now, simple example:
  QString cmd = QString("t%1%2%3\r")
                    .arg(message.id, 3, 16, QChar('0'))
                    .arg(message.data.size(), 1, 16)
                    .arg(QString::fromLatin1(message.data.toHex()));
  encoded = cmd.toLatin1();

  qint64 written = m_transport->write(encoded);
  return written == encoded.size();
}

bool CANDevice::setBitRate(quint32 bitRate) {
  m_bitRate = bitRate;
  m_config["bitRate"] = bitRate;

  if (!m_transport || !m_transport->isOpen()) {
    return true;  // Will be applied when transport opens
  }

  // Send bit rate configuration command based on transport
  // USB-CAN adapters typically use AT commands (e.g., "S6\r" for 500kbps)
  // SPI-CAN controllers need CNF register configuration
  // Native CAN uses netlink commands

  Logger::instance()->log(QString("CANDevice: Set bit rate to %1").arg(bitRate));
  return true;
}

quint32 CANDevice::getBitRate() const {
  return m_bitRate;
}

void CANDevice::onTransportDataReceived() {
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
  parseCANData(m_buffer);
}

void CANDevice::parseCANData(const QByteArray& data) {
  // Parse CAN frames based on transport protocol
  // USB-CAN adapters typically send ASCII frames (e.g., "t12301122334455\r\n")
  // SPI-CAN controllers provide binary data in registers
  // Native CAN provides SocketCAN frame structures

  // Example for ASCII protocol (Lawicel/SLCAN):
  int start = 0;
  int end = m_buffer.indexOf('\r', start);

  while (end != -1) {
    QByteArray frame = m_buffer.mid(start, end - start);
    
    if (frame.startsWith('t') || frame.startsWith('T')) {
      // Standard or extended CAN frame
      CANMessage message;
      message.extended = frame.startsWith('T');
      message.rtr = false;
      message.fd = false;
      message.timestamp = QDateTime::currentDateTime();

      // Parse: t123811223344556677
      //        ^id  ^len  ^data...
      QString frameStr = QString::fromLatin1(frame);
      bool ok;
      message.id = frameStr.mid(1, message.extended ? 8 : 3).toUInt(&ok, 16);
      quint8 len = frameStr.mid(message.extended ? 9 : 4, 1).toUInt(&ok, 16);
      QString dataStr = frameStr.mid(message.extended ? 10 : 5, len * 2);
      message.data = QByteArray::fromHex(dataStr.toLatin1());

      Logger::instance()->log(QString("CANDevice: Received message ID 0x%1").arg(message.id, 0, 16));
      emit messageReceived(message);
    }

    // Move to next frame
    start = end + 1;
    end = m_buffer.indexOf('\r', start);
  }

  // Remove processed data from buffer
  if (start > 0) {
    m_buffer.remove(0, start);
  }

  // Prevent buffer overflow
  if (m_buffer.size() > 4096) {
    Logger::instance()->log("CANDevice: Buffer overflow, clearing");
    m_buffer.clear();
  }
}
