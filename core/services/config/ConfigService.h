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
#include <QVariantMap>

class ConfigService : public QObject {
  Q_OBJECT

 public:
  [[nodiscard]] static ConfigService& instance();

  bool load(const QString& filePath);
  bool save(const QString& filePath);

  [[nodiscard]] QVariant get(const QString& key, const QVariant& defaultValue = QVariant()) const;
  void set(const QString& key, const QVariant& value);

 signals:
  void configChanged(const QString& key, const QVariant& value);

 private:
  ConfigService() = default;
  ~ConfigService() = default;
  ConfigService(const ConfigService&) = delete;
  ConfigService& operator=(const ConfigService&) = delete;

  QVariantMap m_config;
};
