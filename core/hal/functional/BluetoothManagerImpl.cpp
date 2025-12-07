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

#include "../wireless/BluetoothManager.h"
#include <QDebug>

// Placeholder implementation of BluetoothManager

class BluetoothManagerImpl : public BluetoothManager {
 public:
  explicit BluetoothManagerImpl(QObject* parent = nullptr)
      : BluetoothManager(parent) {}

  bool initialise() override {
    qDebug() << "[Bluetooth] Initialising BluetoothManager";
    // TODO: Connect to BlueZ via DBus
    return true;
  }

  void deinitialise() override {
    qDebug() << "[Bluetooth] Deinitialising BluetoothManager";
  }

  bool isEnabled() const override { return true; }
  bool setEnabled(bool enabled) override { return true; }

  ConnectionState getConnectionState() const override {
    return ConnectionState::ON;
  }

  QString getLocalAddress() const override {
    return "00:11:22:33:44:55";
  }

  QString getLocalName() const override {
    return "Crankshaft";
  }

  bool startDiscovery() override {
    qDebug() << "[Bluetooth] Starting discovery";
    return true;
  }

  bool stopDiscovery() override {
    qDebug() << "[Bluetooth] Stopping discovery";
    return true;
  }

  bool isDiscovering() const override { return false; }

  QVector<BluetoothDevice> getDiscoveredDevices() const override {
    return {};
  }

  QVector<BluetoothDevice> getPairedDevices() const override {
    return {
        {"Phone", "AA:BB:CC:DD:EE:FF", DeviceType::PHONE, -50, true,
         true, {AudioProfile::A2DP, AudioProfile::HFP}},
    };
  }

  bool pair(const QString& address) override {
    qDebug() << "[Bluetooth] Pairing with" << address;
    return true;
  }

  bool unpair(const QString& address) override {
    qDebug() << "[Bluetooth] Unpairing from" << address;
    return true;
  }

  bool connect(const QString& address) override {
    qDebug() << "[Bluetooth] Connecting to" << address;
    emit deviceConnected(address);
    return true;
  }

  bool disconnect(const QString& address) override {
    qDebug() << "[Bluetooth] Disconnecting from" << address;
    emit deviceDisconnected(address);
    return true;
  }

  QVector<BluetoothDevice> getConnectedDevices() const override {
    return {
        {"Phone", "AA:BB:CC:DD:EE:FF", DeviceType::PHONE, -50, true,
         true, {AudioProfile::A2DP, AudioProfile::HFP}},
    };
  }

  bool connectAudio(const QString& address, AudioProfile profile) override {
    qDebug() << "[Bluetooth] Connecting audio profile" << static_cast<int>(profile)
             << "on" << address;
    emit audioConnected(address, profile);
    return true;
  }

  bool disconnectAudio(const QString& address,
                      AudioProfile profile) override {
    qDebug() << "[Bluetooth] Disconnecting audio profile"
             << static_cast<int>(profile) << "on" << address;
    emit audioDisconnected(address, profile);
    return true;
  }
};
