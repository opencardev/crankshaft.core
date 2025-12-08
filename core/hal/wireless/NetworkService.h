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

#include "WiFiHAL.h"
#include "BluetoothHAL.h"
#include <QString>
#include <QObject>
#include <QVector>
#include <memory>

/**
 * @brief Network connectivity service
 *
 * Manages WiFi and Bluetooth connectivity, providing a unified interface
 * for network operations.
 */
class NetworkService : public QObject {
  Q_OBJECT

 public:
  explicit NetworkService(QObject* parent = nullptr);
  ~NetworkService() override;

  /**
   * @brief Get WiFi HAL instance
   */
  WiFiHAL* wifiHAL() const;

  /**
   * @brief Get Bluetooth HAL instance
   */
  BluetoothHAL* bluetoothHAL() const;

  /**
   * @brief Check if system is online (has network connectivity)
   */
  bool isOnline() const;

  /**
   * @brief Get current connection type
   */
  QString getConnectionType() const;

 signals:
  /**
   * @brief Emitted when overall connectivity changes
   */
  void connectivityChanged();

 private slots:
  void onWiFiEnabledChanged(bool enabled);
  void onWiFiScanningChanged(bool scanning);
  void onWiFiNetworksUpdated(const QList<WiFiNetwork>& networks);
  void onWiFiConnectionStateChanged(WiFiHAL::ConnectionState state);

  void onBluetoothEnabledChanged(bool enabled);
  void onBluetoothDiscoveryStateChanged(bool discovering);
  void onBluetoothDevicesUpdated(const QList<BluetoothDevice>& devices);
  void onBluetoothDevicePaired(const QString& address);
  void onBluetoothDeviceConnected(const QString& address);
  void onBluetoothDeviceDisconnected(const QString& address);
  void onBluetoothPairingFailed(const QString& address, const QString& error);
  void onBluetoothConnectionFailed(const QString& address, const QString& error);
  void onBluetoothError(const QString& error);

 private:
  WiFiHAL* m_wifiHAL;
  BluetoothHAL* m_bluetoothHAL;
};

using NetworkServicePtr = std::shared_ptr<NetworkService>;
