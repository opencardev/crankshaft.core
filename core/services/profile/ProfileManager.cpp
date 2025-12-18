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

#include "ProfileManager.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUuid>

#include "../logging/Logger.h"

ProfileManager::ProfileManager(const QString& configDir, QObject* parent)
    : QObject(parent), m_configDir(configDir) {
  // Ensure config directory exists
  QDir dir(m_configDir);
  if (!dir.exists()) {
    dir.mkpath(".");
  }

  loadProfiles();

  // If no profiles exist, create defaults
  if (m_hostProfiles.isEmpty() && m_vehicleProfiles.isEmpty()) {
    initializeDefaultProfiles();
    saveProfiles();
  }
}

ProfileManager::~ProfileManager() {
  saveProfiles();
}

void ProfileManager::initializeDefaultProfiles() {
  Logger::instance().info("ProfileManager: Initialising default profiles...");

  // Create default development host profile
  HostProfile devHostProfile;
  devHostProfile.id = QUuid::createUuid().toString();
  devHostProfile.name = "Development Host";
  devHostProfile.description = "Default development host with all mock devices enabled";
  devHostProfile.isActive = true;
  devHostProfile.createdAt = QDateTime::currentDateTime();
  devHostProfile.modifiedAt = QDateTime::currentDateTime();
  devHostProfile.cpuModel = "Intel i7 (Development)";
  devHostProfile.ramMB = 16384;
  devHostProfile.osVersion = "Raspberry Pi OS (Simulated)";

  // Add default mock devices
  DeviceConfig androidAutoDevice;
  androidAutoDevice.name = "AndroidAuto";
  androidAutoDevice.type = "AndroidAuto";
  androidAutoDevice.enabled = true;
  androidAutoDevice.useMock = true;
  androidAutoDevice.description = "Android Auto projection service";
  androidAutoDevice.settings["resolution"] = "1024x600";
  androidAutoDevice.settings["fps"] = 30;
  androidAutoDevice.settings["generateTestVideo"] = true;
  androidAutoDevice.settings["generateTestAudio"] = false;
  // Channel configuration
  androidAutoDevice.settings["channels.video"] = true;
  androidAutoDevice.settings["channels.mediaAudio"] = true;
  androidAutoDevice.settings["channels.systemAudio"] = true;
  androidAutoDevice.settings["channels.speechAudio"] = true;
  androidAutoDevice.settings["channels.microphone"] = true;
  androidAutoDevice.settings["channels.input"] = true;
  androidAutoDevice.settings["channels.sensor"] = true;
  androidAutoDevice.settings["channels.bluetooth"] = false;  // Disabled by default

  // Connection mode: "auto", "usb", "wireless"
  // auto = try USB first, fallback to wireless if configured
  // usb = USB only
  // wireless = TCP/IP only (developer option for PC testing)
  androidAutoDevice.settings["connectionMode"] = "auto";
  androidAutoDevice.settings["wireless.enabled"] = false;
  androidAutoDevice.settings["wireless.host"] = "";    // e.g., "192.168.1.100" or "phone.local"
  androidAutoDevice.settings["wireless.port"] = 5277;  // Default Android Auto wireless port

  devHostProfile.devices.append(androidAutoDevice);

  DeviceConfig bluetoothDevice;
  bluetoothDevice.name = "Bluetooth";
  bluetoothDevice.type = "Bluetooth";
  bluetoothDevice.enabled = true;
  bluetoothDevice.useMock = true;
  bluetoothDevice.description = "Bluetooth connectivity";
  devHostProfile.devices.append(bluetoothDevice);

  DeviceConfig wifiDevice;
  wifiDevice.name = "WiFi";
  wifiDevice.type = "WiFi";
  wifiDevice.enabled = true;
  wifiDevice.useMock = true;
  wifiDevice.description = "WiFi connectivity";
  devHostProfile.devices.append(wifiDevice);

  m_hostProfiles[devHostProfile.id] = devHostProfile;
  m_activeHostProfileId = devHostProfile.id;

  // Create default test car profile
  VehicleProfile carProfile;
  carProfile.id = QUuid::createUuid().toString();
  carProfile.name = "Test Vehicle - Sedan";
  carProfile.description = "Generic test vehicle for development";
  carProfile.isActive = true;
  carProfile.createdAt = QDateTime::currentDateTime();
  carProfile.modifiedAt = QDateTime::currentDateTime();
  carProfile.make = "Test Make";
  carProfile.model = "Test Model";
  carProfile.year = "2025";
  carProfile.vin = "TEST000000000001";
  carProfile.licensePlate = "MOCK-001";
  carProfile.vehicleType = "car";
  carProfile.supportedModes = {"PARK", "REVERSE", "NEUTRAL", "DRIVE"};
  carProfile.hasAWD = true;
  carProfile.wheelCount = 4;

  m_vehicleProfiles[carProfile.id] = carProfile;
  m_activeVehicleProfileId = carProfile.id;

  Logger::instance().info("ProfileManager: Default profiles initialised successfully");
}

bool ProfileManager::createHostProfile(const HostProfile& profile) {
  HostProfile newProfile = profile;
  if (newProfile.id.isEmpty()) {
    newProfile.id = QUuid::createUuid().toString();
  }
  newProfile.createdAt = QDateTime::currentDateTime();
  newProfile.modifiedAt = QDateTime::currentDateTime();

  m_hostProfiles[newProfile.id] = newProfile;
  Logger::instance().info(
      QString("ProfileManager: Host profile created: %1 (%2)").arg(newProfile.name, newProfile.id));
  return saveProfiles();
}

bool ProfileManager::updateHostProfile(const HostProfile& profile) {
  if (!m_hostProfiles.contains(profile.id)) {
    Logger::instance().warning(
        QString("ProfileManager: Host profile not found: %1").arg(profile.id));
    return false;
  }

  HostProfile updatedProfile = profile;
  updatedProfile.modifiedAt = QDateTime::currentDateTime();
  m_hostProfiles[profile.id] = updatedProfile;

  Logger::instance().info(QString("ProfileManager: Host profile updated: %1").arg(profile.id));
  emit hostProfileChanged(profile.id);
  return saveProfiles();
}

bool ProfileManager::deleteHostProfile(const QString& profileId) {
  if (!m_hostProfiles.contains(profileId)) {
    Logger::instance().warning(
        QString("ProfileManager: Host profile not found: %1").arg(profileId));
    return false;
  }

  m_hostProfiles.remove(profileId);
  if (m_activeHostProfileId == profileId) {
    m_activeHostProfileId = m_hostProfiles.isEmpty() ? QString() : m_hostProfiles.firstKey();
  }

  Logger::instance().info(QString("ProfileManager: Host profile deleted: %1").arg(profileId));
  return saveProfiles();
}

HostProfile ProfileManager::getHostProfile(const QString& profileId) const {
  if (m_hostProfiles.contains(profileId)) {
    return m_hostProfiles.value(profileId);
  }
  return HostProfile();
}

QList<HostProfile> ProfileManager::getAllHostProfiles() const {
  return m_hostProfiles.values();
}

bool ProfileManager::setActiveHostProfile(const QString& profileId) {
  if (!m_hostProfiles.contains(profileId)) {
    Logger::instance().warning(
        QString("ProfileManager: Host profile not found: %1").arg(profileId));
    return false;
  }

  // Deactivate previous profile
  if (m_hostProfiles.contains(m_activeHostProfileId)) {
    HostProfile oldProfile = m_hostProfiles[m_activeHostProfileId];
    oldProfile.isActive = false;
    m_hostProfiles[m_activeHostProfileId] = oldProfile;
  }

  // Activate new profile
  HostProfile newProfile = m_hostProfiles[profileId];
  newProfile.isActive = true;
  m_hostProfiles[profileId] = newProfile;
  m_activeHostProfileId = profileId;

  Logger::instance().info(QString("ProfileManager: Active host profile changed to: %1 (%2)")
                              .arg(newProfile.name, profileId));
  emit hostProfileChanged(profileId);
  return saveProfiles();
}

HostProfile ProfileManager::getActiveHostProfile() const {
  if (m_hostProfiles.contains(m_activeHostProfileId)) {
    return m_hostProfiles.value(m_activeHostProfileId);
  }
  return HostProfile();
}

bool ProfileManager::createVehicleProfile(const VehicleProfile& profile) {
  VehicleProfile newProfile = profile;
  if (newProfile.id.isEmpty()) {
    newProfile.id = QUuid::createUuid().toString();
  }
  newProfile.createdAt = QDateTime::currentDateTime();
  newProfile.modifiedAt = QDateTime::currentDateTime();

  m_vehicleProfiles[newProfile.id] = newProfile;
  Logger::instance().info(QString("ProfileManager: Vehicle profile created: %1 (%2)")
                              .arg(newProfile.name, newProfile.id));
  return saveProfiles();
}

bool ProfileManager::updateVehicleProfile(const VehicleProfile& profile) {
  if (!m_vehicleProfiles.contains(profile.id)) {
    Logger::instance().warning(
        QString("ProfileManager: Vehicle profile not found: %1").arg(profile.id));
    return false;
  }

  VehicleProfile updatedProfile = profile;
  updatedProfile.modifiedAt = QDateTime::currentDateTime();
  m_vehicleProfiles[profile.id] = updatedProfile;

  Logger::instance().info(QString("ProfileManager: Vehicle profile updated: %1").arg(profile.id));
  emit vehicleProfileChanged(profile.id);
  return saveProfiles();
}

bool ProfileManager::deleteVehicleProfile(const QString& profileId) {
  if (!m_vehicleProfiles.contains(profileId)) {
    Logger::instance().warning(
        QString("ProfileManager: Vehicle profile not found: %1").arg(profileId));
    return false;
  }

  m_vehicleProfiles.remove(profileId);
  if (m_activeVehicleProfileId == profileId) {
    m_activeVehicleProfileId =
        m_vehicleProfiles.isEmpty() ? QString() : m_vehicleProfiles.firstKey();
  }

  Logger::instance().info(QString("ProfileManager: Vehicle profile deleted: %1").arg(profileId));
  return saveProfiles();
}

VehicleProfile ProfileManager::getVehicleProfile(const QString& profileId) const {
  if (m_vehicleProfiles.contains(profileId)) {
    return m_vehicleProfiles.value(profileId);
  }
  return VehicleProfile();
}

QList<VehicleProfile> ProfileManager::getAllVehicleProfiles() const {
  return m_vehicleProfiles.values();
}

bool ProfileManager::setActiveVehicleProfile(const QString& profileId) {
  if (!m_vehicleProfiles.contains(profileId)) {
    Logger::instance().warning(
        QString("ProfileManager: Vehicle profile not found: %1").arg(profileId));
    return false;
  }

  // Deactivate previous profile
  if (m_vehicleProfiles.contains(m_activeVehicleProfileId)) {
    VehicleProfile oldProfile = m_vehicleProfiles[m_activeVehicleProfileId];
    oldProfile.isActive = false;
    m_vehicleProfiles[m_activeVehicleProfileId] = oldProfile;
  }

  // Activate new profile
  VehicleProfile newProfile = m_vehicleProfiles[profileId];
  newProfile.isActive = true;
  m_vehicleProfiles[profileId] = newProfile;
  m_activeVehicleProfileId = profileId;

  Logger::instance().info(QString("ProfileManager: Active vehicle profile changed to: %1 (%2)")
                              .arg(newProfile.name, profileId));
  emit vehicleProfileChanged(profileId);
  return saveProfiles();
}

VehicleProfile ProfileManager::getActiveVehicleProfile() const {
  if (m_vehicleProfiles.contains(m_activeVehicleProfileId)) {
    return m_vehicleProfiles.value(m_activeVehicleProfileId);
  }
  return VehicleProfile();
}

bool ProfileManager::addDeviceToHostProfile(const QString& profileId, const DeviceConfig& device) {
  if (!m_hostProfiles.contains(profileId)) {
    Logger::instance().warning(
        QString("ProfileManager: Host profile not found: %1").arg(profileId));
    return false;
  }

  HostProfile profile = m_hostProfiles[profileId];
  profile.devices.append(device);
  m_hostProfiles[profileId] = profile;

  Logger::instance().info(
      QString("ProfileManager: Device added to host profile %1: %2").arg(profileId, device.name));
  emit deviceConfigChanged(profileId, device.name);
  return saveProfiles();
}

bool ProfileManager::removeDeviceFromHostProfile(const QString& profileId,
                                                 const QString& deviceName) {
  if (!m_hostProfiles.contains(profileId)) {
    Logger::instance().warning(
        QString("ProfileManager: Host profile not found: %1").arg(profileId));
    return false;
  }

  HostProfile profile = m_hostProfiles[profileId];
  profile.devices.erase(std::remove_if(profile.devices.begin(), profile.devices.end(),
                                       [&](const DeviceConfig& d) { return d.name == deviceName; }),
                        profile.devices.end());
  m_hostProfiles[profileId] = profile;

  Logger::instance().info(QString("ProfileManager: Device removed from host profile %1: %2")
                              .arg(profileId, deviceName));
  emit deviceConfigChanged(profileId, deviceName);
  return saveProfiles();
}

bool ProfileManager::setDeviceEnabled(const QString& profileId, const QString& deviceName,
                                      bool enabled) {
  if (!m_hostProfiles.contains(profileId)) {
    return false;
  }

  HostProfile profile = m_hostProfiles[profileId];
  for (auto& device : profile.devices) {
    if (device.name == deviceName) {
      device.enabled = enabled;
      m_hostProfiles[profileId] = profile;
      Logger::instance().debug(QString("ProfileManager: Device %1 in profile %2 set to %3")
                                   .arg(deviceName, profileId, enabled ? "enabled" : "disabled"));
      emit deviceConfigChanged(profileId, deviceName);
      return saveProfiles();
    }
  }

  return false;
}

bool ProfileManager::setDeviceUseMock(const QString& profileId, const QString& deviceName,
                                      bool useMock) {
  if (!m_hostProfiles.contains(profileId)) {
    return false;
  }

  HostProfile profile = m_hostProfiles[profileId];
  for (auto& device : profile.devices) {
    if (device.name == deviceName) {
      device.useMock = useMock;
      m_hostProfiles[profileId] = profile;
      Logger::instance().debug(QString("ProfileManager: Device %1 in profile %2 set to use %3")
                                   .arg(deviceName, profileId, useMock ? "mock" : "real"));
      emit deviceConfigChanged(profileId, deviceName);
      return saveProfiles();
    }
  }

  return false;
}

QList<DeviceConfig> ProfileManager::getProfileDevices(const QString& profileId) const {
  if (m_hostProfiles.contains(profileId)) {
    return m_hostProfiles.value(profileId).devices;
  }
  return QList<DeviceConfig>();
}

bool ProfileManager::loadProfiles() {
  QString hostProfilesPath = QDir(m_configDir).filePath("host_profiles.json");
  QString vehicleProfilesPath = QDir(m_configDir).filePath("vehicle_profiles.json");

  // Load host profiles
  QFile hostFile(hostProfilesPath);
  if (hostFile.exists() && hostFile.open(QIODevice::ReadOnly)) {
    QJsonDocument doc = QJsonDocument::fromJson(hostFile.readAll());
    if (doc.isArray()) {
      for (const auto& value : doc.array()) {
        if (value.isObject()) {
          // TODO: Implement fromJson for HostProfile
        }
      }
    }
    hostFile.close();
  }

  // Load vehicle profiles
  QFile vehicleFile(vehicleProfilesPath);
  if (vehicleFile.exists() && vehicleFile.open(QIODevice::ReadOnly)) {
    QJsonDocument doc = QJsonDocument::fromJson(vehicleFile.readAll());
    if (doc.isArray()) {
      for (const auto& value : doc.array()) {
        if (value.isObject()) {
          // TODO: Implement fromJson for VehicleProfile
        }
      }
    }
    vehicleFile.close();
  }

  return true;
}

bool ProfileManager::saveProfiles() {
  QString hostProfilesPath = QDir(m_configDir).filePath("host_profiles.json");
  QString vehicleProfilesPath = QDir(m_configDir).filePath("vehicle_profiles.json");

  // Save host profiles
  QJsonArray hostArray;
  for (const auto& profile : m_hostProfiles) {
    // TODO: Implement toJson for HostProfile
  }
  QFile hostFile(hostProfilesPath);
  if (hostFile.open(QIODevice::WriteOnly)) {
    hostFile.write(QJsonDocument(hostArray).toJson());
    hostFile.close();
  }

  // Save vehicle profiles
  QJsonArray vehicleArray;
  for (const auto& profile : m_vehicleProfiles) {
    // TODO: Implement toJson for VehicleProfile
  }
  QFile vehicleFile(vehicleProfilesPath);
  if (vehicleFile.open(QIODevice::WriteOnly)) {
    vehicleFile.write(QJsonDocument(vehicleArray).toJson());
    vehicleFile.close();
  }

  return true;
}
