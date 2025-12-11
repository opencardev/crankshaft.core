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

#include "NetworkService.h"

#include <QDebug>

NetworkService::NetworkService(QObject* parent)
    : QObject(parent), m_wifiHAL(new WiFiHAL(this)), m_bluetoothHAL(new BluetoothHAL(this)) {
  // Connect WiFi HAL signals
  connect(m_wifiHAL, &WiFiHAL::enabledChanged, this, &NetworkService::onWiFiEnabledChanged);
  connect(m_wifiHAL, &WiFiHAL::scanningChanged, this, &NetworkService::onWiFiScanningChanged);
  connect(m_wifiHAL, &WiFiHAL::networksUpdated, this, &NetworkService::onWiFiNetworksUpdated);
  connect(m_wifiHAL, &WiFiHAL::connectionStateChanged, this,
          &NetworkService::onWiFiConnectionStateChanged);

  // Connect Bluetooth HAL signals
  connect(m_bluetoothHAL, &BluetoothHAL::enabledChanged, this,
          &NetworkService::onBluetoothEnabledChanged);
  connect(m_bluetoothHAL, &BluetoothHAL::discoveryStateChanged, this,
          &NetworkService::onBluetoothDiscoveryStateChanged);
  connect(m_bluetoothHAL, &BluetoothHAL::devicesUpdated, this,
          &NetworkService::onBluetoothDevicesUpdated);
  connect(m_bluetoothHAL, &BluetoothHAL::devicePaired, this,
          &NetworkService::onBluetoothDevicePaired);
  connect(m_bluetoothHAL, &BluetoothHAL::deviceConnected, this,
          &NetworkService::onBluetoothDeviceConnected);
  connect(m_bluetoothHAL, &BluetoothHAL::deviceDisconnected, this,
          &NetworkService::onBluetoothDeviceDisconnected);
  connect(m_bluetoothHAL, &BluetoothHAL::pairingFailed, this,
          &NetworkService::onBluetoothPairingFailed);
  connect(m_bluetoothHAL, &BluetoothHAL::connectionFailed, this,
          &NetworkService::onBluetoothConnectionFailed);
  connect(m_bluetoothHAL, &BluetoothHAL::errorOccurred, this, &NetworkService::onBluetoothError);

  qDebug() << "NetworkService initialized";
}

NetworkService::~NetworkService() {}

WiFiHAL* NetworkService::wifiHAL() const {
  return m_wifiHAL;
}

BluetoothHAL* NetworkService::bluetoothHAL() const {
  return m_bluetoothHAL;
}

bool NetworkService::isOnline() const {
  // Check if WiFi is connected
  if (m_wifiHAL->isEnabled() && !m_wifiHAL->getConnectedSSID().isEmpty()) {
    return true;
  }

  // Could also check ethernet, mobile data, etc.
  return false;
}

QString NetworkService::getConnectionType() const {
  if (m_wifiHAL->isEnabled() && !m_wifiHAL->getConnectedSSID().isEmpty()) {
    return "WiFi";
  }

  if (m_bluetoothHAL->isEnabled() && !m_bluetoothHAL->getConnectedDevice().isEmpty()) {
    return "Bluetooth";
  }

  return "None";
}

void NetworkService::onWiFiEnabledChanged(bool enabled) {
  qDebug() << "WiFi enabled changed:" << enabled;
  emit connectivityChanged();
}

void NetworkService::onWiFiScanningChanged(bool scanning) {
  qDebug() << "WiFi scanning changed:" << scanning;
}

void NetworkService::onWiFiNetworksUpdated(const QList<WiFiNetwork>& networks) {
  qDebug() << "WiFi networks updated, count:" << networks.size();
}

void NetworkService::onWiFiConnectionStateChanged(WiFiHAL::ConnectionState state) {
  qDebug() << "WiFi connection state changed:" << static_cast<int>(state);
  emit connectivityChanged();
}

void NetworkService::onBluetoothEnabledChanged(bool enabled) {
  qDebug() << "Bluetooth enabled changed:" << enabled;
}

void NetworkService::onBluetoothDiscoveryStateChanged(bool discovering) {
  qDebug() << "Bluetooth discovery changed:" << discovering;
}

void NetworkService::onBluetoothDevicesUpdated(const QList<BluetoothDevice>& devices) {
  qDebug() << "Bluetooth devices updated, count:" << devices.size();
}

void NetworkService::onBluetoothDevicePaired(const QString& address) {
  qDebug() << "Bluetooth device paired:" << address;
}

void NetworkService::onBluetoothDeviceConnected(const QString& address) {
  qDebug() << "Bluetooth device connected:" << address;
  emit connectivityChanged();
}

void NetworkService::onBluetoothDeviceDisconnected(const QString& address) {
  qDebug() << "Bluetooth device disconnected:" << address;
  emit connectivityChanged();
}

void NetworkService::onBluetoothPairingFailed(const QString& address, const QString& error) {
  qWarning() << "Bluetooth pairing failed for" << address << ":" << error;
}

void NetworkService::onBluetoothConnectionFailed(const QString& address, const QString& error) {
  qWarning() << "Bluetooth connection failed for" << address << ":" << error;
}

void NetworkService::onBluetoothError(const QString& error) {
  qCritical() << "Bluetooth error:" << error;
}
