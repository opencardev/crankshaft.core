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
#include <memory>

// Forward declarations
class ProfileManager;
class AndroidAutoService;
class WiFiManager;
class BluetoothManager;
class MediaPipeline;
struct DeviceConfig;

/**
 * @brief Service lifecycle manager
 *
 * Manages starting, stopping, and reloading services based on
 * ProfileManager device configurations. Allows dynamic service
 * management from UI without requiring application restart.
 */
class ServiceManager : public QObject {
  Q_OBJECT

 public:
  explicit ServiceManager(ProfileManager* profileManager, QObject* parent = nullptr);
  ~ServiceManager() override;

  /**
   * @brief Start all services based on active profile
   * @return true if at least one service started successfully
   */
  bool startAllServices();

  /**
   * @brief Stop all running services
   */
  void stopAllServices();

  /**
   * @brief Reload services based on current active profile
   * Stops all services and restarts based on updated configuration
   */
  void reloadServices();

  /**
   * @brief Start a specific service by device name
   * @param deviceName Device name from profile (e.g., "AndroidAuto", "WiFi")
   * @return true if service started successfully
   */
  bool startService(const QString& deviceName);

  /**
   * @brief Stop a specific service by device name
   * @param deviceName Device name from profile
   * @return true if service stopped successfully
   */
  bool stopService(const QString& deviceName);

  /**
   * @brief Restart a specific service
   * @param deviceName Device name from profile
   * @return true if service restarted successfully
   */
  bool restartService(const QString& deviceName);

  /**
   * @brief Check if a service is currently running
   * @param deviceName Device name from profile
   * @return true if service is running
   */
  bool isServiceRunning(const QString& deviceName) const;

  /**
   * @brief Get list of running service names
   */
  QStringList getRunningServices() const;

  // Service instance getters (for external access if needed)
  AndroidAutoService* getAndroidAutoService() const {
    return m_androidAutoService;
  }
  WiFiManager* getWiFiManager() const {
    return m_wifiManager;
  }
  BluetoothManager* getBluetoothManager() const {
    return m_bluetoothManager;
  }
  MediaPipeline* getMediaPipeline() const {
    return m_mediaPipeline;
  }

 signals:
  /**
   * @brief Emitted when services are reloaded
   */
  void servicesReloaded();

  /**
   * @brief Emitted when a service starts
   * @param deviceName Device name
   * @param success Whether startup was successful
   */
  void serviceStarted(const QString& deviceName, bool success);

  /**
   * @brief Emitted when a service stops
   * @param deviceName Device name
   */
  void serviceStopped(const QString& deviceName);

 public slots:
  /**
   * @brief Slot to handle profile changes
   * Automatically reloads services when active profile changes
   */
  void onProfileChanged(const QString& profileId);

  /**
   * @brief Slot to handle device config changes
   * Restarts affected service when device config changes
   */
  void onDeviceConfigChanged(const QString& profileId, const QString& deviceName);

 private:
  bool startAndroidAutoService(const DeviceConfig& device);
  bool startWiFiService(const DeviceConfig& device);
  bool startBluetoothService(const DeviceConfig& device);

  void stopAndroidAutoService();
  void stopWiFiService();
  void stopBluetoothService();
  void stopMediaPipeline();

  ProfileManager* m_profileManager;

  // Service instances
  AndroidAutoService* m_androidAutoService;
  WiFiManager* m_wifiManager;
  BluetoothManager* m_bluetoothManager;
  MediaPipeline* m_mediaPipeline;
};
