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

#include "UARTTransport.h"

UARTTransport::UARTTransport(const QString& portName, QObject* parent)
    : Transport(parent), m_portName(portName), m_state(TransportState::DISCONNECTED) {
  // Set default configuration
  m_config["port"] = portName;
  m_config["baudRate"] = 9600;
  m_config["dataBits"] = 8;
  m_config["parity"] = "none";
  m_config["stopBits"] = 1;
  m_config["flowControl"] = "none";
}

UARTTransport::~UARTTransport() {
  close();
}

QString UARTTransport::getName() const {
  return QString("UART(%1)").arg(m_portName);
}

bool UARTTransport::open() {
  if (m_state == TransportState::CONNECTED) {
    return true;
  }

  m_state = TransportState::CONNECTING;
  emit stateChanged(m_state);

  // TODO: Implement actual serial port opening
  // This would use platform-specific APIs or Qt SerialPort

  m_state = TransportState::CONNECTED;
  emit stateChanged(m_state);
  emit connected();
  return true;
}

void UARTTransport::close() {
  if (m_state == TransportState::DISCONNECTED) {
    return;
  }

  // TODO: Implement actual serial port closing

  m_state = TransportState::DISCONNECTED;
  emit stateChanged(m_state);
  emit disconnected();
}

bool UARTTransport::isOpen() const {
  return m_state == TransportState::CONNECTED;
}

TransportState UARTTransport::getState() const {
  return m_state;
}

qint64 UARTTransport::write(const QByteArray& data) {
  if (!isOpen()) {
    return -1;
  }

  // TODO: Implement actual serial write
  return data.size();
}

QByteArray UARTTransport::read(qint64 maxSize) {
  if (!isOpen()) {
    return QByteArray();
  }

  // TODO: Implement actual serial read
  return QByteArray();
}

qint64 UARTTransport::bytesAvailable() const {
  if (!isOpen()) {
    return 0;
  }

  // TODO: Implement actual bytes available check
  return 0;
}

void UARTTransport::flush() {
  if (!isOpen()) {
    return;
  }

  // TODO: Implement actual flush
}

bool UARTTransport::configure(const QString& key, const QVariant& value) {
  m_config[key] = value;

  if (key == "baudRate") {
    return setBaudRate(value.toInt());
  } else if (key == "dataBits") {
    return setDataBits(value.toUInt());
  } else if (key == "parity") {
    return setParity(value.toString());
  } else if (key == "stopBits") {
    return setStopBits(value.toUInt());
  } else if (key == "flowControl") {
    return setFlowControl(value.toString());
  }

  return true;
}

QVariant UARTTransport::getConfiguration(const QString& key) const {
  return m_config.value(key);
}

bool UARTTransport::setBaudRate(qint32 baudRate) {
  m_config["baudRate"] = baudRate;
  // TODO: Apply to actual serial port
  return true;
}

bool UARTTransport::setDataBits(quint8 dataBits) {
  m_config["dataBits"] = dataBits;
  // TODO: Apply to actual serial port
  return true;
}

bool UARTTransport::setParity(const QString& parity) {
  m_config["parity"] = parity;
  // TODO: Apply to actual serial port
  return true;
}

bool UARTTransport::setStopBits(quint8 stopBits) {
  m_config["stopBits"] = stopBits;
  // TODO: Apply to actual serial port
  return true;
}

bool UARTTransport::setFlowControl(const QString& flowControl) {
  m_config["flowControl"] = flowControl;
  // TODO: Apply to actual serial port
  return true;
}
