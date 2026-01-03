# Structured JSON Logging Guide

**Purpose**: Implement comprehensive structured logging for debugging, monitoring, and analysis

**Status**: Phase 8 Task T062 - Structured JSON Logging

---

## Overview

Crankshaft now provides structured JSON logging across all services, enabling:
- Machine-readable log analysis
- Efficient log aggregation and searching
- Contextual information capture (component, thread, custom fields)
- Automatic log rotation and retention
- Both JSON (structured) and text (human-readable) formats

---

## Quick Start

### Basic Usage (Backward Compatible)

```cpp
#include "core/services/logging/Logger.h"

Logger::instance().info("Application started");
Logger::instance().warning("Low memory detected");
Logger::instance().error("Connection failed");
```

### Structured Logging with Context

```cpp
// Log with structured context
QJsonObject context;
context["device_id"] = "device-123";
context["session_id"] = "session-456";
context["request_id"] = "req-789";

Logger::instance().infoContext("AndroidAutoService", 
                                "Device connected",
                                context);
```

### Configuration

```cpp
// Set log level (default: Info)
Logger::instance().setLevel(Logger::Level::Debug);

// Enable/disable JSON format (default: enabled)
Logger::instance().setJsonFormat(true);  // true for JSON, false for text

// Set log file (optional, logs to console if not set)
Logger::instance().setLogFile("/var/log/crankshaft/core.log");

// Set max log size for rotation (default: 10 MB)
Logger::instance().setMaxLogSize(50 * 1024 * 1024);  // 50 MB
```

---

## Log Format

### JSON Format (Structured)

```json
{
  "timestamp": "2026-01-03T12:34:56.789+00:00",
  "level": "INFO",
  "component": "AndroidAutoService",
  "message": "Device connected",
  "thread": "140734932707104",
  "device_id": "device-123",
  "session_id": "session-456",
  "request_id": "req-789"
}
```

**Fields**:
- `timestamp`: ISO 8601 timestamp with milliseconds
- `level`: One of DEBUG, INFO, WARNING, ERROR, FATAL
- `component`: Service/module name (e.g., "AndroidAutoService", "MediaService")
- `message`: Human-readable log message
- `thread`: Thread ID (hex representation)
- **Custom fields**: Any fields from context object (e.g., device_id, session_id)

### Text Format (Human-Readable)

```
[2026-01-03T12:34:56.789+00:00] INFO (AndroidAutoService): Device connected
```

---

## API Reference

### Logging Levels

| Level | Value | Use Case |
|-------|-------|----------|
| DEBUG | 0 | Detailed debugging information |
| INFO | 1 | Informational messages (default minimum) |
| WARNING | 2 | Warning conditions |
| ERROR | 3 | Error conditions |
| FATAL | 4 | Fatal conditions (severe errors) |

### Methods

#### Simple Logging (Backward Compatible)

```cpp
void debug(const QString& message);
void info(const QString& message);
void warning(const QString& message);
void error(const QString& message);
void fatal(const QString& message);
```

#### Structured Logging with Context

```cpp
void logStructured(Level level, const QString& component, 
                   const QString& message,
                   const QJsonObject& context = QJsonObject());

void debugContext(const QString& component, const QString& message,
                  const QJsonObject& context = QJsonObject());
void infoContext(const QString& component, const QString& message,
                 const QJsonObject& context = QJsonObject());
void warningContext(const QString& component, const QString& message,
                    const QJsonObject& context = QJsonObject());
void errorContext(const QString& component, const QString& message,
                  const QJsonObject& context = QJsonObject());
```

#### Configuration

```cpp
void setLevel(Level level);
void setLogFile(const QString& filePath);
void setJsonFormat(bool enabled);
void setMaxLogSize(qint64 bytes);
```

---

## Examples

### Example 1: Basic Logging

```cpp
#include "core/services/logging/Logger.h"

int main() {
    Logger::instance().info("Crankshaft Core started");
    Logger::instance().debug("Debug mode enabled");
    
    return 0;
}
```

**Output (JSON)**:
```json
{"timestamp":"2026-01-03T12:34:56","level":"INFO","component":"Crankshaft","message":"Crankshaft Core started","thread":"123"}
{"timestamp":"2026-01-03T12:34:57","level":"DEBUG","component":"Crankshaft","message":"Debug mode enabled","thread":"123"}
```

### Example 2: Android Auto Service Logging

```cpp
void AndroidAutoService::onDeviceConnected(const QString& deviceId) {
    QJsonObject context;
    context["device_id"] = deviceId;
    context["connection_type"] = "USB";
    context["timestamp_ms"] = QDateTime::currentMSecsSinceEpoch();
    
    Logger::instance().infoContext(
        "AndroidAutoService",
        "Device connected successfully",
        context
    );
}
```

**Output**:
```json
{"timestamp":"2026-01-03T12:34:58","level":"INFO","component":"AndroidAutoService","message":"Device connected successfully","thread":"456","device_id":"device-123","connection_type":"USB","timestamp_ms":1672746898123}
```

### Example 3: Error Logging with Request Context

```cpp
void DiagnosticsEndpoint::handleExtensionInstall(const QString& manifestPath) {
    QString requestId = generateUUID();
    
    QJsonObject context;
    context["request_id"] = requestId;
    context["manifest_path"] = manifestPath;
    context["user_agent"] = clientUserAgent();
    context["client_ip"] = clientIpAddress();
    
    try {
        parseAndInstallExtension(manifestPath);
        
        context["status"] = "success";
        context["install_time_ms"] = installTimer.elapsed();
        Logger::instance().infoContext("DiagnosticsEndpoint", 
                                        "Extension installed",
                                        context);
    } catch (const std::exception& e) {
        context["status"] = "error";
        context["error"] = e.what();
        Logger::instance().errorContext("DiagnosticsEndpoint",
                                         "Extension installation failed",
                                         context);
    }
}
```

**Output (Success)**:
```json
{"timestamp":"2026-01-03T12:34:59","level":"INFO","component":"DiagnosticsEndpoint","message":"Extension installed","thread":"789","request_id":"abc-123-def","manifest_path":"/opt/extensions/ext.json","user_agent":"curl/7.64.0","client_ip":"192.168.1.100","status":"success","install_time_ms":245}
```

**Output (Error)**:
```json
{"timestamp":"2026-01-03T12:35:00","level":"ERROR","component":"DiagnosticsEndpoint","message":"Extension installation failed","thread":"789","request_id":"abc-123-def","manifest_path":"/opt/extensions/bad.json","user_agent":"curl/7.64.0","client_ip":"192.168.1.100","status":"error","error":"Invalid manifest schema"}
```

### Example 4: Media Service with Performance Metrics

```cpp
void MediaService::play(const QString& trackId) {
    auto startTime = QDateTime::currentMSecsSinceEpoch();
    
    QJsonObject context;
    context["track_id"] = trackId;
    context["component"] = "MediaService::play";
    
    Logger::instance().debugContext("MediaService", "Starting playback", context);
    
    // ... playback logic ...
    
    auto elapsed = QDateTime::currentMSecsSinceEpoch() - startTime;
    context["duration_ms"] = static_cast<int>(elapsed);
    
    Logger::instance().infoContext("MediaService", "Playback started", context);
}
```

---

## Log Rotation

### Automatic Rotation

Logs automatically rotate when they exceed `maxLogSize` (default: 10 MB):

```cpp
Logger::instance().setMaxLogSize(50 * 1024 * 1024);  // 50 MB
```

When rotation occurs:
1. Current log is renamed: `core.log.20260103_123456`
2. New `core.log` file created
3. Old rotated files are cleaned up (keeps last 5)

### Manual Rotation Example

```cpp
void rotateLogsDaily() {
    QString logPath = "/var/log/crankshaft/core.log";
    QString rotatedPath = logPath + "." + QDate::currentDate().toString("yyyy-MM-dd");
    
    QFile::rename(logPath, rotatedPath);
    Logger::instance().setLogFile(logPath);  // Create fresh log file
}
```

---

## Log Analysis

### Viewing Logs

**JSON format** (parseable by tools):
```bash
cat /var/log/crankshaft/core.log | jq '.'
```

**Text format** (human-readable):
```bash
cat /var/log/crankshaft/core.log
```

### Filtering Logs

**By level** (using jq):
```bash
cat /var/log/crankshaft/core.log | jq 'select(.level == "ERROR")'
```

**By component**:
```bash
cat /var/log/crankshaft/core.log | jq 'select(.component == "AndroidAutoService")'
```

**By device ID** (in context):
```bash
cat /var/log/crankshaft/core.log | jq 'select(.device_id == "device-123")'
```

**By time range**:
```bash
cat /var/log/crankshaft/core.log | jq 'select(.timestamp > "2026-01-03T12:00:00" and .timestamp < "2026-01-03T13:00:00")'
```

### Log Aggregation

**Pipe to Elasticsearch** (example):
```bash
tail -f /var/log/crankshaft/core.log | \
  jq -R 'fromjson?' | \
  curl -X POST "localhost:9200/crankshaft/_doc" -H 'Content-Type: application/json' -d @-
```

**Send to syslog** (example):
```bash
tail -f /var/log/crankshaft/core.log | \
  logger -t crankshaft -p local0.info
```

---

## Configuration File

### crankshaft.conf Example

```ini
[logging]
# Log file path (empty = console only)
log_file=/var/log/crankshaft/core.log

# Log level: DEBUG, INFO, WARNING, ERROR, FATAL
log_level=INFO

# JSON format (true) or text format (false)
json_format=true

# Maximum log file size in bytes (0 = no rotation)
max_log_size=10485760

# Log retention (number of rotated files to keep)
log_retention=5
```

### Environment Variables

```bash
export CRANKSHAFT_LOG_FILE="/var/log/crankshaft/core.log"
export CRANKSHAFT_LOG_LEVEL="DEBUG"
export CRANKSHAFT_JSON_FORMAT="true"
export CRANKSHAFT_LOG_SIZE="10485760"
```

---

## Performance Considerations

### Log Level Impact

- **DEBUG**: Most verbose, highest I/O, use only during development
- **INFO**: Balanced, recommended for production
- **WARNING**: Only warnings and errors logged
- **ERROR**: Only errors logged
- **FATAL**: Only fatal errors logged

**Recommendation**: Use INFO for production, DEBUG only during troubleshooting.

### JSON vs Text Format

- **JSON**: Better for parsing and analysis, slightly larger file size
- **Text**: Human-readable, smaller file size, harder to parse

**Recommendation**: JSON for production servers, text for console during development.

### Log File I/O

Each log write is synchronous. For high-volume logging:

1. Use appropriate log level (INFO or higher)
2. Disable file logging if not needed (console only)
3. Implement buffering for burst traffic
4. Monitor disk I/O performance

---

## Migration Guide

### From Simple to Structured Logging

**Before**:
```cpp
Logger::instance().info("Extension installed: ext-123");
```

**After**:
```cpp
QJsonObject context;
context["extension_id"] = "ext-123";
context["version"] = "1.0.0";
Logger::instance().infoContext("ExtensionManager", 
                                "Extension installed",
                                context);
```

### Benefits
- Searchable structured data
- Consistent log format
- Rich contextual information
- Easy log aggregation

---

## Troubleshooting

### Logs Not Appearing

1. Check log file path is writable:
   ```bash
   touch /var/log/crankshaft/core.log
   ```

2. Check log level:
   ```cpp
   Logger::instance().setLevel(Logger::Level::Debug);
   ```

3. Verify JSON format is correct:
   ```cpp
   Logger::instance().setJsonFormat(false);  // Try text format
   ```

### Log File Growing Too Large

1. Reduce log level:
   ```cpp
   Logger::instance().setLevel(Logger::Level::Warning);
   ```

2. Reduce max size for more frequent rotation:
   ```cpp
   Logger::instance().setMaxLogSize(1 * 1024 * 1024);  // 1 MB
   ```

3. Implement log cleanup:
   ```bash
   find /var/log/crankshaft -name "core.log.*" -mtime +7 -delete
   ```

### JSON Parsing Errors

Ensure each log line is valid JSON:
```bash
cat /var/log/crankshaft/core.log | jq -r '.'
```

If errors appear, check for:
- Newlines in message fields (should be escaped)
- Invalid UTF-8 characters
- Incomplete JSON objects

---

## Best Practices

1. **Use context for correlation**: Include request IDs, session IDs, device IDs
2. **Appropriate log levels**: DEBUG for detailed info, INFO for important events, ERROR for failures
3. **Avoid logging sensitive data**: No passwords, tokens, or personal information
4. **Include timestamps in context**: For custom performance metrics
5. **Log at component boundaries**: Function entry/exit points, API calls, error conditions
6. **Structured context**: Use JSON objects for rich information, not string concatenation
7. **Performance awareness**: Debug logging can impact performance; use INFO for production

---

## References

- [JSON Logging Best Practices](https://jsonlogging.readthedocs.io/)
- [Structured Logging Concepts](https://www.kartar.net/2015/12/structured-logging/)
- [ELK Stack for Log Management](https://www.elastic.co/what-is/elk-stack)

---

**Status**: ✅ Phase 8 Task T062 Complete - Structured JSON Logging Implemented

**Next Task**: T063 - Contract Versioning Documentation
