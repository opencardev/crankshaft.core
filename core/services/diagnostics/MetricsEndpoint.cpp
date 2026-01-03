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

#include "MetricsEndpoint.h"

#include <unistd.h>

#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <algorithm>
#include <fstream>
#include <sstream>

namespace crankshaft {
namespace diagnostics {

// ============================================================================
// MetricTimeSeries Implementation
// ============================================================================

MetricTimeSeries::MetricTimeSeries(const QString& name, const QString& unit, int maxSamples)
    : m_name(name), m_unit(unit), m_maxSamples(maxSamples) {
  m_samples.reserve(maxSamples);
}

void MetricTimeSeries::addSample(double value) {
  addSample(QDateTime::currentMSecsSinceEpoch(), value);
}

void MetricTimeSeries::addSample(qint64 timestamp, double value) {
  QMutexLocker locker(&m_mutex);

  // Add sample
  m_samples.append(MetricSample(timestamp, value, m_unit));

  // Circular buffer: remove oldest if exceeds max
  if (m_samples.size() > m_maxSamples) {
    m_samples.removeFirst();
  }
}

QJsonObject MetricTimeSeries::toJson(int lastN) const {
  QMutexLocker locker(&m_mutex);

  QJsonObject json;
  json["name"] = m_name;
  json["unit"] = m_unit;
  json["sample_count"] = m_samples.size();

  // Determine samples to include
  int startIdx = 0;
  if (lastN > 0 && lastN < m_samples.size()) {
    startIdx = m_samples.size() - lastN;
  }

  QJsonArray samples;
  for (int i = startIdx; i < m_samples.size(); ++i) {
    QJsonObject sample;
    sample["timestamp"] = m_samples[i].timestamp;
    sample["value"] = m_samples[i].value;
    samples.append(sample);
  }

  json["samples"] = samples;
  json["latest"] = getLatest();
  json["average"] = getAverage(lastN);
  json["min"] = getMin(lastN);
  json["max"] = getMax(lastN);

  return json;
}

double MetricTimeSeries::getLatest() const {
  QMutexLocker locker(&m_mutex);
  return m_samples.isEmpty() ? 0.0 : m_samples.last().value;
}

double MetricTimeSeries::getAverage(int lastN) const {
  QMutexLocker locker(&m_mutex);

  if (m_samples.isEmpty()) return 0.0;

  int startIdx = 0;
  if (lastN > 0 && lastN < m_samples.size()) {
    startIdx = m_samples.size() - lastN;
  }

  double sum = 0.0;
  for (int i = startIdx; i < m_samples.size(); ++i) {
    sum += m_samples[i].value;
  }

  return sum / (m_samples.size() - startIdx);
}

double MetricTimeSeries::getMin(int lastN) const {
  QMutexLocker locker(&m_mutex);

  if (m_samples.isEmpty()) return 0.0;

  int startIdx = 0;
  if (lastN > 0 && lastN < m_samples.size()) {
    startIdx = m_samples.size() - lastN;
  }

  double minVal = m_samples[startIdx].value;
  for (int i = startIdx + 1; i < m_samples.size(); ++i) {
    if (m_samples[i].value < minVal) {
      minVal = m_samples[i].value;
    }
  }

  return minVal;
}

double MetricTimeSeries::getMax(int lastN) const {
  QMutexLocker locker(&m_mutex);

  if (m_samples.isEmpty()) return 0.0;

  int startIdx = 0;
  if (lastN > 0 && lastN < m_samples.size()) {
    startIdx = m_samples.size() - lastN;
  }

  double maxVal = m_samples[startIdx].value;
  for (int i = startIdx + 1; i < m_samples.size(); ++i) {
    if (m_samples[i].value > maxVal) {
      maxVal = m_samples[i].value;
    }
  }

  return maxVal;
}

int MetricTimeSeries::getSampleCount() const {
  QMutexLocker locker(&m_mutex);
  return m_samples.size();
}

// ============================================================================
// MetricsEndpoint Implementation
// ============================================================================

MetricsEndpoint::MetricsEndpoint(QObject* parent)
    : QObject(parent),
      m_collectionInterval(60000)  // 1 minute default
      ,
      m_isCollecting(false),
      m_maxHistorySamples(1440)  // 24 hours at 1-minute intervals
      ,
      m_prometheusEnabled(false),
      m_startTime(QDateTime::currentMSecsSinceEpoch()) {
  // Initialize time series
  m_memoryUsage = std::make_unique<MetricTimeSeries>("memory_usage", "MB", m_maxHistorySamples);
  m_cpuUsage = std::make_unique<MetricTimeSeries>("cpu_usage", "%", m_maxHistorySamples);
  m_activeConnections =
      std::make_unique<MetricTimeSeries>("active_connections", "count", m_maxHistorySamples);
  m_totalConnections =
      std::make_unique<MetricTimeSeries>("total_connections", "count", m_maxHistorySamples);
  m_requestLatency =
      std::make_unique<MetricTimeSeries>("request_latency", "ms", m_maxHistorySamples);

  // Setup collection timer
  m_collectionTimer = new QTimer(this);
  connect(m_collectionTimer, &QTimer::timeout, this, &MetricsEndpoint::collectMetrics);

  // Default alerts
  addAlert(MetricAlert("memory_usage", 1536.0, 2048.0, "Memory usage high"));
  addAlert(MetricAlert("cpu_usage", 70.0, 90.0, "CPU usage high"));
  addAlert(MetricAlert("active_connections", 50.0, 100.0, "Too many active connections"));
  addAlert(MetricAlert("request_latency", 500.0, 1000.0, "Request latency high"));
}

MetricsEndpoint::~MetricsEndpoint() {
  stopCollection();
}

void MetricsEndpoint::startCollection(int intervalMs) {
  if (m_isCollecting) {
    qWarning() << "Metrics collection already running";
    return;
  }

  m_collectionInterval = intervalMs;
  m_isCollecting = true;

  // Collect immediately
  collectMetrics();

  // Start periodic collection
  m_collectionTimer->start(m_collectionInterval);

  emit collectionStarted();
  qInfo() << "Metrics collection started (interval:" << m_collectionInterval << "ms)";
}

void MetricsEndpoint::stopCollection() {
  if (!m_isCollecting) {
    return;
  }

  m_collectionTimer->stop();
  m_isCollecting = false;

  emit collectionStopped();
  qInfo() << "Metrics collection stopped";
}

bool MetricsEndpoint::isCollecting() const {
  return m_isCollecting;
}

void MetricsEndpoint::recordMemoryUsage(double memoryMB) {
  m_memoryUsage->addSample(memoryMB);
}

void MetricsEndpoint::recordCpuUsage(double cpuPercent) {
  m_cpuUsage->addSample(cpuPercent);
}

void MetricsEndpoint::recordWebSocketConnections(int active, int total) {
  m_activeConnections->addSample(static_cast<double>(active));
  m_totalConnections->addSample(static_cast<double>(total));
}

void MetricsEndpoint::recordExtensionStatus(const QString& extensionId, const QString& status) {
  QMutexLocker locker(&m_extensionMutex);
  m_extensionStatus[extensionId] = status;
}

void MetricsEndpoint::recordRequestLatency(const QString& endpoint, double latencyMs) {
  QMutexLocker locker(&m_latencyMutex);

  // Store recent latency samples (max 100 per endpoint)
  if (!m_latencyByEndpoint.contains(endpoint)) {
    m_latencyByEndpoint[endpoint] = QVector<double>();
  }

  QVector<double>& latencies = m_latencyByEndpoint[endpoint];
  latencies.append(latencyMs);

  // Keep only last 100 samples
  if (latencies.size() > 100) {
    latencies.removeFirst();
  }

  // Update global latency metric (average across all endpoints)
  double totalLatency = 0.0;
  int sampleCount = 0;
  for (const auto& samples : m_latencyByEndpoint) {
    for (double latency : samples) {
      totalLatency += latency;
      ++sampleCount;
    }
  }

  if (sampleCount > 0) {
    m_requestLatency->addSample(totalLatency / sampleCount);
  }
}

QJsonObject MetricsEndpoint::getMetrics(int lastN) const {
  QMutexLocker locker(&m_mutex);

  QJsonObject metrics;
  metrics["timestamp"] = QDateTime::currentMSecsSinceEpoch();
  metrics["uptime_ms"] = QDateTime::currentMSecsSinceEpoch() - m_startTime;
  metrics["is_collecting"] = m_isCollecting;
  metrics["collection_interval_ms"] = m_collectionInterval;

  // Time series metrics
  metrics["memory"] = m_memoryUsage->toJson(lastN);
  metrics["cpu"] = m_cpuUsage->toJson(lastN);
  metrics["websocket_active"] = m_activeConnections->toJson(lastN);
  metrics["websocket_total"] = m_totalConnections->toJson(lastN);
  metrics["latency"] = m_requestLatency->toJson(lastN);

  // Extension status
  QJsonObject extensions;
  {
    QMutexLocker extLocker(&m_extensionMutex);
    for (auto it = m_extensionStatus.begin(); it != m_extensionStatus.end(); ++it) {
      extensions[it.key()] = it.value();
    }
  }
  metrics["extensions"] = extensions;

  // Latency breakdown by endpoint
  QJsonObject latencyBreakdown;
  {
    QMutexLocker latencyLocker(&m_latencyMutex);
    for (auto it = m_latencyByEndpoint.begin(); it != m_latencyByEndpoint.end(); ++it) {
      const QVector<double>& samples = it.value();
      if (!samples.isEmpty()) {
        QJsonObject endpointLatency;

        // Calculate percentiles
        QVector<double> sorted = samples;
        std::sort(sorted.begin(), sorted.end());

        int p50Idx = sorted.size() / 2;
        int p95Idx = static_cast<int>(sorted.size() * 0.95);
        int p99Idx = static_cast<int>(sorted.size() * 0.99);

        endpointLatency["p50"] = sorted[p50Idx];
        endpointLatency["p95"] = sorted[p95Idx];
        endpointLatency["p99"] = sorted[p99Idx];
        endpointLatency["count"] = samples.size();

        latencyBreakdown[it.key()] = endpointLatency;
      }
    }
  }
  metrics["latency_breakdown"] = latencyBreakdown;

  return metrics;
}

QJsonObject MetricsEndpoint::getMetricsSummary() const {
  QJsonObject summary;
  summary["timestamp"] = QDateTime::currentMSecsSinceEpoch();
  summary["uptime_seconds"] = (QDateTime::currentMSecsSinceEpoch() - m_startTime) / 1000;

  // Latest values
  summary["memory_mb"] = m_memoryUsage->getLatest();
  summary["cpu_percent"] = m_cpuUsage->getLatest();
  summary["active_connections"] = static_cast<int>(m_activeConnections->getLatest());
  summary["total_connections"] = static_cast<int>(m_totalConnections->getLatest());
  summary["avg_latency_ms"] = m_requestLatency->getLatest();

  // Statistics (last hour: 60 samples at 1-minute intervals)
  QJsonObject stats;
  stats["memory_avg"] = m_memoryUsage->getAverage(60);
  stats["memory_max"] = m_memoryUsage->getMax(60);
  stats["cpu_avg"] = m_cpuUsage->getAverage(60);
  stats["cpu_max"] = m_cpuUsage->getMax(60);
  summary["last_hour"] = stats;

  // Active alerts
  summary["active_alerts"] = getAlerts();

  return summary;
}

QJsonArray MetricsEndpoint::getAlerts() const {
  QMutexLocker locker(&m_alertMutex);

  QJsonArray alerts;

  for (const MetricAlert& alert : m_alerts) {
    if (!alert.enabled) continue;

    double currentValue = 0.0;
    if (alert.metricName == "memory_usage") {
      currentValue = m_memoryUsage->getLatest();
    } else if (alert.metricName == "cpu_usage") {
      currentValue = m_cpuUsage->getLatest();
    } else if (alert.metricName == "active_connections") {
      currentValue = m_activeConnections->getLatest();
    } else if (alert.metricName == "request_latency") {
      currentValue = m_requestLatency->getLatest();
    }

    if (currentValue >= alert.warningThreshold) {
      QJsonObject alertObj;
      alertObj["metric"] = alert.metricName;
      alertObj["current_value"] = currentValue;
      alertObj["warning_threshold"] = alert.warningThreshold;
      alertObj["critical_threshold"] = alert.criticalThreshold;
      alertObj["description"] = alert.description;

      if (currentValue >= alert.criticalThreshold) {
        alertObj["level"] = "CRITICAL";
      } else {
        alertObj["level"] = "WARNING";
      }

      alerts.append(alertObj);
    }
  }

  return alerts;
}

void MetricsEndpoint::addAlert(const MetricAlert& alert) {
  QMutexLocker locker(&m_alertMutex);

  // Remove existing alert with same name
  for (int i = 0; i < m_alerts.size(); ++i) {
    if (m_alerts[i].metricName == alert.metricName) {
      m_alerts.removeAt(i);
      break;
    }
  }

  m_alerts.append(alert);
  qInfo() << "Added alert:" << alert.metricName << "warning:" << alert.warningThreshold
          << "critical:" << alert.criticalThreshold;
}

void MetricsEndpoint::removeAlert(const QString& metricName) {
  QMutexLocker locker(&m_alertMutex);

  for (int i = 0; i < m_alerts.size(); ++i) {
    if (m_alerts[i].metricName == metricName) {
      m_alerts.removeAt(i);
      qInfo() << "Removed alert:" << metricName;
      return;
    }
  }
}

void MetricsEndpoint::enableAlert(const QString& metricName, bool enabled) {
  QMutexLocker locker(&m_alertMutex);

  for (MetricAlert& alert : m_alerts) {
    if (alert.metricName == metricName) {
      alert.enabled = enabled;
      qInfo() << "Alert" << metricName << (enabled ? "enabled" : "disabled");
      return;
    }
  }
}

QVector<MetricAlert> MetricsEndpoint::getActiveAlerts() const {
  QMutexLocker locker(&m_alertMutex);
  return m_alerts;
}

void MetricsEndpoint::setCollectionInterval(int intervalMs) {
  m_collectionInterval = intervalMs;

  if (m_isCollecting) {
    m_collectionTimer->setInterval(intervalMs);
  }
}

void MetricsEndpoint::setMaxHistorySamples(int maxSamples) {
  m_maxHistorySamples = maxSamples;
  // Note: This doesn't retroactively change existing time series
  // New time series will use this limit
}

void MetricsEndpoint::setPrometheusEnabled(bool enabled) {
  m_prometheusEnabled = enabled;
}

QString MetricsEndpoint::exportPrometheus() const {
  if (!m_prometheusEnabled) {
    return QString();
  }

  QString output;
  QTextStream stream(&output);

  // Memory usage
  stream << "# HELP crankshaft_memory_usage_mb Process memory usage in megabytes\n";
  stream << "# TYPE crankshaft_memory_usage_mb gauge\n";
  stream << "crankshaft_memory_usage_mb " << m_memoryUsage->getLatest() << "\n\n";

  // CPU usage
  stream << "# HELP crankshaft_cpu_usage_percent Process CPU usage percentage\n";
  stream << "# TYPE crankshaft_cpu_usage_percent gauge\n";
  stream << "crankshaft_cpu_usage_percent " << m_cpuUsage->getLatest() << "\n\n";

  // Active connections
  stream << "# HELP crankshaft_websocket_active_connections Active WebSocket connections\n";
  stream << "# TYPE crankshaft_websocket_active_connections gauge\n";
  stream << "crankshaft_websocket_active_connections " << m_activeConnections->getLatest()
         << "\n\n";

  // Request latency
  stream << "# HELP crankshaft_request_latency_ms Average request latency in milliseconds\n";
  stream << "# TYPE crankshaft_request_latency_ms gauge\n";
  stream << "crankshaft_request_latency_ms " << m_requestLatency->getLatest() << "\n\n";

  return output;
}

QJsonObject MetricsEndpoint::exportJson() const {
  return getMetrics(-1);  // Export all samples
}

QString MetricsEndpoint::exportCsv() const {
  QString csv;
  QTextStream stream(&csv);

  // Header
  stream << "timestamp,memory_mb,cpu_percent,active_connections,total_connections,latency_ms\n";

  // Data rows
  int sampleCount = m_memoryUsage->getSampleCount();
  for (int i = 0; i < sampleCount; ++i) {
    // Note: This is simplified - would need access to individual samples
    // In production, implement iterator or accessor for time series
  }

  return csv;
}

void MetricsEndpoint::collectMetrics() {
  // Collect system metrics
  double memoryMB = getCurrentMemoryUsageMB();
  double cpuPercent = getCurrentCpuPercent();
  int activeConns = getActiveWebSocketConnections();
  int totalConns = getTotalWebSocketConnections();

  // Record samples
  m_memoryUsage->addSample(memoryMB);
  m_cpuUsage->addSample(cpuPercent);
  m_activeConnections->addSample(static_cast<double>(activeConns));
  m_totalConnections->addSample(static_cast<double>(totalConns));

  // Check alerts
  checkAlerts();

  emit metricsCollected();
}

void MetricsEndpoint::checkAlerts() {
  QMutexLocker locker(&m_alertMutex);

  for (const MetricAlert& alert : m_alerts) {
    if (!alert.enabled) continue;

    double currentValue = 0.0;
    if (alert.metricName == "memory_usage") {
      currentValue = m_memoryUsage->getLatest();
    } else if (alert.metricName == "cpu_usage") {
      currentValue = m_cpuUsage->getLatest();
    } else if (alert.metricName == "active_connections") {
      currentValue = m_activeConnections->getLatest();
    } else if (alert.metricName == "request_latency") {
      currentValue = m_requestLatency->getLatest();
    }

    evaluateAlert(alert, currentValue);
  }
}

double MetricsEndpoint::getCurrentMemoryUsageMB() const {
  // Read from /proc/self/status
  std::ifstream status("/proc/self/status");
  if (!status.is_open()) {
    return 0.0;
  }

  std::string line;
  while (std::getline(status, line)) {
    if (line.find("VmRSS:") == 0) {
      std::istringstream iss(line);
      std::string label;
      int valueKB;
      std::string unit;
      iss >> label >> valueKB >> unit;
      return valueKB / 1024.0;  // Convert KB to MB
    }
  }

  return 0.0;
}

double MetricsEndpoint::getCurrentCpuPercent() const {
  // Read from /proc/self/stat
  std::ifstream stat("/proc/self/stat");
  if (!stat.is_open()) {
    return 0.0;
  }

  // Parse CPU times (simplified)
  std::string line;
  std::getline(stat, line);

  // In production, calculate CPU percentage from utime + stime
  // This requires tracking previous values and time deltas
  // For now, return placeholder
  return 0.0;
}

int MetricsEndpoint::getActiveWebSocketConnections() const {
  // TODO: Query WebSocketServer for active connection count
  return 0;
}

int MetricsEndpoint::getTotalWebSocketConnections() const {
  // TODO: Query WebSocketServer for total connection count
  return 0;
}

void MetricsEndpoint::evaluateAlert(const MetricAlert& alert, double currentValue) {
  if (currentValue >= alert.criticalThreshold) {
    emit alertTriggered(alert.metricName, "CRITICAL", currentValue);
    qCritical() << "ALERT [CRITICAL]:" << alert.description << "-" << alert.metricName << "="
                << currentValue << "(threshold:" << alert.criticalThreshold << ")";
  } else if (currentValue >= alert.warningThreshold) {
    emit alertTriggered(alert.metricName, "WARNING", currentValue);
    qWarning() << "ALERT [WARNING]:" << alert.description << "-" << alert.metricName << "="
               << currentValue << "(threshold:" << alert.warningThreshold << ")";
  }
}

}  // namespace diagnostics
}  // namespace crankshaft
