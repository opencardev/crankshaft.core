# Performance Monitoring System

**Purpose**: Real-time metrics collection, historical analysis, and alert management for production monitoring  
**Component**: `MetricsEndpoint`  
**REST API**: `/metrics` endpoint  
**Update Frequency**: Configurable (default: 60 seconds)

---

## Overview

The Performance Monitoring System provides comprehensive visibility into Crankshaft's runtime behavior. It collects system metrics, stores historical data, evaluates alert thresholds, and exposes data via REST API for dashboards and monitoring tools.

### Key Features

- **Automatic Collection**: Background metric sampling at configurable intervals
- **Time Series Storage**: Circular buffer with 24-hour history (1440 samples @ 1min)
- **Alert System**: Configurable warning/critical thresholds with signal emission
- **Multiple Export Formats**: JSON, Prometheus, CSV
- **Thread-Safe**: Concurrent metric recording from multiple threads
- **Low Overhead**: <1% CPU, <50 MB memory for 24h history

---

## Quick Start

### Enable Metrics Collection

```cpp
#include "core/services/diagnostics/MetricsEndpoint.h"

using namespace crankshaft::diagnostics;

// Create metrics endpoint
MetricsEndpoint* metrics = new MetricsEndpoint(this);

// Start automatic collection (60-second intervals)
metrics->startCollection(60000);

// Connect to alerts
connect(metrics, &MetricsEndpoint::alertTriggered,
        [](const QString& metric, const QString& level, double value) {
    qWarning() << "Alert:" << level << metric << "=" << value;
});
```

### Query Current Metrics

```bash
# Get all metrics (last 60 samples)
curl http://localhost:9001/metrics

# Get summary (latest values + stats)
curl http://localhost:9001/metrics/summary

# Get active alerts
curl http://localhost:9001/metrics/alerts
```

### Manual Recording

```cpp
// Record custom metrics
metrics->recordMemoryUsage(256.5);  // MB
metrics->recordCpuUsage(45.3);      // %
metrics->recordWebSocketConnections(12, 150);
metrics->recordExtensionStatus("media-player", "running");
metrics->recordRequestLatency("/api/play", 85.2);  // ms
```

---

## Architecture

### Components

```
┌──────────────────────────────────────┐
│      MetricsEndpoint                 │
│  - Collection Timer                  │
│  - Alert Manager                     │
│  - Export Handlers                   │
└────────────┬─────────────────────────┘
             │
             ├──> MetricTimeSeries (Memory)
             ├──> MetricTimeSeries (CPU)
             ├──> MetricTimeSeries (Connections)
             ├──> MetricTimeSeries (Latency)
             └──> Extension Status Map
```

### Data Flow

```
1. Timer Triggers
   └─> collectMetrics()
       ├─> getCurrentMemoryUsageMB() (/proc/self/status)
       ├─> getCurrentCpuPercent() (/proc/self/stat)
       ├─> getActiveWebSocketConnections()
       └─> Store in TimeSeries (circular buffer)

2. Alert Evaluation
   └─> checkAlerts()
       ├─> Compare current values to thresholds
       ├─> Emit alertTriggered() signal if exceeded
       └─> Log warning/critical messages

3. REST API Query
   └─> getMetrics()
       ├─> Aggregate time series data
       ├─> Calculate statistics (avg, min, max)
       ├─> Format as JSON
       └─> Return to client
```

---

## API Reference

### MetricsEndpoint Class

#### Constructor

```cpp
explicit MetricsEndpoint(QObject* parent = nullptr);
```

Creates metrics endpoint with default configuration:
- Collection interval: 60 seconds
- Max history samples: 1440 (24 hours)
- Default alerts: memory, CPU, connections, latency

#### Collection Control

```cpp
void startCollection(int intervalMs = 60000);
void stopCollection();
bool isCollecting() const;
void setCollectionInterval(int intervalMs);
```

**Example**:
```cpp
metrics->startCollection(30000);  // 30-second intervals
// ... later ...
metrics->stopCollection();
```

#### Manual Recording

```cpp
void recordMemoryUsage(double memoryMB);
void recordCpuUsage(double cpuPercent);
void recordWebSocketConnections(int active, int total);
void recordExtensionStatus(const QString& extensionId, const QString& status);
void recordRequestLatency(const QString& endpoint, double latencyMs);
```

**Example**:
```cpp
// Record API request latency
auto start = std::chrono::steady_clock::now();
processRequest();
auto end = std::chrono::steady_clock::now();
double latencyMs = std::chrono::duration<double, std::milli>(end - start).count();
metrics->recordRequestLatency("/api/media/play", latencyMs);
```

#### Metric Retrieval

```cpp
QJsonObject getMetrics(int lastN = -1) const;
QJsonObject getMetricsSummary() const;
QJsonArray getAlerts() const;
```

**Example**:
```cpp
// Get last hour (60 samples @ 1min)
QJsonObject hourData = metrics->getMetrics(60);

// Get summary
QJsonObject summary = metrics->getMetricsSummary();
qDebug() << "Memory:" << summary["memory_mb"].toDouble() << "MB";
qDebug() << "CPU:" << summary["cpu_percent"].toDouble() << "%";
```

#### Alert Management

```cpp
void addAlert(const MetricAlert& alert);
void removeAlert(const QString& metricName);
void enableAlert(const QString& metricName, bool enabled);
QVector<MetricAlert> getActiveAlerts() const;
```

**Example**:
```cpp
// Add custom alert
MetricAlert alert("memory_usage", 1800.0, 2200.0, "High memory usage");
metrics->addAlert(alert);

// Disable alert temporarily
metrics->enableAlert("memory_usage", false);

// Remove alert
metrics->removeAlert("memory_usage");
```

#### Export Formats

```cpp
QString exportPrometheus() const;
QJsonObject exportJson() const;
QString exportCsv() const;
```

**Example**:
```cpp
// Export for Prometheus
QString prometheus = metrics->exportPrometheus();
// Serve at /metrics endpoint

// Export JSON for dashboard
QJsonObject jsonData = metrics->exportJson();
// Send to monitoring service

// Export CSV for analysis
QString csv = metrics->exportCsv();
// Save to file
```

#### Signals

```cpp
void metricsCollected();
void alertTriggered(const QString& metricName, const QString& level, double value);
void collectionStarted();
void collectionStopped();
```

**Example**:
```cpp
connect(metrics, &MetricsEndpoint::metricsCollected, []() {
    qDebug() << "Metrics updated";
});

connect(metrics, &MetricsEndpoint::alertTriggered,
        [](const QString& metric, const QString& level, double value) {
    // Send notification, log to file, trigger action
    if (level == "CRITICAL") {
        sendPagerDutyAlert(metric, value);
    }
});
```

---

## REST API Endpoints

### GET /metrics

Returns all metrics with time series data.

**Query Parameters**:
- `last_n` (optional): Number of recent samples to include

**Response** (JSON):
```json
{
  "timestamp": 1735932000000,
  "uptime_ms": 3600000,
  "is_collecting": true,
  "collection_interval_ms": 60000,
  "memory": {
    "name": "memory_usage",
    "unit": "MB",
    "sample_count": 60,
    "latest": 245.8,
    "average": 238.4,
    "min": 220.1,
    "max": 265.3,
    "samples": [
      {"timestamp": 1735928400000, "value": 220.1},
      {"timestamp": 1735928460000, "value": 225.3},
      ...
    ]
  },
  "cpu": {
    "name": "cpu_usage",
    "unit": "%",
    "latest": 18.5,
    "average": 15.2,
    "min": 8.3,
    "max": 42.1
  },
  "websocket_active": {
    "name": "active_connections",
    "unit": "count",
    "latest": 8,
    "average": 6.7
  },
  "latency": {
    "name": "request_latency",
    "unit": "ms",
    "latest": 45.2,
    "average": 52.3,
    "min": 28.1,
    "max": 185.4
  },
  "extensions": {
    "media-player": "running",
    "navigation": "running",
    "phone": "stopped"
  },
  "latency_breakdown": {
    "/api/media/play": {
      "p50": 42.1,
      "p95": 95.3,
      "p99": 145.8,
      "count": 250
    },
    "/api/settings/get": {
      "p50": 15.2,
      "p95": 35.1,
      "p99": 62.3,
      "count": 180
    }
  }
}
```

**Example**:
```bash
# Get all metrics
curl http://localhost:9001/metrics

# Get last 10 samples
curl http://localhost:9001/metrics?last_n=10
```

### GET /metrics/summary

Returns summary statistics (latest values + hourly stats).

**Response** (JSON):
```json
{
  "timestamp": 1735932000000,
  "uptime_seconds": 3600,
  "memory_mb": 245.8,
  "cpu_percent": 18.5,
  "active_connections": 8,
  "total_connections": 152,
  "avg_latency_ms": 45.2,
  "last_hour": {
    "memory_avg": 238.4,
    "memory_max": 265.3,
    "cpu_avg": 15.2,
    "cpu_max": 42.1
  },
  "active_alerts": [
    {
      "metric": "memory_usage",
      "current_value": 245.8,
      "warning_threshold": 220.0,
      "critical_threshold": 280.0,
      "description": "Memory usage high",
      "level": "WARNING"
    }
  ]
}
```

**Example**:
```bash
curl http://localhost:9001/metrics/summary
```

### GET /metrics/alerts

Returns active alerts only.

**Response** (JSON):
```json
[
  {
    "metric": "memory_usage",
    "current_value": 245.8,
    "warning_threshold": 220.0,
    "critical_threshold": 280.0,
    "description": "Memory usage high",
    "level": "WARNING"
  },
  {
    "metric": "cpu_usage",
    "current_value": 85.3,
    "warning_threshold": 70.0,
    "critical_threshold": 90.0,
    "description": "CPU usage high",
    "level": "CRITICAL"
  }
]
```

**Example**:
```bash
curl http://localhost:9001/metrics/alerts
```

### GET /metrics/prometheus

Returns Prometheus-compatible metrics export.

**Response** (Prometheus text format):
```
# HELP crankshaft_memory_usage_mb Process memory usage in megabytes
# TYPE crankshaft_memory_usage_mb gauge
crankshaft_memory_usage_mb 245.8

# HELP crankshaft_cpu_usage_percent Process CPU usage percentage
# TYPE crankshaft_cpu_usage_percent gauge
crankshaft_cpu_usage_percent 18.5

# HELP crankshaft_websocket_active_connections Active WebSocket connections
# TYPE crankshaft_websocket_active_connections gauge
crankshaft_websocket_active_connections 8

# HELP crankshaft_request_latency_ms Average request latency in milliseconds
# TYPE crankshaft_request_latency_ms gauge
crankshaft_request_latency_ms 45.2
```

**Example**:
```bash
curl http://localhost:9001/metrics/prometheus
```

---

## Metrics Reference

### System Metrics

#### memory_usage
- **Unit**: MB (megabytes)
- **Source**: `/proc/self/status` (VmRSS)
- **Description**: Resident Set Size (physical memory in use)
- **Default Alerts**: Warning @ 1536 MB, Critical @ 2048 MB
- **Target**: <1024 MB idle, <1536 MB active

#### cpu_usage
- **Unit**: % (percentage)
- **Source**: `/proc/self/stat` (utime + stime)
- **Description**: Process CPU utilization
- **Default Alerts**: Warning @ 70%, Critical @ 90%
- **Target**: <15% idle, <30% active

### Connection Metrics

#### active_connections
- **Unit**: count
- **Source**: WebSocketServer connection pool
- **Description**: Currently active WebSocket connections
- **Default Alerts**: Warning @ 50, Critical @ 100
- **Target**: <20 connections

#### total_connections
- **Unit**: count
- **Source**: WebSocketServer statistics
- **Description**: Total connections established since startup
- **Default Alerts**: None
- **Target**: N/A (monotonically increasing)

### Performance Metrics

#### request_latency
- **Unit**: ms (milliseconds)
- **Source**: Manual recording per endpoint
- **Description**: Average API request latency
- **Default Alerts**: Warning @ 500 ms, Critical @ 1000 ms
- **Target**: <200 ms (p95), <500 ms (p99)

#### Latency Percentiles (per endpoint)
- **p50** (median): 50% of requests faster than this
- **p95**: 95% of requests faster than this
- **p99**: 99% of requests faster than this

### Extension Metrics

#### extension_status
- **Type**: Map<extensionId, status>
- **Values**: "running", "stopped", "crashed", "starting"
- **Source**: ExtensionManager lifecycle events
- **Description**: Current state of each extension

---

## Alert Configuration

### Default Alerts

| Metric | Warning | Critical | Description |
|--------|---------|----------|-------------|
| memory_usage | 1536 MB | 2048 MB | Memory usage high |
| cpu_usage | 70% | 90% | CPU usage high |
| active_connections | 50 | 100 | Too many active connections |
| request_latency | 500 ms | 1000 ms | Request latency high |

### Custom Alerts

```cpp
// Add custom threshold
MetricAlert customAlert(
    "memory_usage",      // metric name
    1800.0,              // warning threshold
    2200.0,              // critical threshold
    "Custom memory alert"  // description
);
metrics->addAlert(customAlert);
```

### Alert Actions

```cpp
// Connect to alert signal
connect(metrics, &MetricsEndpoint::alertTriggered,
        [this](const QString& metric, const QString& level, double value) {
    if (level == "CRITICAL") {
        // Critical alert actions
        sendEmail("admin@example.com", "Critical Alert", 
                  QString("%1 = %2").arg(metric).arg(value));
        triggerPagerDuty(metric, value);
        logToFile("/var/log/crankshaft/alerts.log", metric, value);
    } else if (level == "WARNING") {
        // Warning alert actions
        logToFile("/var/log/crankshaft/warnings.log", metric, value);
    }
});
```

---

## Integration Examples

### Dashboard Integration

#### Grafana

**Prometheus Datasource**:
1. Enable Prometheus export: `metrics->setPrometheusEnabled(true);`
2. Add datasource in Grafana: `http://localhost:9001/metrics/prometheus`
3. Create dashboard with panels for each metric

**JSON API Datasource**:
1. Install JSON API plugin
2. Add datasource: `http://localhost:9001/metrics/summary`
3. Use JSONPath to extract values:
   - `$.memory_mb` → Memory gauge
   - `$.cpu_percent` → CPU gauge
   - `$.active_alerts` → Alert table

#### Custom Web Dashboard

```html
<!DOCTYPE html>
<html>
<head>
    <title>Crankshaft Metrics</title>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
</head>
<body>
    <canvas id="memoryChart"></canvas>
    <canvas id="cpuChart"></canvas>
    
    <script>
        // Fetch metrics every 10 seconds
        setInterval(async () => {
            const response = await fetch('http://localhost:9001/metrics?last_n=60');
            const data = await response.json();
            
            // Update charts
            updateChart(memoryChart, data.memory.samples);
            updateChart(cpuChart, data.cpu.samples);
        }, 10000);
    </script>
</body>
</html>
```

### Monitoring Service Integration

#### Datadog

```cpp
// Send metrics to Datadog
connect(metrics, &MetricsEndpoint::metricsCollected, [metrics]() {
    QJsonObject summary = metrics->getMetricsSummary();
    
    // Send via Datadog API
    QJsonObject payload;
    payload["series"] = QJsonArray{
        {{"metric", "crankshaft.memory.usage"},
         {"points", QJsonArray{{summary["timestamp"].toDouble() / 1000, 
                                summary["memory_mb"].toDouble()}}},
         {"type", "gauge"}},
        {{"metric", "crankshaft.cpu.usage"},
         {"points", QJsonArray{{summary["timestamp"].toDouble() / 1000, 
                                summary["cpu_percent"].toDouble()}}},
         {"type", "gauge"}}
    };
    
    sendToDatadog(payload);
});
```

#### New Relic

```cpp
// Send events to New Relic
connect(metrics, &MetricsEndpoint::alertTriggered,
        [](const QString& metric, const QString& level, double value) {
    QJsonObject event;
    event["eventType"] = "CrankshaftAlert";
    event["metric"] = metric;
    event["level"] = level;
    event["value"] = value;
    event["timestamp"] = QDateTime::currentSecsSinceEpoch();
    
    sendToNewRelic(event);
});
```

---

## Performance Impact

### Resource Usage

| Configuration | Memory | CPU | Disk I/O |
|---------------|--------|-----|----------|
| 1-minute sampling, 24h history | ~15 MB | <0.1% | None |
| 30-second sampling, 48h history | ~30 MB | <0.2% | None |
| 10-second sampling, 7d history | ~50 MB | <0.5% | None |

### Overhead Analysis

**Metric Collection**:
- Memory reading: ~50 µs (`/proc/self/status` read)
- CPU reading: ~50 µs (`/proc/self/stat` read + calculation)
- Time series insert: ~10 µs (vector append + circular buffer management)
- **Total per sample**: ~150 µs (~0.00015 seconds)

**At 60-second intervals**:
- CPU time per hour: 60 samples × 150 µs = 9 ms
- CPU percentage: 9 ms / 3,600,000 ms = 0.00025% ≈ **negligible**

**At 10-second intervals**:
- CPU time per hour: 360 samples × 150 µs = 54 ms
- CPU percentage: 54 ms / 3,600,000 ms = 0.0015% ≈ **<0.01%**

---

## Troubleshooting

### Issue: Metrics not updating

**Symptoms**:
```json
{
  "is_collecting": false,
  "sample_count": 0
}
```

**Solutions**:
1. Check collection started: `metrics->startCollection();`
2. Verify timer running: `metrics->isCollecting()` should return `true`
3. Check logs for errors during collection

### Issue: Memory readings always 0

**Symptoms**:
```json
{
  "memory": {"latest": 0.0, "average": 0.0}
}
```

**Solutions**:
1. Verify `/proc/self/status` accessible: `cat /proc/self/status | grep VmRSS`
2. Check permissions: metrics collection requires read access to `/proc`
3. Try manual recording: `metrics->recordMemoryUsage(100.0);`

### Issue: CPU readings inaccurate

**Symptoms**:
```json
{
  "cpu": {"latest": 0.0}
}
```

**Solutions**:
1. CPU calculation requires time delta between samples
2. First sample may be 0 (no previous value)
3. Verify `/proc/self/stat` readable: `cat /proc/self/stat`
4. Use external tool for validation: `top -p $(pgrep crankshaft-core)`

### Issue: Alerts not triggering

**Symptoms**:
- Metric exceeds threshold but no signal emitted

**Solutions**:
1. Check alert enabled: `metrics->getActiveAlerts()`
2. Verify signal connected: `connect(metrics, &MetricsEndpoint::alertTriggered, ...)`
3. Confirm threshold values: `alert.warningThreshold` and `alert.criticalThreshold`
4. Check metric name matches: "memory_usage", not "memory" or "memory_mb"

---

## Best Practices

### Collection Intervals

| Use Case | Recommended Interval | Reasoning |
|----------|----------------------|-----------|
| Production monitoring | 60 seconds | Balance between visibility and overhead |
| Development/debugging | 10-30 seconds | More granular data for issue diagnosis |
| Soak testing | 60 seconds | 24-hour history fits in ~15 MB memory |
| Performance profiling | 1-5 seconds | High-resolution data for short bursts |

### History Retention

| Interval | Max Samples | Memory | Duration |
|----------|-------------|--------|----------|
| 60s | 1440 | ~15 MB | 24 hours |
| 30s | 2880 | ~30 MB | 24 hours |
| 10s | 8640 | ~50 MB | 24 hours |
| 10s | 60480 | ~350 MB | 7 days |

### Alert Tuning

1. **Start with defaults**: Use default thresholds initially
2. **Collect baseline**: Run for 1 week to establish normal ranges
3. **Set warning at p95**: Warning threshold = 95th percentile of baseline
4. **Set critical at p99**: Critical threshold = 99th percentile + 10%
5. **Review weekly**: Adjust thresholds as system evolves

### Integration Patterns

**Real-time Dashboard**:
- Poll `/metrics/summary` every 5-10 seconds
- Display latest values + hourly trends
- Show active alerts prominently

**Historical Analysis**:
- Poll `/metrics?last_n=1440` every 5 minutes
- Store in time-series database (InfluxDB, Prometheus)
- Create retention policies (7d high-res, 30d aggregated, 1y summarized)

**Alert Actions**:
- WARNING: Log to file, increment counter
- CRITICAL: Send notification, create incident ticket

---

## Next Steps

After T068 completion:

1. ✅ **T068 Complete**: Performance profiling system operational
2. ⏳ **T069**: Validate quickstart on clean system
3. ⏳ **T070**: Write final README documentation

---

## References

- **Header**: `core/services/diagnostics/MetricsEndpoint.h`
- **Implementation**: `core/services/diagnostics/MetricsEndpoint.cpp`
- **REST API**: `contracts/openapi-diagnostics.yaml`
- **Soak Test**: `docs/SOAK_TEST_GUIDE.md`
- **Logging**: `docs/STRUCTURED_JSON_LOGGING.md`

---

*Guide version: 1.0*  
*Last updated: January 3, 2026*  
*Task: T068 - Performance Profiling Dashboard*
