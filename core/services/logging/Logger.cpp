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

#include "Logger.h"

#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QTextStream>

Logger& Logger::instance() {
  static Logger instance;
  return instance;
}

void Logger::setLevel(Level level) {
  m_level = level;
}

void Logger::setLogFile(const QString& filePath) {
  m_logFile = filePath;
}

void Logger::debug(const QString& message) {
  log(Level::Debug, message);
}

void Logger::info(const QString& message) {
  log(Level::Info, message);
}

void Logger::warning(const QString& message) {
  log(Level::Warning, message);
}

void Logger::error(const QString& message) {
  log(Level::Error, message);
}

void Logger::log(Level level, const QString& message) {
  if (level < m_level) return;

  QString timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);
  QString levelStr = levelToString(level);
  QString logMessage = QString("[%1] %2: %3").arg(timestamp, levelStr, message);

  // Console output
  qDebug().noquote() << logMessage;

  // File output
  if (!m_logFile.isEmpty()) {
    QFile file(m_logFile);
    if (file.open(QIODevice::WriteOnly | QIODevice::Append)) {
      QTextStream stream(&file);
      stream << logMessage << "\n";
      file.close();
    }
  }
}

QString Logger::levelToString(Level level) const {
  switch (level) {
    case Level::Debug:
      return "DEBUG";
    case Level::Info:
      return "INFO";
    case Level::Warning:
      return "WARNING";
    case Level::Error:
      return "ERROR";
    default:
      return "UNKNOWN";
  }
}
