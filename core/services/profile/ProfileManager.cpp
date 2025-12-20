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
#include <QStandardPaths>
#include <QUuid>
#include <fstream>

// JSON libraries (nlohmann) and optional pboettch json-schema-validator
#include <nlohmann/json.hpp>

using nlohmann::json;

// json-schema validator is optional in CI; the build system defines
// CRANKSHAFT_JSON_SCHEMA_VALIDATOR as 1 when the validator headers are available
#if CRANKSHAFT_JSON_SCHEMA_VALIDATOR
#include <nlohmann/json-schema.hpp>
namespace json_schema = nlohmann::json_schema;
#endif

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

// --- Serialization helpers for profiles ---

QString HostProfile::toJson() const {
  QJsonObject obj;
  obj["id"] = id;
  obj["name"] = name;
  obj["description"] = description;
  obj["isActive"] = isActive;
  if (createdAt.isValid()) obj["createdAt"] = createdAt.toString(Qt::ISODate);
  if (modifiedAt.isValid()) obj["modifiedAt"] = modifiedAt.toString(Qt::ISODate);
  obj["cpuModel"] = cpuModel;
  obj["ramMB"] = static_cast<qint64>(ramMB);
  obj["osVersion"] = osVersion;

  // properties
  QVariantMap props;
  for (auto it = properties.constBegin(); it != properties.constEnd(); ++it) {
    props.insert(it.key(), it.value());
  }
  obj["properties"] = QJsonObject::fromVariantMap(props);

  // devices
  QJsonArray devArray;
  for (const auto& d : devices) {
    QJsonObject dobj;
    dobj["name"] = d.name;
    dobj["type"] = d.type;
    dobj["enabled"] = d.enabled;
    dobj["useMock"] = d.useMock;
    dobj["description"] = d.description;

    QVariantMap settingsMap;
    for (auto sit = d.settings.constBegin(); sit != d.settings.constEnd(); ++sit) {
      settingsMap.insert(sit.key(), sit.value());
    }
    dobj["settings"] = QJsonObject::fromVariantMap(settingsMap);

    devArray.append(dobj);
  }
  obj["devices"] = devArray;

  return QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

HostProfile HostProfile::fromJson(const QString& json) {
  HostProfile hp;
  QJsonParseError err;
  QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &err);
  if (err.error != QJsonParseError::NoError || !doc.isObject()) {
    return hp;
  }

  QJsonObject obj = doc.object();
  hp.id = obj.value("id").toString();
  hp.name = obj.value("name").toString();
  hp.description = obj.value("description").toString();
  hp.isActive = obj.value("isActive").toBool(false);
  hp.createdAt = QDateTime::fromString(obj.value("createdAt").toString(), Qt::ISODate);
  hp.modifiedAt = QDateTime::fromString(obj.value("modifiedAt").toString(), Qt::ISODate);
  hp.cpuModel = obj.value("cpuModel").toString();
  hp.ramMB = static_cast<quint32>(obj.value("ramMB").toInt());
  hp.osVersion = obj.value("osVersion").toString();

  // properties
  QJsonObject propObj = obj.value("properties").toObject();
  QVariantMap propMap = propObj.toVariantMap();
  for (auto it = propMap.constBegin(); it != propMap.constEnd(); ++it) {
    hp.properties.insert(it.key(), it.value());
  }

  // devices
  QJsonArray devArray = obj.value("devices").toArray();
  for (const auto& v : devArray) {
    if (!v.isObject()) continue;
    QJsonObject dobj = v.toObject();
    DeviceConfig d;
    d.name = dobj.value("name").toString();
    d.type = dobj.value("type").toString();
    d.enabled = dobj.value("enabled").toBool(true);
    d.useMock = dobj.value("useMock").toBool(true);
    d.description = dobj.value("description").toString();

    QJsonObject s = dobj.value("settings").toObject();
    QVariantMap sm = s.toVariantMap();
    for (auto sit = sm.constBegin(); sit != sm.constEnd(); ++sit) {
      d.settings.insert(sit.key(), sit.value());
    }

    hp.devices.append(d);
  }

  // ensure id
  if (hp.id.isEmpty()) {
    hp.id = QUuid::createUuid().toString();
  }

  return hp;
}

QString VehicleProfile::toJson() const {
  QJsonObject obj;
  obj["id"] = id;
  obj["name"] = name;
  obj["description"] = description;
  obj["isActive"] = isActive;
  if (createdAt.isValid()) obj["createdAt"] = createdAt.toString(Qt::ISODate);
  if (modifiedAt.isValid()) obj["modifiedAt"] = modifiedAt.toString(Qt::ISODate);
  obj["make"] = make;
  obj["model"] = model;
  obj["year"] = year;
  obj["vin"] = vin;
  obj["licensePlate"] = licensePlate;
  obj["vehicleType"] = vehicleType;

  QJsonArray modes;
  for (const auto& m : supportedModes) modes.append(m);
  obj["supportedModes"] = modes;
  obj["hasAWD"] = hasAWD;
  obj["wheelCount"] = static_cast<qint64>(wheelCount);

  QVariantMap props;
  for (auto it = properties.constBegin(); it != properties.constEnd(); ++it)
    props.insert(it.key(), it.value());
  obj["properties"] = QJsonObject::fromVariantMap(props);

  QVariantMap mocks;
  for (auto it = mockDefaults.constBegin(); it != mockDefaults.constEnd(); ++it)
    mocks.insert(it.key(), it.value());
  obj["mockDefaults"] = QJsonObject::fromVariantMap(mocks);

  return QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

VehicleProfile VehicleProfile::fromJson(const QString& json) {
  VehicleProfile vp;
  QJsonParseError err;
  QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &err);
  if (err.error != QJsonParseError::NoError || !doc.isObject()) return vp;

  QJsonObject obj = doc.object();
  vp.id = obj.value("id").toString();
  vp.name = obj.value("name").toString();
  vp.description = obj.value("description").toString();
  vp.isActive = obj.value("isActive").toBool(false);
  vp.createdAt = QDateTime::fromString(obj.value("createdAt").toString(), Qt::ISODate);
  vp.modifiedAt = QDateTime::fromString(obj.value("modifiedAt").toString(), Qt::ISODate);
  vp.make = obj.value("make").toString();
  vp.model = obj.value("model").toString();
  vp.year = obj.value("year").toString();
  vp.vin = obj.value("vin").toString();
  vp.licensePlate = obj.value("licensePlate").toString();
  vp.vehicleType = obj.value("vehicleType").toString();

  QJsonArray modes = obj.value("supportedModes").toArray();
  for (const auto& m : modes)
    if (m.isString()) vp.supportedModes.append(m.toString());

  vp.hasAWD = obj.value("hasAWD").toBool(false);
  vp.wheelCount = static_cast<quint32>(obj.value("wheelCount").toInt(0));

  QJsonObject propObj = obj.value("properties").toObject();
  QVariantMap pmap = propObj.toVariantMap();
  for (auto it = pmap.constBegin(); it != pmap.constEnd(); ++it)
    vp.properties.insert(it.key(), it.value());

  QJsonObject mockObj = obj.value("mockDefaults").toObject();
  QVariantMap mmap = mockObj.toVariantMap();
  for (auto it = mmap.constBegin(); it != mmap.constEnd(); ++it)
    vp.mockDefaults.insert(it.key(), it.value());

  if (vp.id.isEmpty()) vp.id = QUuid::createUuid().toString();
  return vp;
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
    // Try validating the whole document against the host_profiles schema first.
    bool wholeDocValid = false;
    if (doc.isArray()) {
      try {
        // locate schema file relative to source dir
        QString schemaPath =
            QString(CRANKSHAFT_SOURCE_DIR) + "/docs/schemas/host_profiles.schema.json";
        std::ifstream f(schemaPath.toStdString());
#if CRANKSHAFT_JSON_SCHEMA_VALIDATOR
        if (f.good()) {
          json schemaJson;
          f >> schemaJson;
          json instance = json::parse(doc.toJson(QJsonDocument::Compact).toStdString());
          json_schema::json_validator validator;
          validator.set_root_schema(schemaJson);
          validator.validate(instance);
          wholeDocValid = true;
        } else {
          Logger::instance().debug(
              QString("ProfileManager: Schema file not found: %1").arg(schemaPath));
        }
#else
        Q_UNUSED(f);
        Logger::instance().debug(
            QString("ProfileManager: json-schema-validator not available; skipping whole-host_profiles.json validation"));
#endif
      } catch (const std::exception& ex) {
        Logger::instance().warning(
            QString("ProfileManager: Whole-host_profiles.json schema validation failed: %1")
                .arg(QString::fromLatin1(ex.what())));
      }

      // If whole doc valid, accept items directly; otherwise validate each item individually
      // (fallback)
      if (wholeDocValid) {
        for (const auto& value : doc.array()) {
          if (value.isObject()) {
            QString itemJson =
                QString::fromUtf8(QJsonDocument(value.toObject()).toJson(QJsonDocument::Compact));
            HostProfile p = HostProfile::fromJson(itemJson);
            m_hostProfiles[p.id] = p;
            if (p.isActive) m_activeHostProfileId = p.id;
          }
        }
      } else {
        for (const auto& value : doc.array()) {
          if (!value.isObject()) continue;
          QJsonObject obj = value.toObject();

          // per-item validation by wrapping into a single-element array and validating against
          // schema
          bool itemValid = false;
          try {
            QString schemaPath =
                QString(CRANKSHAFT_SOURCE_DIR) + "/docs/schemas/host_profiles.schema.json";
            std::ifstream f(schemaPath.toStdString());
#if CRANKSHAFT_JSON_SCHEMA_VALIDATOR
            if (f.good()) {
              json schemaJson;
              f >> schemaJson;
              QJsonArray arr;
              arr.append(obj);
              json instance =
                  json::parse(QJsonDocument(arr).toJson(QJsonDocument::Compact).toStdString());
              json_schema::json_validator validator;
              validator.set_root_schema(schemaJson);
              validator.validate(instance);
              itemValid = true;
            }
#else
            Q_UNUSED(f);
#endif
          } catch (const std::exception& ex) {
            Q_UNUSED(ex);
          }

          if (!itemValid) {
            // fallback lightweight checks
            if (!obj.contains("name") || !obj.value("name").isString() ||
                !obj.contains("devices") || !obj.value("devices").isArray()) {
              Logger::instance().warning(
                  QString("ProfileManager: Skipping invalid host profile entry in %1")
                      .arg(hostProfilesPath));
              continue;
            }
          }

          QString itemJson = QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));
          HostProfile p = HostProfile::fromJson(itemJson);
          m_hostProfiles[p.id] = p;
          if (p.isActive) m_activeHostProfileId = p.id;
        }
      }
    }
    hostFile.close();
  }

  // Load vehicle profiles
  QFile vehicleFile(vehicleProfilesPath);
  if (vehicleFile.exists() && vehicleFile.open(QIODevice::ReadOnly)) {
    QJsonDocument doc = QJsonDocument::fromJson(vehicleFile.readAll());
    bool wholeDocValid = false;
    if (doc.isArray()) {
      try {
        QString schemaPath =
            QString(CRANKSHAFT_SOURCE_DIR) + "/docs/schemas/vehicle_profiles.schema.json";
        std::ifstream f(schemaPath.toStdString());
#if CRANKSHAFT_JSON_SCHEMA_VALIDATOR
        if (f.good()) {
          json schemaJson;
          f >> schemaJson;
          json instance = json::parse(doc.toJson(QJsonDocument::Compact).toStdString());
          json_schema::json_validator validator;
          validator.set_root_schema(schemaJson);
          validator.validate(instance);
          wholeDocValid = true;
        } else {
          Logger::instance().debug(
              QString("ProfileManager: Schema file not found: %1").arg(schemaPath));
        }
#else
        Q_UNUSED(f);
        Logger::instance().debug(
            QString("ProfileManager: json-schema-validator not available; skipping whole-vehicle_profiles.json validation"));
#endif
      } catch (const std::exception& ex) {
        Logger::instance().warning(
            QString("ProfileManager: Whole-vehicle_profiles.json schema validation failed: %1")
                .arg(QString::fromLatin1(ex.what())));
      }

      if (wholeDocValid) {
        for (const auto& value : doc.array()) {
          if (value.isObject()) {
            QString itemJson =
                QString::fromUtf8(QJsonDocument(value.toObject()).toJson(QJsonDocument::Compact));
            VehicleProfile v = VehicleProfile::fromJson(itemJson);
            m_vehicleProfiles[v.id] = v;
            if (v.isActive) m_activeVehicleProfileId = v.id;
          }
        }
      } else {
        for (const auto& value : doc.array()) {
          if (!value.isObject()) continue;
          QJsonObject obj = value.toObject();

          bool itemValid = false;
          try {
            QString schemaPath =
                QString(CRANKSHAFT_SOURCE_DIR) + "/docs/schemas/vehicle_profiles.schema.json";
            std::ifstream f(schemaPath.toStdString());
#if CRANKSHAFT_JSON_SCHEMA_VALIDATOR
            if (f.good()) {
              json schemaJson;
              f >> schemaJson;
              QJsonArray arr;
              arr.append(obj);
              json instance =
                  json::parse(QJsonDocument(arr).toJson(QJsonDocument::Compact).toStdString());
              json_schema::json_validator validator;
              validator.set_root_schema(schemaJson);
              validator.validate(instance);
              itemValid = true;
            }
#else
            Q_UNUSED(f);
#endif
          } catch (const std::exception& ex) {
            Q_UNUSED(ex);
          }

          if (!itemValid) {
            if (!obj.contains("name") || !obj.value("name").isString()) {
              Logger::instance().warning(
                  QString("ProfileManager: Skipping invalid vehicle profile entry in %1")
                      .arg(vehicleProfilesPath));
              continue;
            }
          }

          QString itemJson = QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));
          VehicleProfile v = VehicleProfile::fromJson(itemJson);
          m_vehicleProfiles[v.id] = v;
          if (v.isActive) m_activeVehicleProfileId = v.id;
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
    QString item = profile.toJson();
    QJsonDocument doc = QJsonDocument::fromJson(item.toUtf8());
    if (doc.isObject()) hostArray.append(doc.object());
  }
  QFile hostFile(hostProfilesPath);
  if (hostFile.open(QIODevice::WriteOnly)) {
    hostFile.write(QJsonDocument(hostArray).toJson());
    hostFile.close();
  }

  // Save vehicle profiles
  QJsonArray vehicleArray;
  for (const auto& profile : m_vehicleProfiles) {
    QString item = profile.toJson();
    QJsonDocument doc = QJsonDocument::fromJson(item.toUtf8());
    if (doc.isObject()) vehicleArray.append(doc.object());
  }
  QFile vehicleFile(vehicleProfilesPath);
  if (vehicleFile.open(QIODevice::WriteOnly)) {
    vehicleFile.write(QJsonDocument(vehicleArray).toJson());
    vehicleFile.close();
  }

  return true;
}
