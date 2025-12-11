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

#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>

/**
 * @brief WiFi security types
 */
enum class WiFiSecurity { Open, WEP, WPA_PSK, WPA2_PSK, WPA3_SAE };

/**
 * @brief WiFi network information
 */
struct WiFiNetwork {
  QString ssid;
  QString bssid;
  int signalStrength;  // -100 to 0 dBm
  int quality;         // 0-100%
  int frequency;       // MHz
  WiFiSecurity security;
  bool connected;
  bool known;
};

/**
 * @brief Hardware Abstraction Layer for WiFi
 *
 * Provides WiFi scanning, connection management via NetworkManager DBus interface.
 */
class WiFiHAL : public QObject {
  Q_OBJECT

 public:
  enum class ConnectionState { Unknown, Disconnected, Connecting, Connected, Failed };
  Q_ENUM(ConnectionState)

  explicit WiFiHAL(QObject* parent = nullptr);
  ~WiFiHAL() override;

  bool isEnabled() const;
  bool setEnabled(bool enabled);

  bool isScanning() const;
  bool startScan();
  QList<WiFiNetwork> getAvailableNetworks() const;

  ConnectionState getConnectionState() const;
  QString getConnectedSSID() const;
  int getSignalStrength() const;
  QString getIPAddress() const;

  bool connectToNetwork(const QString& ssid, const QString& password, WiFiSecurity security);
  bool disconnect();
  bool forgetNetwork(const QString& ssid);
  QStringList getSavedNetworks() const;

 signals:
  void enabledChanged(bool enabled);
  void scanningChanged(bool scanning);
  void networksUpdated(const QList<WiFiNetwork>& networks);
  void connectionStateChanged(ConnectionState state);
  void signalStrengthChanged(int strength);
  void errorOccurred(const QString& error);

 private slots:
  void updateNetworkList();
  void onDevicePropertiesChanged(const QString& interface, const QVariantMap& changedProperties,
                                 const QStringList& invalidatedProperties);

 private:
  class WiFiHALPrivate;
  WiFiHALPrivate* d;
};
