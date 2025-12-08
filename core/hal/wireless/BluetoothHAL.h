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

#include <QObject>
#include <QString>
#include <QList>
#include <QStringList>
#include <QDBusObjectPath>
#include <QVariantMap>

/**
 * @brief Bluetooth device information
 */
struct BluetoothDevice {
    enum class DeviceType {
        Unknown,
        Computer,
        Phone,
        Audio,
        AudioAndInput,
        Input,
        Peripheral,
        Imaging
    };

    enum class AudioProfile {
        None,
        A2DP,    // Audio streaming
        HFP,     // Hands-free profile
        HSP,     // Headset profile
        AVRCP    // Remote control
    };

    QString path;
    QString name;
    QString address;
    DeviceType type;
    int rssi;  // Signal strength in dBm
    int signalStrength;  // Signal strength as percentage 0-100
    bool paired;
    bool connected;
    bool trusted;
    QList<AudioProfile> supportedProfiles;
};

/**
 * @brief Hardware Abstraction Layer for Bluetooth
 * 
 * Provides Bluetooth device discovery, pairing, and connection via BlueZ DBus interface.
 */
class BluetoothHAL : public QObject {
  Q_OBJECT

 public:
  enum class State {
    Off,
    TurningOn,
    On,
    TurningOff
  };
  Q_ENUM(State)

  explicit BluetoothHAL(QObject* parent = nullptr);
  ~BluetoothHAL() override;

  bool isEnabled() const;
  bool setEnabled(bool enabled);
  
  State getState() const;
  QString getLocalAddress() const;
  QString getLocalName() const;
  bool setLocalName(const QString& name);
  
  bool isDiscovering() const;
  bool startDiscovery();
  bool stopDiscovery();
  QList<BluetoothDevice> getDevices() const;
  QList<BluetoothDevice> getPairedDevices() const;
  
  bool pairDevice(const QString& address);
  bool connectDevice(const QString& address);
  bool disconnectDevice(const QString& address);
  bool removeDevice(const QString& address);
  
  QString getConnectedDevice() const;
  bool isConnected(const QString& address) const;

 signals:
  void enabledChanged(bool enabled);
  void stateChanged(State state);
  void discoveryStateChanged(bool discovering);
  void devicesUpdated(const QList<BluetoothDevice>& devices);
  void deviceDiscovered(const BluetoothDevice& device);
  void devicePaired(const QString& address);
  void deviceConnected(const QString& address);
  void deviceDisconnected(const QString& address);
  void pairingFailed(const QString& address, const QString& error);
  void connectionFailed(const QString& address, const QString& error);
  void errorOccurred(const QString& error);

 private slots:
  void updateDeviceList();
  void onInterfacesAdded(const QDBusObjectPath& objectPath, const QMap<QString, QVariantMap>& interfaces);
  void onInterfacesRemoved(const QDBusObjectPath& objectPath, const QStringList& interfaces);

 private:
  QString findDevicePath(const QString& deviceAddress) const;
  
  class BluetoothHALPrivate;
  BluetoothHALPrivate* d;
};
