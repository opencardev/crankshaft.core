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
#include <QString>
#include <QVariant>
#include <memory>

class QSqlDatabase;

/**
 * @brief SQLite-backed user preferences service
 *
 * Manages persistent key-value preferences for the application. Preferences are stored
 * in SQLite with in-memory caching for frequently accessed entries.
 */
class PreferencesService : public QObject {
  Q_OBJECT

 public:
  explicit PreferencesService(const QString& dbPath = QString(), QObject* parent = nullptr);
  ~PreferencesService() override;

  // Initialize database and schema
  [[nodiscard]] bool initialize();

  // Get preference value (cached)
  [[nodiscard]] QVariant get(const QString& key, const QVariant& defaultValue = QVariant()) const;

  // Set preference value (updates cache and database)
  [[nodiscard]] bool set(const QString& key, const QVariant& value);

  // Check if preference exists
  [[nodiscard]] bool contains(const QString& key) const;

  // Remove preference
  [[nodiscard]] bool remove(const QString& key);

  // Clear all preferences
  [[nodiscard]] bool clear();

  // Get all preference keys
  [[nodiscard]] QStringList allKeys() const;

 signals:
  void preferenceChanged(const QString& key, const QVariant& newValue);

 private:
  [[nodiscard]] bool createSchema();
  [[nodiscard]] bool loadPreferences();

  std::unique_ptr<QSqlDatabase> m_db;
  QString m_dbPath;
  QMap<QString, QVariant> m_cache;
};
