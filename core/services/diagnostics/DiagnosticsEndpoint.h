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
#include <memory>

class EventBus;
class ServiceManager;
class Logger;
class ExtensionManager;

/**
 * Diagnostics REST Endpoint
 *
 * Provides operational diagnostics endpoints for monitoring and troubleshooting
 * Crankshaft core application health, metrics, and extension status.
 *
 * Endpoints:
 * - GET /health        : Application health status
 * - GET /metrics       : Performance and resource metrics
 * - GET /extensions    : List of installed/loaded extensions
 * - POST /extensions   : Reload extension registry
 */
class DiagnosticsEndpoint : public QObject {
  Q_OBJECT

 public:
  explicit DiagnosticsEndpoint(EventBus* eventBus, ServiceManager* serviceManager,
                                Logger* logger, QObject* parent = nullptr);
  ~DiagnosticsEndpoint() override = default;

  /**
   * Initialise the diagnostics endpoint
   * @return true if successfully initialised, false otherwise
   */
  bool init();

  /**
   * Shutdown the diagnostics endpoint
   */
  void shutdown();

 signals:
  /**
   * Emitted when an API request is received
   * @param endpoint The endpoint path (e.g., "/health")
   * @param method HTTP method (GET, POST, etc.)
   */
  void requestReceived(const QString& endpoint, const QString& method);

 public slots:
  /**
   * Handle GET /health request
   * Returns JSON with application status, startup time, and uptime
   */
  QJsonObject handleHealthCheck();

  /**
   * Handle GET /metrics request
   * Returns JSON with CPU usage, memory usage, and active extensions count
   */
  QJsonObject handleMetricsRequest();

  /**
   * Handle GET /extensions request
   * Returns JSON array with installed extensions and their status
   */
  QJsonObject handleExtensionsListRequest();

  /**
   * Handle POST /extensions request
   * Installs or updates an extension from JSON manifest
   * Request body: { "manifest": "<json manifest>", "target_dir": "<optional dir>" }
   */
  QJsonObject handleExtensionsInstallRequest(const QJsonObject& requestBody);

  /**
   * Handle DELETE /extensions/{id} request
   * Uninstalls an extension
   */
  QJsonObject handleExtensionsUninstallRequest(const QString& extensionId);

  /**
   * Handle POST /extensions/{id}/start request
   * Starts an extension process
   */
  QJsonObject handleExtensionsStartRequest(const QString& extensionId);

  /**
   * Handle POST /extensions/{id}/stop request
   * Stops an extension process
   */
  QJsonObject handleExtensionsStopRequest(const QString& extensionId);

  /**
   * Handle POST /extensions/{id}/restart request
   * Restarts an extension process
   */
  QJsonObject handleExtensionsRestartRequest(const QString& extensionId);

  /**
   * Handle POST /extensions request (legacy)
   * Triggers extension registry reload
   */
  QJsonObject handleExtensionsReloadRequest();

 private:
  /**
   * Gather system health information
   */
  QJsonObject gatherHealthStatus();

  /**
   * Gather performance metrics
   */
  QJsonObject gatherMetrics();

  /**
   * Get list of loaded extensions with metadata
   */
  QJsonObject gatherExtensionsList();

  /**
   * Calculate CPU usage percentage
   */
  float calculateCpuUsage();

  /**
   * Calculate memory usage in MB
   */
  float calculateMemoryUsage();

  EventBus* m_eventBus;
  ServiceManager* m_serviceManager;
  Logger* m_logger;
  ExtensionManager* m_extensionManager;

  // Timing for uptime calculation
  qint64 m_startTime;
};
