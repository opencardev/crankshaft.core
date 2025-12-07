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

#include "../wireless/NetworkService.h"
#include <QDebug>

// Placeholder implementation of NetworkService

class NetworkServiceImpl : public NetworkService {
 public:
  explicit NetworkServiceImpl(QObject* parent = nullptr)
      : NetworkService(parent) {}

  bool initialise() override {
    qDebug() << "[Network] Initialising NetworkService";
    return true;
  }

  void deinitialise() override {
    qDebug() << "[Network] Deinitialising NetworkService";
  }

  NetworkType getActiveNetworkType() const override {
    return NetworkType::WIFI;
  }

  bool isConnected() const override { return true; }

  QVector<NetworkInterface> getNetworkInterfaces() const override {
    return {
        {"eth0", "00:11:22:33:44:55", "192.168.1.100", "",
         NetworkType::ETHERNET, true, 1500},
        {"wlan0", "AA:BB:CC:DD:EE:FF", "192.168.1.101", "",
         NetworkType::WIFI, true, 1500},
    };
  }

  NetworkInterface getActiveInterface() const override {
    return {"wlan0", "AA:BB:CC:DD:EE:FF", "192.168.1.101", "",
            NetworkType::WIFI, true, 1500};
  }

  QString getHostname() const override {
    return "crankshaft";
  }

  bool setHostname(const QString& hostname) override {
    return true;
  }

  QVector<QString> getDNSServers() const override {
    return {"8.8.8.8", "8.8.4.4"};
  }

  bool setDNSServers(const QVector<QString>& servers) override {
    return true;
  }

  QString getGateway() const override {
    return "192.168.1.1";
  }

  int ping(const QString& host) override {
    return 25;
  }

  int getLatency() const override {
    return 25;
  }
};
