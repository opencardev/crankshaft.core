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

#include "PreferencesService.h"

#include <QJsonDocument>
#include <QJsonValue>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>

#include "../logging/Logger.h"

PreferencesService::PreferencesService(const QString& dbPath, QObject* parent)
    : QObject(parent),
      m_db(std::make_unique<QSqlDatabase>(QSqlDatabase::addDatabase("QSQLITE"))),
      m_dbPath(dbPath.isEmpty()
                   ? QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
                         "/preferences.db"
                   : dbPath) {
  Logger::instance().info(QString("[PreferencesService] Initialized with database: %1")
                              .arg(m_dbPath));
}

PreferencesService::~PreferencesService() = default;

bool PreferencesService::initialize() {
  m_db->setDatabaseName(m_dbPath);

  if (!m_db->open()) {
    Logger::instance().error(
        QString("[PreferencesService] Failed to open database: %1").arg(m_db->lastError().text()));
    return false;
  }

  if (!createSchema()) {
    Logger::instance().error("[PreferencesService] Failed to create schema");
    m_db->close();
    return false;
  }

  if (!loadPreferences()) {
    Logger::instance().error("[PreferencesService] Failed to load preferences from database");
    m_db->close();
    return false;
  }

  Logger::instance().info("[PreferencesService] Initialized successfully");
  return true;
}

bool PreferencesService::createSchema() {
  QSqlQuery query(*m_db);
  const QString createTableSQL =
      QStringLiteral(
          "CREATE TABLE IF NOT EXISTS preferences ("
          "  user_id TEXT DEFAULT 'default',"
          "  key TEXT NOT NULL,"
          "  value TEXT NOT NULL,"
          "  PRIMARY KEY (user_id, key)"
          ")");

  if (!query.exec(createTableSQL)) {
    Logger::instance().error(
        QString("[PreferencesService] Schema creation error: %1").arg(query.lastError().text()));
    return false;
  }

  return true;
}

bool PreferencesService::loadPreferences() {
  QSqlQuery query(*m_db);
  if (!query.exec(QStringLiteral("SELECT key, value FROM preferences WHERE user_id = 'default'"))) {
    Logger::instance().error(
        QString("[PreferencesService] Failed to load preferences: %1").arg(query.lastError().text()));
    return false;
  }

  m_cache.clear();
  while (query.next()) {
    const QString key = query.value(0).toString();
    const QString valueString = query.value(1).toString();
    
    // Deserialize JSON string back to QVariant
    QVariant cachedValue;
    if (valueString == QStringLiteral("true")) {
      cachedValue = true;
    } else if (valueString == QStringLiteral("false")) {
      cachedValue = false;
    } else if (valueString.startsWith('"') && valueString.endsWith('"')) {
      // String value with quotes - remove them
      cachedValue = valueString.mid(1, valueString.length() - 2).replace("\\\"", "\"");
    } else {
      // Try as number
      bool ok = false;
      const int intValue = valueString.toInt(&ok);
      if (ok) {
        cachedValue = intValue;
      } else {
        const double doubleValue = valueString.toDouble(&ok);
        if (ok) {
          cachedValue = doubleValue;
        } else {
          // Complex type - parse as JSON
          const QByteArray valueBytes = valueString.toUtf8();
          const QJsonDocument doc = QJsonDocument::fromJson(valueBytes);
          cachedValue = doc.toVariant();
        }
      }
    }
    
    m_cache[key] = cachedValue;
    Logger::instance().debug(QString("[PreferencesService] Loaded preference: %1").arg(key));
  }

  return true;
}

QVariant PreferencesService::get(const QString& key, const QVariant& defaultValue) const {
  if (m_cache.contains(key)) {
    return m_cache.value(key);
  }
  return defaultValue;
}

bool PreferencesService::set(const QString& key, const QVariant& value) {
  // Convert QVariant to JSON string for storage
  QString jsonString;
  
  // Handle different QVariant types for proper JSON serialization
  if (value.type() == QVariant::Bool) {
    jsonString = value.toBool() ? QStringLiteral("true") : QStringLiteral("false");
  } else if (value.type() == QVariant::Int || value.type() == QVariant::Double) {
    jsonString = value.toString();
  } else if (value.type() == QVariant::String) {
    // Escape quotes in string values
    jsonString = QStringLiteral("\"%1\"").arg(value.toString().replace("\"", "\\\""));
  } else {
    // For complex types, use JSON document
    const QJsonDocument doc = QJsonDocument::fromVariant(value);
    jsonString = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
  }

  QSqlQuery query(*m_db);
  query.prepare(
      QStringLiteral("INSERT OR REPLACE INTO preferences (user_id, key, value) VALUES ('default', ?, ?)"));
  query.addBindValue(key);
  query.addBindValue(jsonString);

  if (!query.exec()) {
    Logger::instance().error(
        QString("[PreferencesService] Failed to set %1: %2").arg(key, query.lastError().text()));
    return false;
  }

  m_cache[key] = value;
  Logger::instance().info(QString("[PreferencesService] Preference set: %1").arg(key));
  emit preferenceChanged(key, value);
  return true;
}

bool PreferencesService::contains(const QString& key) const {
  return m_cache.contains(key);
}

bool PreferencesService::remove(const QString& key) {
  QSqlQuery query(*m_db);
  query.prepare(QStringLiteral("DELETE FROM preferences WHERE user_id = 'default' AND key = ?"));
  query.addBindValue(key);

  if (!query.exec()) {
    Logger::instance().error(
        QString("[PreferencesService] Failed to remove %1: %2").arg(key, query.lastError().text()));
    return false;
  }

  m_cache.remove(key);
  Logger::instance().info(QString("[PreferencesService] Preference removed: %1").arg(key));
  return true;
}

bool PreferencesService::clear() {
  QSqlQuery query(*m_db);
  if (!query.exec(QStringLiteral("DELETE FROM preferences WHERE user_id = 'default'"))) {
    Logger::instance().error(
        QString("[PreferencesService] Failed to clear preferences: %1")
            .arg(query.lastError().text()));
    return false;
  }

  m_cache.clear();
  Logger::instance().info("[PreferencesService] All preferences cleared");
  return true;
}

QStringList PreferencesService::allKeys() const {
  return m_cache.keys();
}
