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

#include "../wireless/WiFiManager.h"
#include <QDebug>

// Placeholder implementation of WiFiManager

class WiFiManagerImpl : public WiFiManager {
 public:
  explicit WiFiManagerImpl(QObject* parent = nullptr)
      : WiFiManager(parent) {}

  bool initialise() override {
    qDebug() << "[WiFi] Initialising WiFiManager";
    // TODO: Connect to NetworkManager via DBus
    return true;
  }

  void deinitialise() override {
    qDebug() << "[WiFi] Deinitialising WiFiManager";
  }

  bool isEnabled() const override { return true; }
  bool setEnabled(bool enabled) override { return true; }

  ConnectionState getConnectionState() const override {
    return ConnectionState::CONNECTED;
  }

  QString getConnectedSSID() const override {
    return "MyNetwork";
  }

  bool startScan() override { return true; }

  QVector<WiFiNetwork> getAvailableNetworks() const override {
    return {
        {"Network1", "", 80, Security::WPA2, 2400, false},
        {"Network2", "", 60, Security::WPA, 5200, false},
    };
  }

  bool connect(const QString& ssid, const QString& password,
               Security security) override {
    qDebug() << "[WiFi] Connecting to" << ssid;
    return true;
  }

  bool disconnect() override { return true; }

  bool forgetNetwork(const QString& ssid) override {
    return true;
  }

  QVector<QString> getSavedNetworks() const override {
    return {"MyNetwork"};
  }

  int getSignalStrength() const override { return 80; }

  QString getIPAddress() const override {
    return "192.168.1.100";
  }
};
