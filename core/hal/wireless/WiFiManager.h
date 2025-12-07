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
#include <QJsonObject>
#include <memory>

/**
 * @brief WiFi network manager
 *
 * Manages WiFi connectivity, scanning, and connection to networks.
 * Uses NetworkManager or similar backend.
 */
class WiFiManager : public QObject {
  Q_OBJECT

 public:
  enum class ConnectionState {
    UNKNOWN,
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    RECONNECTING,
    FAILED,
  };

  enum class Security {
    NONE,
    WEP,
    WPA,
    WPA2,
    WPA3,
  };

  struct WiFiNetwork {
    QString ssid;
    QString bssid;
    int signal_strength;  // -100 to 0 dBm
    Security security_type;
    int frequency;  // MHz
    bool is_saved;
  };

  explicit WiFiManager(QObject* parent = nullptr);
  ~WiFiManager() override;

  /**
   * @brief Initialise the WiFi manager
   */
  virtual bool initialise() = 0;

  /**
   * @brief Deinitialise the WiFi manager
   */
  virtual void deinitialise() = 0;

  /**
   * @brief Check if WiFi is enabled
   */
  virtual bool isEnabled() const = 0;

  /**
   * @brief Enable/disable WiFi
   */
  virtual bool setEnabled(bool enabled) = 0;

  /**
   * @brief Get current connection state
   */
  virtual ConnectionState getConnectionState() const = 0;

  /**
   * @brief Get connected network SSID
   */
  virtual QString getConnectedSSID() const = 0;

  /**
   * @brief Start scanning for available networks
   */
  virtual bool startScan() = 0;

  /**
   * @brief Get available networks from last scan
   */
  virtual QVector<WiFiNetwork> getAvailableNetworks() const = 0;

  /**
   * @brief Connect to a WiFi network
   * @param ssid Network SSID
   * @param password Network password (empty for open networks)
   * @param security Security type
   */
  virtual bool connect(const QString& ssid, const QString& password,
                       Security security) = 0;

  /**
   * @brief Disconnect from current network
   */
  virtual bool disconnect() = 0;

  /**
   * @brief Forget a saved network
   */
  virtual bool forgetNetwork(const QString& ssid) = 0;

  /**
   * @brief Get saved networks
   */
  virtual QVector<QString> getSavedNetworks() const = 0;

  /**
   * @brief Get signal strength of connected network (0-100)
   */
  virtual int getSignalStrength() const = 0;

  /**
   * @brief Get IP address of current connection
   */
  virtual QString getIPAddress() const = 0;

 signals:
  /**
   * @brief Emitted when connection state changes
   */
  void connectionStateChanged(ConnectionState state);

  /**
   * @brief Emitted when WiFi is enabled/disabled
   */
  void enabledChanged(bool enabled);

  /**
   * @brief Emitted when available networks are updated
   */
  void networksUpdated(const QVector<WiFiNetwork>& networks);

  /**
   * @brief Emitted when signal strength changes
   */
  void signalStrengthChanged(int percent);

  /**
   * @brief Emitted when WiFi error occurs
   */
  void errorOccurred(const QString& error);
};

using WiFiManagerPtr = std::shared_ptr<WiFiManager>;
