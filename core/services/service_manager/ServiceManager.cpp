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

#include "ServiceManager.h"

#include "../../hal/multimedia/MediaPipeline.h"
#include "../../hal/wireless/BluetoothManager.h"
#include "../../hal/wireless/WiFiManager.h"
#include "../android_auto/AndroidAutoService.h"
#include "../logging/Logger.h"
#include "../profile/ProfileManager.h"

ServiceManager::ServiceManager(ProfileManager* profileManager, QObject* parent)
    : QObject(parent),
      m_profileManager(profileManager),
      m_androidAutoService(nullptr),
      m_wifiManager(nullptr),
      m_bluetoothManager(nullptr),
      m_mediaPipeline(nullptr) {
  
  if (!m_profileManager) {
    Logger::instance().error("[ServiceManager] ProfileManager is null");
    return;
  }

  // Connect to profile change signals
  connect(m_profileManager, &ProfileManager::hostProfileChanged, this,
          &ServiceManager::onProfileChanged);
  connect(m_profileManager, &ProfileManager::deviceConfigChanged, this,
          &ServiceManager::onDeviceConfigChanged);

  Logger::instance().info("[ServiceManager] ServiceManager initialised");
}

ServiceManager::~ServiceManager() {
  Logger::instance().info("[ServiceManager] Shutting down ServiceManager");
  stopAllServices();
}

bool ServiceManager::startAllServices() {
  if (!m_profileManager) {
    Logger::instance().error("[ServiceManager] Cannot start services: ProfileManager is null");
    return false;
  }

  HostProfile activeProfile = m_profileManager->getActiveHostProfile();
  Logger::instance().info(
      QString("[ServiceManager] Starting services for profile: %1 (%2)")
          .arg(activeProfile.name, activeProfile.id));
  Logger::instance().info(
      QString("[ServiceManager] Profile has %1 device(s) configured").arg(activeProfile.devices.size()));

  bool anyStarted = false;
  int successCount = 0;
  int failCount = 0;

  for (const auto& device : activeProfile.devices) {
    Logger::instance().info(
        QString("[ServiceManager] Processing device: %1 (type: %2, enabled: %3, useMock: %4)")
            .arg(device.name)
            .arg(device.type)
            .arg(device.enabled ? "true" : "false")
            .arg(device.useMock ? "true" : "false"));

    if (!device.enabled) {
      Logger::instance().info(
          QString("[ServiceManager]   → Skipping disabled device: %1").arg(device.name));
      continue;
    }

    bool started = false;
    if (device.type == "AndroidAuto" || device.name == "AndroidAuto") {
      started = startAndroidAutoService(device);
    } else if (device.type == "WiFi" || device.name == "WiFi") {
      started = startWiFiService(device);
    } else if (device.type == "Bluetooth" || device.name == "Bluetooth") {
      started = startBluetoothService(device);
    } else {
      Logger::instance().warning(
          QString("[ServiceManager]   → Unknown device type: %1 (skipping)").arg(device.type));
      continue;
    }

    if (started) {
      successCount++;
      anyStarted = true;
      emit serviceStarted(device.name, true);
    } else {
      failCount++;
      emit serviceStarted(device.name, false);
    }
  }

  Logger::instance().info(
      QString("[ServiceManager] Service startup complete: %1 started, %2 failed")
          .arg(successCount)
          .arg(failCount));
  Logger::instance().info(
      QString("[ServiceManager] Services running: AndroidAuto=%1, WiFi=%2, Bluetooth=%3")
          .arg(m_androidAutoService ? "yes" : "no")
          .arg(m_wifiManager ? "yes" : "no")
          .arg(m_bluetoothManager ? "yes" : "no"));

  return anyStarted;
}

void ServiceManager::stopAllServices() {
  Logger::instance().info("[ServiceManager] Stopping all services...");

  stopAndroidAutoService();
  stopWiFiService();
  stopBluetoothService();
  stopMediaPipeline();

  Logger::instance().info("[ServiceManager] All services stopped");
}

void ServiceManager::reloadServices() {
  Logger::instance().info("[ServiceManager] Reloading services from profile configuration...");
  
  stopAllServices();
  startAllServices();
  
  emit servicesReloaded();
  Logger::instance().info("[ServiceManager] Services reloaded successfully");
}

bool ServiceManager::startService(const QString& deviceName) {
  if (!m_profileManager) {
    Logger::instance().error(
        QString("[ServiceManager] Cannot start service '%1': ProfileManager is null")
            .arg(deviceName));
    return false;
  }

  HostProfile activeProfile = m_profileManager->getActiveHostProfile();
  
  for (const auto& device : activeProfile.devices) {
    if (device.name == deviceName || device.type == deviceName) {
      if (!device.enabled) {
        Logger::instance().warning(
            QString("[ServiceManager] Cannot start disabled service: %1").arg(deviceName));
        return false;
      }

      Logger::instance().info(QString("[ServiceManager] Starting service: %1").arg(deviceName));

      bool started = false;
      if (device.type == "AndroidAuto" || device.name == "AndroidAuto") {
        started = startAndroidAutoService(device);
      } else if (device.type == "WiFi" || device.name == "WiFi") {
        started = startWiFiService(device);
      } else if (device.type == "Bluetooth" || device.name == "Bluetooth") {
        started = startBluetoothService(device);
      }

      emit serviceStarted(deviceName, started);
      return started;
    }
  }

  Logger::instance().error(
      QString("[ServiceManager] Device not found in profile: %1").arg(deviceName));
  return false;
}

bool ServiceManager::stopService(const QString& deviceName) {
  Logger::instance().info(QString("[ServiceManager] Stopping service: %1").arg(deviceName));

  if (deviceName == "AndroidAuto") {
    stopAndroidAutoService();
  } else if (deviceName == "WiFi") {
    stopWiFiService();
  } else if (deviceName == "Bluetooth") {
    stopBluetoothService();
  } else {
    Logger::instance().warning(
        QString("[ServiceManager] Unknown service name: %1").arg(deviceName));
    return false;
  }

  emit serviceStopped(deviceName);
  return true;
}

bool ServiceManager::restartService(const QString& deviceName) {
  Logger::instance().info(QString("[ServiceManager] Restarting service: %1").arg(deviceName));
  
  stopService(deviceName);
  return startService(deviceName);
}

bool ServiceManager::isServiceRunning(const QString& deviceName) const {
  if (deviceName == "AndroidAuto") {
    return m_androidAutoService != nullptr;
  } else if (deviceName == "WiFi") {
    return m_wifiManager != nullptr;
  } else if (deviceName == "Bluetooth") {
    return m_bluetoothManager != nullptr;
  }
  return false;
}

QStringList ServiceManager::getRunningServices() const {
  QStringList running;
  if (m_androidAutoService) running.append("AndroidAuto");
  if (m_wifiManager) running.append("WiFi");
  if (m_bluetoothManager) running.append("Bluetooth");
  return running;
}

void ServiceManager::onProfileChanged(const QString& profileId) {
  Logger::instance().info(
      QString("[ServiceManager] Active profile changed to: %1, reloading services...").arg(profileId));
  reloadServices();
}

void ServiceManager::onDeviceConfigChanged(const QString& profileId, const QString& deviceName) {
  // Only reload if this is the active profile
  HostProfile activeProfile = m_profileManager->getActiveHostProfile();
  if (activeProfile.id == profileId) {
    Logger::instance().info(
        QString("[ServiceManager] Device config changed: %1, restarting service...").arg(deviceName));
    restartService(deviceName);
  }
}

bool ServiceManager::startAndroidAutoService(const DeviceConfig& device) {
  Logger::instance().info(
      QString("[ServiceManager]   → Starting AndroidAuto service (%1)")
          .arg(device.useMock ? "Mock" : "Real/AASDK"));

  // Stop existing instance if running
  if (m_androidAutoService) {
    Logger::instance().info("[ServiceManager]      Stopping existing AndroidAuto instance");
    stopAndroidAutoService();
  }

  // Create MediaPipeline if needed for Real service
  if (!device.useMock && !m_mediaPipeline) {
    Logger::instance().info("[ServiceManager]      Creating MediaPipeline for Real AndroidAuto");
    m_mediaPipeline = new MediaPipeline(this);
    if (!m_mediaPipeline->initialise()) {
      Logger::instance().error("[ServiceManager]      Failed to initialise MediaPipeline");
      delete m_mediaPipeline;
      m_mediaPipeline = nullptr;
      return false;
    }
    Logger::instance().info("[ServiceManager]      MediaPipeline initialised successfully");
  }

  m_androidAutoService = AndroidAutoService::create(m_mediaPipeline, m_profileManager, this);
  if (!m_androidAutoService) {
    Logger::instance().error("[ServiceManager]      Failed to create AndroidAuto service instance");
    return false;
  }

  // Configure transport settings (USB/Wireless mode)
  m_androidAutoService->configureTransport(device.settings);

  if (!m_androidAutoService->initialise()) {
    Logger::instance().error("[ServiceManager]      Failed to initialise AndroidAuto service");
    delete m_androidAutoService;
    m_androidAutoService = nullptr;
    return false;
  }

  Logger::instance().info(
      QString("[ServiceManager]      AndroidAuto service started successfully (%1)")
          .arg(device.useMock ? "Mock mode" : "Real mode"));

  // Log channel configuration if available
  if (device.settings.contains("channels.video")) {
    Logger::instance().info(
        QString("[ServiceManager]      Channel config: video=%1, mediaAudio=%2, input=%3")
            .arg(device.settings["channels.video"].toBool() ? "enabled" : "disabled")
            .arg(device.settings["channels.mediaAudio"].toBool() ? "enabled" : "disabled")
            .arg(device.settings["channels.input"].toBool() ? "enabled" : "disabled"));
  }

  return true;
}

bool ServiceManager::startWiFiService(const DeviceConfig& device) {
  Logger::instance().info(
      QString("[ServiceManager]   → Starting WiFi service (%1)")
          .arg(device.useMock ? "Mock" : "Real"));

  // Stop existing instance if running
  if (m_wifiManager) {
    Logger::instance().info("[ServiceManager]      Stopping existing WiFi instance");
    stopWiFiService();
  }

  m_wifiManager = new WiFiManager(this);
  if (!m_wifiManager->initialise()) {
    Logger::instance().error("[ServiceManager]      Failed to initialise WiFi service");
    delete m_wifiManager;
    m_wifiManager = nullptr;
    return false;
  }

  Logger::instance().info("[ServiceManager]      WiFi service started successfully");

  // Apply settings if available
  if (device.settings.contains("autoConnect")) {
    bool autoConnect = device.settings["autoConnect"].toBool();
    Logger::instance().info(
        QString("[ServiceManager]      Auto-connect: %1").arg(autoConnect ? "enabled" : "disabled"));
  }

  return true;
}

bool ServiceManager::startBluetoothService(const DeviceConfig& device) {
  Logger::instance().info(
      QString("[ServiceManager]   → Starting Bluetooth service (%1)")
          .arg(device.useMock ? "Mock" : "Real"));

  // Stop existing instance if running
  if (m_bluetoothManager) {
    Logger::instance().info("[ServiceManager]      Stopping existing Bluetooth instance");
    stopBluetoothService();
  }

  m_bluetoothManager = new BluetoothManager(this);
  if (!m_bluetoothManager->initialise()) {
    Logger::instance().error("[ServiceManager]      Failed to initialise Bluetooth service");
    delete m_bluetoothManager;
    m_bluetoothManager = nullptr;
    return false;
  }

  Logger::instance().info("[ServiceManager]      Bluetooth service started successfully");

  // Apply settings if available
  if (device.settings.contains("autoDiscovery")) {
    bool autoDiscovery = device.settings["autoDiscovery"].toBool();
    Logger::instance().info(
        QString("[ServiceManager]      Auto-discovery: %1")
            .arg(autoDiscovery ? "enabled" : "disabled"));
  }

  return true;
}

void ServiceManager::stopAndroidAutoService() {
  if (m_androidAutoService) {
    Logger::instance().info("[ServiceManager] Stopping AndroidAuto service");
    m_androidAutoService->deinitialise();
    delete m_androidAutoService;
    m_androidAutoService = nullptr;
    Logger::instance().info("[ServiceManager] AndroidAuto service stopped");
  }
}

void ServiceManager::stopWiFiService() {
  if (m_wifiManager) {
    Logger::instance().info("[ServiceManager] Stopping WiFi service");
    m_wifiManager->deinitialise();
    delete m_wifiManager;
    m_wifiManager = nullptr;
    Logger::instance().info("[ServiceManager] WiFi service stopped");
  }
}

void ServiceManager::stopBluetoothService() {
  if (m_bluetoothManager) {
    Logger::instance().info("[ServiceManager] Stopping Bluetooth service");
    m_bluetoothManager->deinitialise();
    delete m_bluetoothManager;
    m_bluetoothManager = nullptr;
    Logger::instance().info("[ServiceManager] Bluetooth service stopped");
  }
}

void ServiceManager::stopMediaPipeline() {
  if (m_mediaPipeline) {
    Logger::instance().info("[ServiceManager] Stopping MediaPipeline");
    m_mediaPipeline->deinitialise();
    delete m_mediaPipeline;
    m_mediaPipeline = nullptr;
    Logger::instance().info("[ServiceManager] MediaPipeline stopped");
  }
}
