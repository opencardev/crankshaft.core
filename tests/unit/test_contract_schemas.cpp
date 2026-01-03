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

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QTest>

/**
 * Simple JSON Schema validator implementation for contract testing.
 * Validates common schema patterns without external dependencies.
 */
class SimpleJsonSchemaValidator {
 public:
  static bool validateAgainstSchema(const QJsonObject& data, const QJsonObject& schema,
                                     QString& error) {
    // Check type constraint
    if (schema.contains("type")) {
      const QString expectedType = schema.value("type").toString();
      if (!validateType(data, expectedType, error)) {
        return false;
      }
    }

    // Check required fields
    if (schema.contains("required")) {
      const QJsonArray required = schema.value("required").toArray();
      for (const QJsonValue& field : required) {
        const QString fieldName = field.toString();
        if (!data.contains(fieldName)) {
          error = QStringLiteral("Missing required field: %1").arg(fieldName);
          return false;
        }
      }
    }

    // Check properties constraints
    if (schema.contains("properties")) {
      const QJsonObject properties = schema.value("properties").toObject();
      for (const QString& key : properties.keys()) {
        if (data.contains(key)) {
          const QJsonObject propSchema = properties.value(key).toObject();
          if (!validateAgainstSchema(QJsonObject{{key, data.value(key)}}, propSchema, error)) {
            error = QStringLiteral("Property '%1': %2").arg(key, error);
            return false;
          }
        }
      }
    }

    // Check enum constraint
    if (schema.contains("enum")) {
      const QJsonArray enumValues = schema.value("enum").toArray();
      bool found = false;
      for (const QJsonValue& val : enumValues) {
        if (data == val.toObject() || data.value("type") == val) {
          found = true;
          break;
        }
      }
      if (!found) {
        error = QStringLiteral("Value not in enum list");
        return false;
      }
    }

    return true;
  }

 private:
  static bool validateType(const QJsonObject& data, const QString& expectedType,
                          QString& error) {
    bool valid = false;
    if (expectedType == "object") {
      valid = data.isEmpty() || true;  // Any object is valid if type is "object"
    } else if (expectedType == "array") {
      error = QStringLiteral("Expected array type");
      return false;
    } else if (expectedType == "string") {
      error = QStringLiteral("Expected string type");
      return false;
    }
    return valid;
  }
};

class TestWebSocketContractSchema : public QObject {
  Q_OBJECT

 private slots:
  /**
   * Test WebSocket schema validation for message contracts.
   * Validates structure of subscribe, unsubscribe, publish, and service_command messages.
   */

  void testSubscribeMessageContract() {
    QJsonObject msg;
    msg["type"] = "subscribe";
    msg["topic"] = "android-auto/status";

    QJsonObject schema;
    schema["type"] = "object";
    QJsonArray required;
    required.append("type");
    required.append("topic");
    schema["required"] = required;

    QString error;
    QVERIFY2(SimpleJsonSchemaValidator::validateAgainstSchema(msg, schema, error),
             error.toLatin1());
  }

  void testUnsubscribeMessageContract() {
    QJsonObject msg;
    msg["type"] = "unsubscribe";
    msg["topic"] = "android-auto/status";

    QJsonObject schema;
    schema["type"] = "object";
    QJsonArray required;
    required.append("type");
    required.append("topic");
    schema["required"] = required;

    QString error;
    QVERIFY2(SimpleJsonSchemaValidator::validateAgainstSchema(msg, schema, error),
             error.toLatin1());
  }

  void testPublishMessageContract() {
    QJsonObject payload;
    payload["status"] = "active";
    QJsonObject msg;
    msg["type"] = "publish";
    msg["topic"] = "android-auto/status";
    msg["payload"] = payload;

    QJsonObject schema;
    schema["type"] = "object";
    QJsonArray required;
    required.append("type");
    required.append("topic");
    required.append("payload");
    schema["required"] = required;

    QString error;
    QVERIFY2(SimpleJsonSchemaValidator::validateAgainstSchema(msg, schema, error),
             error.toLatin1());
  }

  void testServiceCommandMessageContract() {
    QJsonObject params;
    params["service"] = "media";
    QJsonObject msg;
    msg["type"] = "service_command";
    msg["command"] = "start_service";
    msg["params"] = params;

    QJsonObject schema;
    schema["type"] = "object";
    QJsonArray required;
    required.append("type");
    required.append("command");
    required.append("params");
    schema["required"] = required;

    QString error;
    QVERIFY2(SimpleJsonSchemaValidator::validateAgainstSchema(msg, schema, error),
             error.toLatin1());
  }

  void testErrorResponseContract() {
    QJsonObject msg;
    msg["type"] = "error";
    msg["message"] = "Invalid message type";

    QJsonObject schema;
    schema["type"] = "object";
    QJsonArray required;
    required.append("type");
    required.append("message");
    schema["required"] = required;

    QString error;
    QVERIFY2(SimpleJsonSchemaValidator::validateAgainstSchema(msg, schema, error),
             error.toLatin1());
  }
};

class TestExtensionManifestContractSchema : public QObject {
  Q_OBJECT

 private slots:
  /**
   * Test extension manifest schema validation.
   * Validates required fields: name, version, entry_point, manifest_version.
   */

  void testValidExtensionManifest() {
    QJsonObject manifest;
    manifest["name"] = "media-player";
    manifest["version"] = "1.0.0";
    manifest["entry_point"] = "/usr/lib/crankshaft/extensions/media-player/lib.so";
    manifest["manifest_version"] = "1";
    manifest["description"] = "Media player extension";

    QJsonObject schema;
    schema["type"] = "object";
    QJsonArray required;
    required.append("name");
    required.append("version");
    required.append("entry_point");
    required.append("manifest_version");
    schema["required"] = required;

    QString error;
    QVERIFY2(SimpleJsonSchemaValidator::validateAgainstSchema(manifest, schema, error),
             error.toLatin1());
  }

  void testMissingRequiredField_Name() {
    QJsonObject manifest;
    manifest["version"] = "1.0.0";
    manifest["entry_point"] = "/path/to/lib.so";
    manifest["manifest_version"] = "1";

    QJsonObject schema;
    schema["type"] = "object";
    QJsonArray required;
    required.append("name");
    required.append("version");
    required.append("entry_point");
    required.append("manifest_version");
    schema["required"] = required;

    QString error;
    QVERIFY(!SimpleJsonSchemaValidator::validateAgainstSchema(manifest, schema, error));
    QVERIFY(error.contains("name"));
  }

  void testMissingRequiredField_EntryPoint() {
    QJsonObject manifest;
    manifest["name"] = "media-player";
    manifest["version"] = "1.0.0";
    manifest["manifest_version"] = "1";

    QJsonObject schema;
    schema["type"] = "object";
    QJsonArray required;
    required.append("name");
    required.append("version");
    required.append("entry_point");
    required.append("manifest_version");
    schema["required"] = required;

    QString error;
    QVERIFY(!SimpleJsonSchemaValidator::validateAgainstSchema(manifest, schema, error));
    QVERIFY(error.contains("entry_point"));
  }

  void testExtensionManifestWithDependencies() {
    QJsonArray dependencies;
    dependencies.append("qt6-core");
    dependencies.append("qt6-gui");

    QJsonObject manifest;
    manifest["name"] = "ui-extension";
    manifest["version"] = "1.0.0";
    manifest["entry_point"] = "/usr/lib/crankshaft/extensions/ui/lib.so";
    manifest["manifest_version"] = "1";
    manifest["dependencies"] = dependencies;

    QJsonObject schema;
    schema["type"] = "object";
    QJsonArray required;
    required.append("name");
    required.append("version");
    required.append("entry_point");
    required.append("manifest_version");
    schema["required"] = required;

    QString error;
    QVERIFY2(SimpleJsonSchemaValidator::validateAgainstSchema(manifest, schema, error),
             error.toLatin1());
  }

  void testExtensionManifestWithPermissions() {
    QJsonArray permissions;
    permissions.append("audio_device_access");
    permissions.append("event_bus_publish");

    QJsonObject manifest;
    manifest["name"] = "audio-extension";
    manifest["version"] = "1.0.0";
    manifest["entry_point"] = "/usr/lib/crankshaft/extensions/audio/lib.so";
    manifest["manifest_version"] = "1";
    manifest["permissions"] = permissions;

    QJsonObject schema;
    schema["type"] = "object";
    QJsonArray required;
    required.append("name");
    required.append("version");
    required.append("entry_point");
    required.append("manifest_version");
    schema["required"] = required;

    QString error;
    QVERIFY2(SimpleJsonSchemaValidator::validateAgainstSchema(manifest, schema, error),
             error.toLatin1());
  }
};

QTEST_MAIN(TestWebSocketContractSchema)
#include "test_contract_schemas.moc"
