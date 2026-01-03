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

#ifndef CRANKSHAFT_METRICS_ENDPOINT_H
#define CRANKSHAFT_METRICS_ENDPOINT_H

#include <QDateTime>
#include <QJsonArray>
#include <QJsonObject>
#include <QMutex>
#include <QObject>
#include <QString>
#include <QTimer>
#include <QVector>
#include <chrono>
#include <memory>

namespace crankshaft {
namespace diagnostics {

/**
 * @brief Metric data point with timestamp and value
 */
struct MetricSample {
  qint64 timestamp;  // Unix timestamp (milliseconds)
  double value;      // Metric value
  QString unit;      // Unit of measurement (MB, %, ms, etc.)

  MetricSample(qint64 ts, double val, const QString& u) : timestamp(ts), value(val), unit(u) {}
};

/**
 * @brief Metric time series with circular buffer
 */
class MetricTimeSeries {
 public:
  explicit MetricTimeSeries(const QString& name, const QString& unit, int maxSamples = 1440);

  void addSample(double value);
  void addSample(qint64 timestamp, double value);

  QJsonObject toJson(int lastN = -1) const;

  double getLatest() const;
  double getAverage(int lastN = -1) const;
  double getMin(int lastN = -1) const;
  double getMax(int lastN = -1) const;
  int getSampleCount() const;

  const QString& getName() const {
    return m_name;
  }
  const QString& getUnit() const {
    return m_unit;
  }

 private:
  QString m_name;
  QString m_unit;
  int m_maxSamples;
  QVector<MetricSample> m_samples;
  mutable QMutex m_mutex;
};

/**
 * @brief Alert configuration for metric thresholds
 */
struct MetricAlert {
  QString metricName;
  double warningThreshold;
  double criticalThreshold;
  bool enabled;
  QString description;

  MetricAlert(const QString& name, double warning, double critical, const QString& desc)
      : metricName(name),
        warningThreshold(warning),
        criticalThreshold(critical),
        enabled(true),
        description(desc) {}
};

/**
 * @brief Performance metrics endpoint for real-time monitoring
 *
 * Provides REST API endpoint (/metrics) for retrieving system metrics:
 * - Memory usage (RSS, heap)
 * - CPU utilization (process, system)
 * - WebSocket connections (active, total)
 * - Extension status (running, crashed)
 * - Request latency (p50, p95, p99)
 *
 * Features:
 * - Automatic metric collection (configurable interval)
 * - Historical data with circular buffer
 * - Alert thresholds with warning/critical levels
 * - JSON export for dashboards
 * - Prometheus-compatible format (optional)
 */
class MetricsEndpoint : public QObject {
  Q_OBJECT

 public:
  explicit MetricsEndpoint(QObject* parent = nullptr);
  ~MetricsEndpoint() override;

  // Metric collection control
  void startCollection(int intervalMs = 60000);  // Default: 1 minute
  void stopCollection();
  bool isCollecting() const;

  // Manual metric recording
  void recordMemoryUsage(double memoryMB);
  void recordCpuUsage(double cpuPercent);
  void recordWebSocketConnections(int active, int total);
  void recordExtensionStatus(const QString& extensionId, const QString& status);
  void recordRequestLatency(const QString& endpoint, double latencyMs);

  // Metric retrieval
  QJsonObject getMetrics(int lastN = -1) const;
  QJsonObject getMetricsSummary() const;
  QJsonArray getAlerts() const;

  // Alert management
  void addAlert(const MetricAlert& alert);
  void removeAlert(const QString& metricName);
  void enableAlert(const QString& metricName, bool enabled);
  QVector<MetricAlert> getActiveAlerts() const;

  // Configuration
  void setCollectionInterval(int intervalMs);
  void setMaxHistorySamples(int maxSamples);
  void setPrometheusEnabled(bool enabled);

  // Export formats
  QString exportPrometheus() const;
  QJsonObject exportJson() const;
  QString exportCsv() const;

 signals:
  void metricsCollected();
  void alertTriggered(const QString& metricName, const QString& level, double value);
  void collectionStarted();
  void collectionStopped();

 private slots:
  void collectMetrics();
  void checkAlerts();

 private:
  // System metrics collection
  double getCurrentMemoryUsageMB() const;
  double getCurrentCpuPercent() const;
  int getActiveWebSocketConnections() const;
  int getTotalWebSocketConnections() const;

  // Alert evaluation
  void evaluateAlert(const MetricAlert& alert, double currentValue);

  // Time series storage
  std::unique_ptr<MetricTimeSeries> m_memoryUsage;
  std::unique_ptr<MetricTimeSeries> m_cpuUsage;
  std::unique_ptr<MetricTimeSeries> m_activeConnections;
  std::unique_ptr<MetricTimeSeries> m_totalConnections;
  std::unique_ptr<MetricTimeSeries> m_requestLatency;

  // Collection control
  QTimer* m_collectionTimer;
  int m_collectionInterval;
  bool m_isCollecting;
  int m_maxHistorySamples;

  // Alert management
  QVector<MetricAlert> m_alerts;
  mutable QMutex m_alertMutex;

  // Configuration
  bool m_prometheusEnabled;
  qint64 m_startTime;  // Unix timestamp when collection started

  // Extension tracking
  QMap<QString, QString> m_extensionStatus;  // extensionId -> status
  mutable QMutex m_extensionMutex;

  // Request latency tracking
  QMap<QString, QVector<double>> m_latencyByEndpoint;  // endpoint -> latency samples
  mutable QMutex m_latencyMutex;

  // Thread safety
  mutable QMutex m_mutex;
};

}  // namespace diagnostics
}  // namespace crankshaft

#endif  // CRANKSHAFT_METRICS_ENDPOINT_H
