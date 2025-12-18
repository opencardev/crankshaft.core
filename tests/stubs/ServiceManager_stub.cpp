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

#include <QString>
#include <QStringList>

#include "../../core/services/service_manager/ServiceManager.h"

ServiceManager::ServiceManager(ProfileManager* profileManager, QObject* parent)
    : QObject(parent),
      m_profileManager(profileManager),
      m_androidAutoService(nullptr),
      m_wifiManager(nullptr),
      m_bluetoothManager(nullptr),
      m_mediaPipeline(nullptr) {}

ServiceManager::~ServiceManager() = default;

bool ServiceManager::startAllServices() { return true; }

void ServiceManager::stopAllServices() {}

void ServiceManager::reloadServices() {}

bool ServiceManager::startService(const QString&) { return true; }

bool ServiceManager::stopService(const QString&) { return true; }

bool ServiceManager::restartService(const QString&) { return true; }

bool ServiceManager::isServiceRunning(const QString&) const { return false; }

QStringList ServiceManager::getRunningServices() const { return QStringList{}; }

void ServiceManager::onProfileChanged(const QString&) {}

void ServiceManager::onDeviceConfigChanged(const QString&, const QString&) {}
