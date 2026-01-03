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

#include <QJsonDocument>
#include <QJsonObject>
#include <QTest>

// Mock WebSocketServer for testing validation logic
class WebSocketServerValidator {
 public:
  static bool validateMessage(const QJsonObject& obj, QString& error) {
    static const QSet<QString> allowedTypes = {QStringLiteral("subscribe"),
                                                QStringLiteral("unsubscribe"),
                                                QStringLiteral("publish"),
                                                QStringLiteral("service_command")};

    const QString type = obj.value("type").toString();
    if (type.isEmpty() || !allowedTypes.contains(type)) {
      error = QStringLiteral("invalid_type");
      return false;
    }

    // Validate required fields for each type
    if (type == "subscribe" || type == "unsubscribe" || type == "publish") {
      if (!obj.contains("topic") || obj.value("topic").toString().isEmpty()) {
        error = QStringLiteral("missing_topic");
        return false;
      }
    }

    if (type == "publish") {
      if (!obj.contains("payload") || !obj.value("payload").isObject()) {
        error = QStringLiteral("invalid_payload");
        return false;
      }
    }

    if (type == "service_command") {
      if (!obj.contains("command") || obj.value("command").toString().isEmpty()) {
        error = QStringLiteral("missing_command");
        return false;
      }
      if (!obj.contains("params") || !obj.value("params").isObject()) {
        error = QStringLiteral("missing_params");
        return false;
      }
    }

    return true;
  }

  static bool validateServiceCommand(const QString& command, QString& error) {
    static const QSet<QString> allowedCommands = {QStringLiteral("reload_services"),
                                                   QStringLiteral("start_service"),
                                                   QStringLiteral("stop_service"),
                                                   QStringLiteral("restart_service"),
                                                   QStringLiteral("get_running_services")};

    if (!allowedCommands.contains(command)) {
      error = QStringLiteral("unauthorised_command");
      return false;
    }
    return true;
  }
};

class TestWebSocketValidation : public QObject {
  Q_OBJECT

 private slots:
  // Subscribe message tests
  void testSubscribeValid() {
    QJsonObject msg;
    msg["type"] = "subscribe";
    msg["topic"] = "android-auto/status";
    QString error;
    QVERIFY(WebSocketServerValidator::validateMessage(msg, error));
  }

  void testSubscribeMissingTopic() {
    QJsonObject msg;
    msg["type"] = "subscribe";
    QString error;
    QVERIFY(!WebSocketServerValidator::validateMessage(msg, error));
    QCOMPARE(error, "missing_topic");
  }

  void testSubscribeEmptyTopic() {
    QJsonObject msg;
    msg["type"] = "subscribe";
    msg["topic"] = "";
    QString error;
    QVERIFY(!WebSocketServerValidator::validateMessage(msg, error));
    QCOMPARE(error, "missing_topic");
  }

  // Unsubscribe message tests
  void testUnsubscribeValid() {
    QJsonObject msg;
    msg["type"] = "unsubscribe";
    msg["topic"] = "android-auto/status";
    QString error;
    QVERIFY(WebSocketServerValidator::validateMessage(msg, error));
  }

  void testUnsubscribeMissingTopic() {
    QJsonObject msg;
    msg["type"] = "unsubscribe";
    QString error;
    QVERIFY(!WebSocketServerValidator::validateMessage(msg, error));
    QCOMPARE(error, "missing_topic");
  }

  // Publish message tests
  void testPublishValid() {
    QJsonObject payload;
    payload["data"] = "test";
    QJsonObject msg;
    msg["type"] = "publish";
    msg["topic"] = "test/topic";
    msg["payload"] = payload;
    QString error;
    QVERIFY(WebSocketServerValidator::validateMessage(msg, error));
  }

  void testPublishMissingTopic() {
    QJsonObject payload;
    payload["data"] = "test";
    QJsonObject msg;
    msg["type"] = "publish";
    msg["payload"] = payload;
    QString error;
    QVERIFY(!WebSocketServerValidator::validateMessage(msg, error));
    QCOMPARE(error, "missing_topic");
  }

  void testPublishMissingPayload() {
    QJsonObject msg;
    msg["type"] = "publish";
    msg["topic"] = "test/topic";
    QString error;
    QVERIFY(!WebSocketServerValidator::validateMessage(msg, error));
    QCOMPARE(error, "invalid_payload");
  }

  void testPublishInvalidPayload() {
    QJsonObject msg;
    msg["type"] = "publish";
    msg["topic"] = "test/topic";
    msg["payload"] = "not an object";  // String instead of object
    QString error;
    QVERIFY(!WebSocketServerValidator::validateMessage(msg, error));
    QCOMPARE(error, "invalid_payload");
  }

  // Service command tests
  void testServiceCommandValid() {
    QJsonObject params;
    params["service"] = "media";
    QJsonObject msg;
    msg["type"] = "service_command";
    msg["command"] = "start_service";
    msg["params"] = params;
    QString error;
    QVERIFY(WebSocketServerValidator::validateMessage(msg, error));
  }

  void testServiceCommandMissingCommand() {
    QJsonObject params;
    QJsonObject msg;
    msg["type"] = "service_command";
    msg["params"] = params;
    QString error;
    QVERIFY(!WebSocketServerValidator::validateMessage(msg, error));
    QCOMPARE(error, "missing_command");
  }

  void testServiceCommandMissingParams() {
    QJsonObject msg;
    msg["type"] = "service_command";
    msg["command"] = "start_service";
    QString error;
    QVERIFY(!WebSocketServerValidator::validateMessage(msg, error));
    QCOMPARE(error, "missing_params");
  }

  void testServiceCommandEmptyCommand() {
    QJsonObject params;
    QJsonObject msg;
    msg["type"] = "service_command";
    msg["command"] = "";
    msg["params"] = params;
    QString error;
    QVERIFY(!WebSocketServerValidator::validateMessage(msg, error));
    QCOMPARE(error, "missing_command");
  }

  // Invalid type tests
  void testInvalidMessageType() {
    QJsonObject msg;
    msg["type"] = "unknown_type";
    QString error;
    QVERIFY(!WebSocketServerValidator::validateMessage(msg, error));
    QCOMPARE(error, "invalid_type");
  }

  void testMissingMessageType() {
    QJsonObject msg;
    msg["topic"] = "test";
    QString error;
    QVERIFY(!WebSocketServerValidator::validateMessage(msg, error));
    QCOMPARE(error, "invalid_type");
  }

  // Service command allowlist tests
  void testAllowedServiceCommand_ReloadServices() {
    QString error;
    QVERIFY(WebSocketServerValidator::validateServiceCommand("reload_services", error));
  }

  void testAllowedServiceCommand_StartService() {
    QString error;
    QVERIFY(WebSocketServerValidator::validateServiceCommand("start_service", error));
  }

  void testAllowedServiceCommand_StopService() {
    QString error;
    QVERIFY(WebSocketServerValidator::validateServiceCommand("stop_service", error));
  }

  void testAllowedServiceCommand_RestartService() {
    QString error;
    QVERIFY(WebSocketServerValidator::validateServiceCommand("restart_service", error));
  }

  void testAllowedServiceCommand_GetRunningServices() {
    QString error;
    QVERIFY(WebSocketServerValidator::validateServiceCommand("get_running_services", error));
  }

  void testUnauthorisedServiceCommand() {
    QString error;
    QVERIFY(!WebSocketServerValidator::validateServiceCommand("delete_service", error));
    QCOMPARE(error, "unauthorised_command");
  }
};

QTEST_MAIN(TestWebSocketValidation)
#include "test_websocket_validation.moc"
