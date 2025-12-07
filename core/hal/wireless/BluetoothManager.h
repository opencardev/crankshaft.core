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

#include <QString>
#include <QObject>
#include <QVector>
#include <QBluetoothAddress>
#include <memory>

/**
 * @brief Bluetooth device manager
 *
 * Manages Bluetooth connectivity, device discovery, and pairing.
 * Uses BlueZ backend on Linux.
 */
class BluetoothManager : public QObject {
  Q_OBJECT

 public:
  enum class ConnectionState {
    UNKNOWN,
    OFF,
    TURNING_ON,
    ON,
    TURNING_OFF,
  };

  enum class DeviceType {
    UNKNOWN,
    COMPUTER,
    PHONE,
    AUDIO,
    PERIPHERAL,
    IMAGING,
  };

  enum class AudioProfile {
    NONE,
    A2DP,       // Audio streaming
    HFP,        // Hands-free profile
    HSP,        // Headset profile
    AVRCP,      // Remote control
  };

  struct BluetoothDevice {
    QString name;
    QString address;
    DeviceType type;
    int rssi;  // Signal strength
    bool paired;
    bool connected;
    QVector<AudioProfile> supported_profiles;
  };

  explicit BluetoothManager(QObject* parent = nullptr);
  ~BluetoothManager() override;

  /**
   * @brief Initialise the Bluetooth manager
   */
  virtual bool initialise() = 0;

  /**
   * @brief Deinitialise the Bluetooth manager
   */
  virtual void deinitialise() = 0;

  /**
   * @brief Check if Bluetooth is enabled
   */
  virtual bool isEnabled() const = 0;

  /**
   * @brief Enable/disable Bluetooth
   */
  virtual bool setEnabled(bool enabled) = 0;

  /**
   * @brief Get current Bluetooth state
   */
  virtual ConnectionState getConnectionState() const = 0;

  /**
   * @brief Get local Bluetooth address
   */
  virtual QString getLocalAddress() const = 0;

  /**
   * @brief Get local device name
   */
  virtual QString getLocalName() const = 0;

  /**
   * @brief Start discovering devices
   */
  virtual bool startDiscovery() = 0;

  /**
   * @brief Stop discovering devices
   */
  virtual bool stopDiscovery() = 0;

  /**
   * @brief Check if discovery is active
   */
  virtual bool isDiscovering() const = 0;

  /**
   * @brief Get discovered devices
   */
  virtual QVector<BluetoothDevice> getDiscoveredDevices() const = 0;

  /**
   * @brief Get paired devices
   */
  virtual QVector<BluetoothDevice> getPairedDevices() const = 0;

  /**
   * @brief Pair with a device
   */
  virtual bool pair(const QString& address) = 0;

  /**
   * @brief Unpair from a device
   */
  virtual bool unpair(const QString& address) = 0;

  /**
   * @brief Connect to a device
   */
  virtual bool connect(const QString& address) = 0;

  /**
   * @brief Disconnect from a device
   */
  virtual bool disconnect(const QString& address) = 0;

  /**
   * @brief Get connected devices
   */
  virtual QVector<BluetoothDevice> getConnectedDevices() const = 0;

  /**
   * @brief Connect audio profile on device
   */
  virtual bool connectAudio(const QString& address,
                           AudioProfile profile) = 0;

  /**
   * @brief Disconnect audio profile on device
   */
  virtual bool disconnectAudio(const QString& address,
                              AudioProfile profile) = 0;

 signals:
  /**
   * @brief Emitted when Bluetooth state changes
   */
  void stateChanged(ConnectionState state);

  /**
   * @brief Emitted when Bluetooth is enabled/disabled
   */
  void enabledChanged(bool enabled);

  /**
   * @brief Emitted when devices are discovered
   */
  void devicesDiscovered(const QVector<BluetoothDevice>& devices);

  /**
   * @brief Emitted when a device is discovered
   */
  void deviceDiscovered(const BluetoothDevice& device);

  /**
   * @brief Emitted when a device is paired
   */
  void devicePaired(const QString& address);

  /**
   * @brief Emitted when a device is unpaired
   */
  void deviceUnpaired(const QString& address);

  /**
   * @brief Emitted when a device is connected
   */
  void deviceConnected(const QString& address);

  /**
   * @brief Emitted when a device is disconnected
   */
  void deviceDisconnected(const QString& address);

  /**
   * @brief Emitted when audio profile is connected
   */
  void audioConnected(const QString& address, AudioProfile profile);

  /**
   * @brief Emitted when audio profile is disconnected
   */
  void audioDisconnected(const QString& address, AudioProfile profile);

  /**
   * @brief Emitted when Bluetooth error occurs
   */
  void errorOccurred(const QString& error);
};

using BluetoothManagerPtr = std::shared_ptr<BluetoothManager>;
