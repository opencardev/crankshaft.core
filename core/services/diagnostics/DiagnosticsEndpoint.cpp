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

#include "DiagnosticsEndpoint.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QSysInfo>

#include "../eventbus/EventBus.h"
#include "../extensions/ExtensionManager.h"
#include "../logging/Logger.h"
#include "../service_manager/ServiceManager.h"

#ifndef CRANKSHAFT_VERSION
#define CRANKSHAFT_VERSION "unknown"
#endif

DiagnosticsEndpoint::DiagnosticsEndpoint(EventBus* eventBus, ServiceManager* serviceManager,
                                         Logger* logger, QObject* parent)
    : QObject(parent),
      m_eventBus(eventBus),
      m_serviceManager(serviceManager),
      m_logger(logger),
      m_extensionManager(nullptr),
      m_startTime(QDateTime::currentMSecsSinceEpoch()) {
  if (m_logger) {
    m_logger->info(QStringLiteral("DiagnosticsEndpoint constructed"));
  }
}

bool DiagnosticsEndpoint::init() {
  if (!m_eventBus || !m_serviceManager || !m_logger) {
    if (m_logger) {
      m_logger->error(
          QStringLiteral("DiagnosticsEndpoint initialisation failed: missing dependencies"));
    }
    return false;
  }

  m_logger->info(QStringLiteral("DiagnosticsEndpoint initialised successfully"));
  return true;
}

void DiagnosticsEndpoint::shutdown() {
  m_logger->info(QStringLiteral("DiagnosticsEndpoint shutdown"));
}

QJsonObject DiagnosticsEndpoint::handleHealthCheck() {
  emit requestReceived(QStringLiteral("/health"), QStringLiteral("GET"));
  return gatherHealthStatus();
}

QJsonObject DiagnosticsEndpoint::handleMetricsRequest() {
  emit requestReceived(QStringLiteral("/metrics"), QStringLiteral("GET"));
  return gatherMetrics();
}

QJsonObject DiagnosticsEndpoint::handleExtensionsListRequest() {
  emit requestReceived(QStringLiteral("/extensions"), QStringLiteral("GET"));
  return gatherExtensionsList();
}

QJsonObject DiagnosticsEndpoint::handleExtensionsReloadRequest() {
  emit requestReceived(QStringLiteral("/extensions"), QStringLiteral("POST"));

  QJsonObject response;
  response[QStringLiteral("status")] = QStringLiteral("success");
  response[QStringLiteral("message")] = QStringLiteral("Extension registry reload requested");
  response[QStringLiteral("timestamp")] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

  // Emit event to trigger extension reload
  if (m_eventBus) {
    QVariantMap payload;
    payload[QStringLiteral("action")] = QStringLiteral("reload");
    m_eventBus->publish(QStringLiteral("extensions/reload"), payload);
  }

  return response;
}

QJsonObject DiagnosticsEndpoint::gatherHealthStatus() {
  QJsonObject health;

  // Application status
  health[QStringLiteral("status")] = QStringLiteral("healthy");
  health[QStringLiteral("application")] = QStringLiteral("crankshaft-core");
  health[QStringLiteral("version")] = CRANKSHAFT_VERSION;

  // Timestamps
  qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
  qint64 uptimeMs = currentTime - m_startTime;
  qint64 uptimeSec = uptimeMs / 1000;
  qint64 uptimeMin = uptimeSec / 60;
  qint64 uptimeHour = uptimeMin / 60;

  health[QStringLiteral("started_at")] =
      QDateTime::fromMSecsSinceEpoch(m_startTime).toString(Qt::ISODate);
  health[QStringLiteral("uptime_seconds")] = static_cast<int>(uptimeSec % 60);
  health[QStringLiteral("uptime_minutes")] = static_cast<int>(uptimeMin % 60);
  health[QStringLiteral("uptime_hours")] = static_cast<int>(uptimeHour);

  // System information
  QJsonObject systemInfo;
  systemInfo[QStringLiteral("os")] = QSysInfo::prettyProductName();
  systemInfo[QStringLiteral("kernel")] = QSysInfo::kernelType();
  systemInfo[QStringLiteral("kernel_version")] = QSysInfo::kernelVersion();
  systemInfo[QStringLiteral("cpu_architecture")] = QSysInfo::currentCpuArchitecture();
  systemInfo[QStringLiteral("host_name")] = QSysInfo::machineHostName();
  health[QStringLiteral("system")] = systemInfo;

  // Service status (placeholder)
  QJsonObject services;
  services[QStringLiteral("event_bus")] =
      m_eventBus ? QStringLiteral("ok") : QStringLiteral("error");
  services[QStringLiteral("service_manager")] =
      m_serviceManager ? QStringLiteral("ok") : QStringLiteral("error");
  services[QStringLiteral("logger")] = m_logger ? QStringLiteral("ok") : QStringLiteral("error");
  health[QStringLiteral("services")] = services;

  return health;
}

QJsonObject DiagnosticsEndpoint::gatherMetrics() {
  QJsonObject metrics;

  // CPU and memory metrics
  QJsonObject performance;
  performance[QStringLiteral("cpu_usage_percent")] = calculateCpuUsage();
  performance[QStringLiteral("memory_usage_mb")] = calculateMemoryUsage();
  metrics[QStringLiteral("performance")] = performance;

  // Event bus metrics
  QJsonObject eventBusMetrics;
  eventBusMetrics[QStringLiteral("subscribers")] = 0;         // Placeholder
  eventBusMetrics[QStringLiteral("published_messages")] = 0;  // Placeholder
  metrics[QStringLiteral("eventbus")] = eventBusMetrics;

  // Active services count
  if (m_serviceManager) {
    metrics[QStringLiteral("active_services")] = 0;  // Placeholder: get from ServiceManager
  }

  // Timestamp
  metrics[QStringLiteral("timestamp")] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

  return metrics;
}

QJsonObject DiagnosticsEndpoint::gatherExtensionsList() {
  QJsonObject response;

  // Extensions list (placeholder)
  QJsonArray extensions;

  // Example extension entry
  QJsonObject exampleExt;
  exampleExt[QStringLiteral("name")] = QStringLiteral("media-player");
  exampleExt[QStringLiteral("version")] = QStringLiteral("1.0.0");
  exampleExt[QStringLiteral("status")] = QStringLiteral("loaded");
  exampleExt[QStringLiteral("path")] =
      QStringLiteral("/usr/lib/crankshaft/extensions/media-player/lib.so");
  exampleExt[QStringLiteral("permissions")] =
      QJsonArray{QStringLiteral("audio_device_access"), QStringLiteral("event_bus_publish")};

  // Placeholder: add actual extensions from ServiceManager
  // for (const auto& ext : m_serviceManager->loadedExtensions()) {
  //   extensions.append(ext.toJson());
  // }

  response[QStringLiteral("extensions")] = extensions;
  response[QStringLiteral("total_count")] = extensions.size();
  response[QStringLiteral("timestamp")] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

  return response;
}

float DiagnosticsEndpoint::calculateCpuUsage() {
  // Placeholder implementation
  // In production, read from /proc/stat or use QProcess to run 'ps' command
  QProcess ps;
  ps.start(QStringLiteral("ps"), QStringList() << QStringLiteral("aux"));
  ps.waitForFinished();

  QString output = QString::fromLocal8Bit(ps.readAll());
  QStringList lines = output.split('\n');

  // Find Crankshaft process and extract CPU percentage
  for (const auto& line : lines) {
    if (line.contains(QStringLiteral("crankshaft"))) {
      QStringList parts = line.split(QRegularExpression(QStringLiteral("\\s+")));
      if (parts.size() >= 3) {
        bool ok = false;
        float cpuPercent = parts[2].toFloat(&ok);
        if (ok) {
          return cpuPercent;
        }
      }
    }
  }

  return 0.0f;
}

float DiagnosticsEndpoint::calculateMemoryUsage() {
  // Placeholder implementation
  // In production, read from /proc/self/status or use system memory info
  QFile statusFile(QStringLiteral("/proc/self/status"));
  if (!statusFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return 0.0f;
  }

  QTextStream stream(&statusFile);
  QString line;
  while (stream.readLineInto(&line)) {
    if (line.startsWith(QStringLiteral("VmRSS:"))) {
      QStringList parts = line.split(QRegularExpression(QStringLiteral("\\s+")));
      if (parts.size() >= 2) {
        bool ok = false;
        // VmRSS is in KB, convert to MB
        float memoryKb = parts[1].toFloat(&ok);
        if (ok) {
          statusFile.close();
          return memoryKb / 1024.0f;
        }
      }
    }
  }

  statusFile.close();
  return 0.0f;
}

QJsonObject DiagnosticsEndpoint::handleExtensionsInstallRequest(const QJsonObject& requestBody) {
  if (!m_extensionManager) {
    return QJsonObject{{QStringLiteral("error"), QStringLiteral("ExtensionManager not available")}};
  }

  QString manifestJson = requestBody.value(QStringLiteral("manifest")).toString();
  QString targetDir = requestBody.value(QStringLiteral("target_dir")).toString();

  QString result = m_extensionManager->installExtension(manifestJson, targetDir);

  if (!result.isEmpty()) {
    return QJsonObject{{QStringLiteral("error"), result}};
  }

  return QJsonObject{{QStringLiteral("status"), QStringLiteral("success")}};
}

QJsonObject DiagnosticsEndpoint::handleExtensionsUninstallRequest(const QString& extensionId) {
  if (!m_extensionManager) {
    return QJsonObject{{QStringLiteral("error"), QStringLiteral("ExtensionManager not available")}};
  }

  QString result = m_extensionManager->uninstallExtension(extensionId);

  if (!result.isEmpty()) {
    return QJsonObject{{QStringLiteral("error"), result}};
  }

  return QJsonObject{{QStringLiteral("status"), QStringLiteral("success")}};
}

QJsonObject DiagnosticsEndpoint::handleExtensionsStartRequest(const QString& extensionId) {
  if (!m_extensionManager) {
    return QJsonObject{{QStringLiteral("error"), QStringLiteral("ExtensionManager not available")}};
  }

  QString result = m_extensionManager->startExtension(extensionId);

  if (!result.isEmpty()) {
    return QJsonObject{{QStringLiteral("error"), result}};
  }

  QJsonObject info = m_extensionManager->getExtensionInfo(extensionId);
  return QJsonObject{{QStringLiteral("status"), QStringLiteral("success")},
                     {QStringLiteral("extension"), info}};
}

QJsonObject DiagnosticsEndpoint::handleExtensionsStopRequest(const QString& extensionId) {
  if (!m_extensionManager) {
    return QJsonObject{{QStringLiteral("error"), QStringLiteral("ExtensionManager not available")}};
  }

  QString result = m_extensionManager->stopExtension(extensionId);

  if (!result.isEmpty()) {
    return QJsonObject{{QStringLiteral("error"), result}};
  }

  return QJsonObject{{QStringLiteral("status"), QStringLiteral("success")}};
}

QJsonObject DiagnosticsEndpoint::handleExtensionsRestartRequest(const QString& extensionId) {
  if (!m_extensionManager) {
    return QJsonObject{{QStringLiteral("error"), QStringLiteral("ExtensionManager not available")}};
  }

  QString result = m_extensionManager->restartExtension(extensionId);

  if (!result.isEmpty()) {
    return QJsonObject{{QStringLiteral("error"), result}};
  }

  QJsonObject info = m_extensionManager->getExtensionInfo(extensionId);
  return QJsonObject{{QStringLiteral("status"), QStringLiteral("success")},
                     {QStringLiteral("extension"), info}};
}
