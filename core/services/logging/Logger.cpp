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
#include <QFileInfo>
#include <QJsonDocument>
#include <QTextStream>
#include <QThread>

Logger& Logger::instance() {
  static Logger instance;
  return instance;
}

void Logger::setLevel(Level level) {
  m_level = level;
}

void Logger::setLogFile(const QString& filePath) {
  m_logFile = filePath;
  m_currentLogSize = 0;

  // Check initial log file size
  if (!m_logFile.isEmpty()) {
    QFileInfo fileInfo(m_logFile);
    if (fileInfo.exists()) {
      m_currentLogSize = fileInfo.size();
    }
  }
}

void Logger::setJsonFormat(bool enabled) {
  m_jsonFormat = enabled;
}

void Logger::setMaxLogSize(qint64 bytes) {
  m_maxLogSize = bytes;
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

void Logger::fatal(const QString& message) {
  log(Level::Fatal, message);
}

void Logger::logStructured(Level level, const QString& component, const QString& message,
                           const QJsonObject& context) {
  if (level < m_level) return;

  QJsonObject logEntry = createLogEntry(level, component, message, context);
  QJsonDocument doc(logEntry);

  QString logMessage;
  if (m_jsonFormat) {
    logMessage = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
  } else {
    // Fallback to readable format
    logMessage =
        QString("[%1] %2 (%3): %4")
            .arg(logEntry["timestamp"].toString(), levelToString(level), component, message);
  }

  // Console output
  qDebug().noquote() << logMessage;

  // File output
  if (!m_logFile.isEmpty()) {
    rotateLogIfNeeded();
    QFile file(m_logFile);
    if (file.open(QIODevice::WriteOnly | QIODevice::Append)) {
      QTextStream stream(&file);
      stream << logMessage << "\n";
      m_currentLogSize += logMessage.length() + 1;  // +1 for newline
      file.close();
    }
  }
}

void Logger::debugContext(const QString& component, const QString& message,
                          const QJsonObject& context) {
  logStructured(Level::Debug, component, message, context);
}

void Logger::infoContext(const QString& component, const QString& message,
                         const QJsonObject& context) {
  logStructured(Level::Info, component, message, context);
}

void Logger::warningContext(const QString& component, const QString& message,
                            const QJsonObject& context) {
  logStructured(Level::Warning, component, message, context);
}

void Logger::errorContext(const QString& component, const QString& message,
                          const QJsonObject& context) {
  logStructured(Level::Error, component, message, context);
}

void Logger::log(Level level, const QString& message) {
  if (level < m_level) return;

  logStructured(level, "Crankshaft", message, QJsonObject());
}

void Logger::rotateLogIfNeeded() {
  if (m_currentLogSize >= m_maxLogSize && !m_logFile.isEmpty()) {
    QString rotatedFile =
        m_logFile + "." + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QFile::rename(m_logFile, rotatedFile);
    m_currentLogSize = 0;

    // Clean up old rotated logs (keep last 5)
    QFileInfo fileInfo(m_logFile);
    QDir dir = fileInfo.dir();
    QStringList filters;
    filters << (fileInfo.baseName() + "*");
    dir.setFilter(QDir::Files);
    dir.setSorting(QDir::Time);

    QFileInfoList logs = dir.entryInfoList(filters);
    while (logs.count() > 5) {
      QFile::remove(logs.last().filePath());
      logs.removeLast();
    }
  }
}

QJsonObject Logger::createLogEntry(Level level, const QString& component, const QString& message,
                                   const QJsonObject& context) const {
  QJsonObject entry;

  entry["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
  entry["level"] = levelToString(level);
  entry["component"] = component;
  entry["message"] = message;
  entry["thread"] = QString::number(reinterpret_cast<qint64>(QThread::currentThread()));

  // Merge context if provided
  for (auto it = context.constBegin(); it != context.constEnd(); ++it) {
    entry[it.key()] = it.value();
  }

  return entry;
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
    case Level::Fatal:
      return "FATAL";
    default:
      return "UNKNOWN";
  }
}
