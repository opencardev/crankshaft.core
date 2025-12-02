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

class Logger : public QObject {
  Q_OBJECT

 public:
  enum Level { Debug, Info, Warning, Error };

  static Logger& instance();

  void setLevel(Level level);
  void setLogFile(const QString& filePath);

  void debug(const QString& message);
  void info(const QString& message);
  void warning(const QString& message);
  void error(const QString& message);

 private:
  Logger() = default;
  ~Logger() = default;
  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;

  void log(Level level, const QString& message);
  QString levelToString(Level level) const;

  Level m_level = Info;
  QString m_logFile;
};
