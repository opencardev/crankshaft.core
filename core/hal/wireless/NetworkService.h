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
#include <memory>

/**
 * @brief Network connectivity service
 *
 * Manages network connectivity, routing, and DNS configuration.
 */
class NetworkService : public QObject {
  Q_OBJECT

 public:
  enum class NetworkType {
    UNKNOWN,
    NONE,
    ETHERNET,
    WIFI,
    CELLULAR,
    BLUETOOTH,
  };

  struct NetworkInterface {
    QString name;
    QString mac_address;
    QString ipv4_address;
    QString ipv6_address;
    NetworkType type;
    bool is_active;
    int mtu;
  };

  explicit NetworkService(QObject* parent = nullptr);
  ~NetworkService() override;

  /**
   * @brief Initialise the network service
   */
  virtual bool initialise() = 0;

  /**
   * @brief Deinitialise the network service
   */
  virtual void deinitialise() = 0;

  /**
   * @brief Get active network type
   */
  virtual NetworkType getActiveNetworkType() const = 0;

  /**
   * @brief Check if network is connected
   */
  virtual bool isConnected() const = 0;

  /**
   * @brief Get all network interfaces
   */
  virtual QVector<NetworkInterface> getNetworkInterfaces() const = 0;

  /**
   * @brief Get active network interface
   */
  virtual NetworkInterface getActiveInterface() const = 0;

  /**
   * @brief Get hostname
   */
  virtual QString getHostname() const = 0;

  /**
   * @brief Set hostname
   */
  virtual bool setHostname(const QString& hostname) = 0;

  /**
   * @brief Get DNS servers
   */
  virtual QVector<QString> getDNSServers() const = 0;

  /**
   * @brief Set DNS servers
   */
  virtual bool setDNSServers(const QVector<QString>& servers) = 0;

  /**
   * @brief Get gateway IP address
   */
  virtual QString getGateway() const = 0;

  /**
   * @brief Ping a host
   * @return Round-trip time in milliseconds, -1 on failure
   */
  virtual int ping(const QString& host) = 0;

  /**
   * @brief Get network latency (ping to gateway)
   */
  virtual int getLatency() const = 0;

 signals:
  /**
   * @brief Emitted when network connectivity changes
   */
  void connectivityChanged(bool connected);

  /**
   * @brief Emitted when active network type changes
   */
  void networkTypeChanged(NetworkType type);

  /**
   * @brief Emitted when IP address changes
   */
  void ipAddressChanged(const QString& address);

  /**
   * @brief Emitted when network error occurs
   */
  void errorOccurred(const QString& error);
};

using NetworkServicePtr = std::shared_ptr<NetworkService>;
