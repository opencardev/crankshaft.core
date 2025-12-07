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

#include "ConfigService.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

#include "Logger.h"

ConfigService& ConfigService::instance() {
  static ConfigService instance;
  return instance;
}

bool ConfigService::load(const QString& filePath) {
  QFile file(filePath);
  if (!file.open(QIODevice::ReadOnly)) {
    Logger::instance().error(QString("Failed to open config file: %1").arg(filePath));
    return false;
  }

  QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
  file.close();

  if (!doc.isObject()) {
    Logger::instance().error("Invalid config file format");
    return false;
  }

  m_config = doc.object().toVariantMap();
  Logger::instance().info(QString("Loaded configuration from %1").arg(filePath));
  return true;
}

bool ConfigService::save(const QString& filePath) {
  QJsonDocument doc(QJsonObject::fromVariantMap(m_config));

  QFile file(filePath);
  if (!file.open(QIODevice::WriteOnly)) {
    Logger::instance().error(QString("Failed to write config file: %1").arg(filePath));
    return false;
  }

  file.write(doc.toJson(QJsonDocument::Indented));
  file.close();

  Logger::instance().info(QString("Saved configuration to %1").arg(filePath));
  return true;
}

QVariant ConfigService::get(const QString& key, const QVariant& defaultValue) const {
  QStringList keys = key.split('.');
  QVariantMap current = m_config;

  for (int i = 0; i < keys.size() - 1; ++i) {
    if (!current.contains(keys[i]) || !current[keys[i]].canConvert<QVariantMap>()) {
      return defaultValue;
    }
    current = current[keys[i]].toMap();
  }

  return current.value(keys.last(), defaultValue);
}

void ConfigService::set(const QString& key, const QVariant& value) {
  QStringList keys = key.split('.');
  
  // Convert to JSON for easier nested modification
  QJsonObject root = QJsonObject::fromVariantMap(m_config);
  QJsonObject* current = &root;
  
  // Navigate to the parent object
  for (int i = 0; i < keys.size() - 1; ++i) {
    if (!current->contains(keys[i])) {
      current->insert(keys[i], QJsonObject());
    }
    
    QJsonValue val = (*current)[keys[i]];
    if (!val.isObject()) {
      current->insert(keys[i], QJsonObject());
      val = (*current)[keys[i]];
    }
    
    // Can't get pointer to nested object, so we'll use a different approach
    // Just flatten the logic
    break;
  }
  
  // Simpler approach: rebuild the path
  if (keys.size() == 1) {
    root[keys[0]] = QJsonValue::fromVariant(value);
  } else if (keys.size() == 2) {
    QJsonObject level1 = root[keys[0]].toObject();
    level1[keys[1]] = QJsonValue::fromVariant(value);
    root[keys[0]] = level1;
  } else if (keys.size() == 3) {
    QJsonObject level1 = root[keys[0]].toObject();
    QJsonObject level2 = level1[keys[1]].toObject();
    level2[keys[2]] = QJsonValue::fromVariant(value);
    level1[keys[1]] = level2;
    root[keys[0]] = level1;
  }
  
  m_config = root.toVariantMap();
  emit configChanged(key, value);
}
