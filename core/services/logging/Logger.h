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

#include <QJsonObject>
#include <QObject>
#include <QString>

class Logger : public QObject {
  Q_OBJECT

 public:
  enum class Level { Debug = 0, Info = 1, Warning = 2, Error = 3, Fatal = 4 };

  [[nodiscard]] static Logger& instance();

  // Configuration
  void setLevel(Level level);
  void setLogFile(const QString& filePath);
  void setJsonFormat(bool enabled);
  void setMaxLogSize(qint64 bytes);  // For log rotation

  // Simple logging (backward compatible)
  void debug(const QString& message);
  void info(const QString& message);
  void warning(const QString& message);
  void error(const QString& message);
  void fatal(const QString& message);

  // Structured logging with context
  void logStructured(Level level, const QString& component, const QString& message,
                     const QJsonObject& context = QJsonObject());

  // Contextual logging helpers
  void debugContext(const QString& component, const QString& message,
                    const QJsonObject& context = QJsonObject());
  void infoContext(const QString& component, const QString& message,
                   const QJsonObject& context = QJsonObject());
  void warningContext(const QString& component, const QString& message,
                      const QJsonObject& context = QJsonObject());
  void errorContext(const QString& component, const QString& message,
                    const QJsonObject& context = QJsonObject());

 private:
  Logger() = default;
  ~Logger() = default;
  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;

  void log(Level level, const QString& message);
  void rotateLogIfNeeded();
  [[nodiscard]] QString levelToString(Level level) const;
  [[nodiscard]] QJsonObject createLogEntry(Level level, const QString& component,
                                           const QString& message,
                                           const QJsonObject& context) const;

  Level m_level{Level::Info};
  QString m_logFile;
  bool m_jsonFormat{true};                // Default to JSON format
  qint64 m_maxLogSize{10 * 1024 * 1024};  // 10 MB default
  qint64 m_currentLogSize{0};
};
