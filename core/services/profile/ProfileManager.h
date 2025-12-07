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
#include <QMap>
#include <QList>
#include <QVariant>
#include <QDateTime>
#include <memory>

/**
 * @brief Device configuration entry in a profile
 */
struct DeviceConfig {
  QString name;
  QString type;              // DeviceInterfaceType as string
  bool enabled{true};
  bool useMock{true};
  QMap<QString, QVariant> settings;
  QString description;
};

/**
 * @brief Host profile containing device configurations
 *
 * Allows defining different host setups with mixed real/mock devices.
 * Useful for development/testing with incomplete hardware.
 */
struct HostProfile {
  QString id;
  QString name;
  QString description;
  bool isActive{false};
  QDateTime createdAt;
  QDateTime modifiedAt;
  QList<DeviceConfig> devices;

  // Host properties
  QString cpuModel;
  quint32 ramMB{0};
  QString osVersion;
  QMap<QString, QVariant> properties;

  QString toJson() const;
  static HostProfile fromJson(const QString& json);
};

/**
 * @brief Vehicle profile containing vehicle-specific configurations
 *
 * Allows defining different vehicle types (car, truck, bus, etc.)
 * with different VehicleHAL properties and mock data.
 */
struct VehicleProfile {
  QString id;
  QString name;
  QString description;
  bool isActive{false};
  QDateTime createdAt;
  QDateTime modifiedAt;

  // Vehicle information
  QString make;              // e.g., "Tesla"
  QString model;             // e.g., "Model 3"
  QString year;
  QString vin;
  QString licensePlate;
  QString vehicleType;       // "car", "truck", "bus", "van", etc.

  // Vehicle capabilities
  QList<QString> supportedModes;  // e.g., "PARK", "REVERSE", "NEUTRAL", "DRIVE"
  bool hasAWD{false};
  quint32 wheelCount{4};

  // Mock data configuration
  QMap<QString, QVariant> properties;
  QMap<QString, QVariant> mockDefaults;

  QString toJson() const;
  static VehicleProfile fromJson(const QString& json);
};

/**
 * @brief Profile Manager for handling host and vehicle configurations
 *
 * Manages creation, persistence, and activation of host and vehicle profiles.
 * Allows switching between different configurations at runtime.
 */
class ProfileManager : public QObject {
  Q_OBJECT

 public:
  explicit ProfileManager(const QString& configDir, QObject* parent = nullptr);
  ~ProfileManager();

  // Host profile management
  bool createHostProfile(const HostProfile& profile);
  bool updateHostProfile(const HostProfile& profile);
  bool deleteHostProfile(const QString& profileId);
  HostProfile getHostProfile(const QString& profileId) const;
  QList<HostProfile> getAllHostProfiles() const;
  bool setActiveHostProfile(const QString& profileId);
  HostProfile getActiveHostProfile() const;

  // Vehicle profile management
  bool createVehicleProfile(const VehicleProfile& profile);
  bool updateVehicleProfile(const VehicleProfile& profile);
  bool deleteVehicleProfile(const QString& profileId);
  VehicleProfile getVehicleProfile(const QString& profileId) const;
  QList<VehicleProfile> getAllVehicleProfiles() const;
  bool setActiveVehicleProfile(const QString& profileId);
  VehicleProfile getActiveVehicleProfile() const;

  // Device management within profiles
  bool addDeviceToHostProfile(const QString& profileId, const DeviceConfig& device);
  bool removeDeviceFromHostProfile(const QString& profileId, const QString& deviceName);
  bool setDeviceEnabled(const QString& profileId, const QString& deviceName, bool enabled);
  bool setDeviceUseMock(const QString& profileId, const QString& deviceName, bool useMock);
  QList<DeviceConfig> getProfileDevices(const QString& profileId) const;

  // Persistence
  bool loadProfiles();
  bool saveProfiles();

 signals:
  void hostProfileChanged(const QString& profileId);
  void vehicleProfileChanged(const QString& profileId);
  void deviceConfigChanged(const QString& profileId, const QString& deviceName);

 private:
  void initializeDefaultProfiles();

  QString m_configDir;
  QMap<QString, HostProfile> m_hostProfiles;
  QMap<QString, VehicleProfile> m_vehicleProfiles;
  QString m_activeHostProfileId;
  QString m_activeVehicleProfileId;
};
