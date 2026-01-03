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

#include "WebSocketServer.h"

#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "../android_auto/AndroidAutoService.h"
#include "../eventbus/EventBus.h"
#include "../logging/Logger.h"
#include "../service_manager/ServiceManager.h"

WebSocketServer::WebSocketServer(quint16 port, QObject* parent)
    : QObject(parent),
      m_server(new QWebSocketServer("CrankshaftCore", QWebSocketServer::NonSecureMode, this)),
      m_serviceManager(nullptr) {
  Logger::instance().info(QString("Initializing WebSocket server on port %1...").arg(port));

  if (m_server->listen(QHostAddress::Any, port)) {
    Logger::instance().info(QString("WebSocket server listening on port %1").arg(port));
    connect(m_server, &QWebSocketServer::newConnection, this, &WebSocketServer::onNewConnection);
  } else {
    Logger::instance().error(QString("Failed to start WebSocket server on port %1: %2")
                                 .arg(port)
                                 .arg(m_server->errorString()));
  }
}

WebSocketServer::~WebSocketServer() {
  m_server->close();
  qDeleteAll(m_clients);
}

bool WebSocketServer::isListening() const {
  return m_server->isListening();
}

void WebSocketServer::setServiceManager(ServiceManager* serviceManager) {
  m_serviceManager = serviceManager;
  Logger::instance().info("[WebSocketServer] ServiceManager registered");
}

void WebSocketServer::initializeServiceConnections() {
  if (!m_serviceManager) {
    Logger::instance().debug("[WebSocketServer] ServiceManager not available");
    return;
  }

  Logger::instance().info("[WebSocketServer] Initializing service connections...");
  setupAndroidAutoConnections();
}

void WebSocketServer::onNewConnection() {
  QWebSocket* client = m_server->nextPendingConnection();

  Logger::instance().info(
      QString("[WebSocketServer] New WebSocket connection from %1, Total clients: %2")
          .arg(client->peerAddress().toString())
          .arg(m_clients.size() + 1));

  connect(client, &QWebSocket::textMessageReceived, this, &WebSocketServer::onTextMessageReceived);
  connect(client, &QWebSocket::disconnected, this, &WebSocketServer::onClientDisconnected);

  m_clients.append(client);
  m_subscriptions[client] = QStringList();
}

void WebSocketServer::onTextMessageReceived(const QString& message) {
  QWebSocket* client = qobject_cast<QWebSocket*>(sender());
  if (!client) return;

  QJsonParseError parseError;
  QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8(), &parseError);
  if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
    Logger::instance().warning(
        QString("[WebSocketServer] Invalid JSON message: %1").arg(parseError.errorString()));
    sendError(client, QStringLiteral("invalid_json"));
    return;
  }

  QJsonObject obj = doc.object();
  QString error;
  if (!validateMessage(obj, error)) {
    Logger::instance().warning(QString("[WebSocketServer] Invalid message: %1").arg(error));
    sendError(client, error);
    return;
  }

  QString type = obj.value("type").toString();

  if (type == "subscribe") {
    QString topic = obj.value("topic").toString();
    handleSubscribe(client, topic);
  } else if (type == "unsubscribe") {
    QString topic = obj.value("topic").toString();
    handleUnsubscribe(client, topic);
  } else if (type == "publish") {
    QString topic = obj.value("topic").toString();
    QVariantMap payload = obj.value("payload").toObject().toVariantMap();
    handlePublish(topic, payload);
  } else if (type == "service_command") {
    QString command = obj.value("command").toString();
    QString commandError;
    if (!validateServiceCommand(command, commandError)) {
      Logger::instance().warning(
          QString("[WebSocketServer] Rejected service command: %1").arg(commandError));
      sendError(client, commandError);
      return;
    }
    QVariantMap params = obj.value("params").toObject().toVariantMap();
    handleServiceCommand(client, command, params);
  }
}

void WebSocketServer::onClientDisconnected() {
  QWebSocket* client = qobject_cast<QWebSocket*>(sender());
  if (client) {
    Logger::instance().info(
        QString("Client disconnected: %1").arg(client->peerAddress().toString()));
    m_clients.removeOne(client);
    m_subscriptions.remove(client);
    client->deleteLater();
  }
}

void WebSocketServer::handleSubscribe(QWebSocket* client, const QString& topic) {
  if (!m_subscriptions[client].contains(topic)) {
    m_subscriptions[client].append(topic);
    Logger::instance().info(QString("[WebSocketServer] Client subscribed to topic: %1").arg(topic));
    Logger::instance().info(QString("[WebSocketServer] Client now has %1 subscriptions")
                                .arg(m_subscriptions[client].size()));
    for (const auto& sub : std::as_const(m_subscriptions[client])) {
      Logger::instance().debug(QString("[WebSocketServer]   - %1").arg(sub));
    }

    // Send current Android Auto state when subscribing to android-auto topics
    if (topic.startsWith("android-auto") && m_serviceManager) {
      AndroidAutoService* aaService = m_serviceManager->getAndroidAutoService();
      if (aaService) {
        Logger::instance().info(
            "[WebSocketServer] Sending current Android Auto state to new subscriber");
        int currentState = static_cast<int>(aaService->getConnectionState());
        onAndroidAutoStateChanged(currentState);

        // If connected, also send device info
        if (aaService->isConnected()) {
          AndroidAutoService::AndroidDevice device = aaService->getConnectedDevice();
          QVariantMap deviceMap;
          deviceMap["serialNumber"] = device.serialNumber;
          deviceMap["manufacturer"] = device.manufacturer;
          deviceMap["model"] = device.model;
          deviceMap["androidVersion"] = device.androidVersion;
          deviceMap["connected"] = device.connected;
          onAndroidAutoConnected(deviceMap);
        }
      }
    }
  } else {
    Logger::instance().debug(
        QString("[WebSocketServer] Client already subscribed to: %1").arg(topic));
  }
}

void WebSocketServer::handlePublish(const QString& topic, const QVariantMap& payload) {
  EventBus::instance().publish(topic, payload);
}

void WebSocketServer::handleServiceCommand(QWebSocket* client, const QString& command,
                                           const QVariantMap& params) {
  if (!m_serviceManager) {
    Logger::instance().warning("[WebSocketServer] ServiceManager not available for command: " +
                               command);

    QJsonObject response;
    response["type"] = "service_response";
    response["command"] = command;
    response["success"] = false;
    response["error"] = "ServiceManager not available";
    client->sendTextMessage(QJsonDocument(response).toJson(QJsonDocument::Compact));
    return;
  }

  Logger::instance().info(QString("[WebSocketServer] Handling service command: %1").arg(command));

  QJsonObject response;
  response["type"] = "service_response";
  response["command"] = command;
  bool success = false;
  QString error;

  if (command == "reload_services") {
    m_serviceManager->reloadServices();
    success = true;
    Logger::instance().info("[WebSocketServer] Services reloaded via WebSocket command");
  } else if (command == "start_service") {
    QString serviceName = params.value("service").toString();
    if (!serviceName.isEmpty()) {
      success = m_serviceManager->startService(serviceName);
      Logger::instance().info(QString("[WebSocketServer] Start service '%1': %2")
                                  .arg(serviceName)
                                  .arg(success ? "success" : "failed"));
    } else {
      error = "Missing 'service' parameter";
    }
  } else if (command == "stop_service") {
    QString serviceName = params.value("service").toString();
    if (!serviceName.isEmpty()) {
      success = m_serviceManager->stopService(serviceName);
      Logger::instance().info(QString("[WebSocketServer] Stop service '%1': %2")
                                  .arg(serviceName)
                                  .arg(success ? "success" : "failed"));
    } else {
      error = "Missing 'service' parameter";
    }
  } else if (command == "restart_service") {
    QString serviceName = params.value("service").toString();
    if (!serviceName.isEmpty()) {
      success = m_serviceManager->restartService(serviceName);
      Logger::instance().info(QString("[WebSocketServer] Restart service '%1': %2")
                                  .arg(serviceName)
                                  .arg(success ? "success" : "failed"));
    } else {
      error = "Missing 'service' parameter";
    }
  } else if (command == "get_running_services") {
    QStringList services = m_serviceManager->getRunningServices();
    response["services"] = QJsonArray::fromStringList(services);
    success = true;
    Logger::instance().info(
        QString("[WebSocketServer] Running services query: %1").arg(services.join(", ")));
  } else {
    error = "Unknown command: " + command;
    Logger::instance().warning("[WebSocketServer] " + error);
  }

  response["success"] = success;
  if (!error.isEmpty()) {
    response["error"] = error;
  }
  response["timestamp"] = QDateTime::currentSecsSinceEpoch();

  client->sendTextMessage(QJsonDocument(response).toJson(QJsonDocument::Compact));
}

void WebSocketServer::broadcastEvent(const QString& topic, const QVariantMap& payload) {
  Logger::instance().info(
      QString("[WebSocketServer] broadcastEvent called - Topic: %1").arg(topic));
  Logger::instance().info(
      QString("[WebSocketServer] Payload keys: %1").arg(payload.keys().join(", ")));

  QJsonObject obj;
  obj["type"] = "event";
  obj["topic"] = topic;
  obj["payload"] = QJsonObject::fromVariantMap(payload);
  obj["timestamp"] = QDateTime::currentSecsSinceEpoch();

  QJsonDocument doc(obj);
  QString message = doc.toJson(QJsonDocument::Compact);

  Logger::instance().info(QString("[WebSocketServer] Message JSON: %1").arg(message));
  Logger::instance().info(
      QString("[WebSocketServer] Number of connected clients: %1").arg(m_clients.size()));

  for (auto* client : std::as_const(m_clients)) {
    bool shouldSend = false;
    Logger::instance().debug(QString("[WebSocketServer] Checking subscriptions for client %1")
                                 .arg(m_clients.indexOf(client)));

    for (const QString& subscription : std::as_const(m_subscriptions[client])) {
      Logger::instance().debug(
          QString("[WebSocketServer]   Subscription pattern: %1, Topic: %2, Match: %3")
              .arg(subscription)
              .arg(topic)
              .arg(topicMatches(topic, subscription) ? "YES" : "NO"));
      if (topicMatches(topic, subscription)) {
        shouldSend = true;
        break;
      }
    }

    if (shouldSend) {
      Logger::instance().info(
          QString("[WebSocketServer] Sending event to client (matched subscription)"));
      client->sendTextMessage(message);
    } else {
      Logger::instance().debug(QString("[WebSocketServer] Client has no matching subscription"));
    }
  }
}

bool WebSocketServer::topicMatches(const QString& topic, const QString& pattern) const {
  // Handle exact match
  if (topic == pattern) {
    return true;
  }

  // Handle wildcard '*' - matches everything
  if (pattern == "*") {
    return true;
  }

  // Handle pattern with wildcard like 'test/*'
  if (pattern.endsWith("/*")) {
    QString prefix = pattern.left(pattern.length() - 2);
    return topic.startsWith(prefix + "/");
  }

  // Handle MQTT-style '#' wildcard - matches everything including nested levels
  if (pattern.endsWith("/#")) {
    QString prefix = pattern.left(pattern.length() - 2);
    return topic.startsWith(prefix + "/");
  }

  return false;
}

void WebSocketServer::setupAndroidAutoConnections() {
  if (!m_serviceManager) {
    Logger::instance().warning(
        "[WebSocketServer] ServiceManager not set, cannot setup Android Auto connections");
    return;
  }

  AndroidAutoService* aaService = m_serviceManager->getAndroidAutoService();
  if (!aaService) {
    Logger::instance().warning(
        "[WebSocketServer] Android Auto service not available - will not broadcast Android Auto "
        "events");
    return;
  }

  Logger::instance().info(
      "[WebSocketServer] Setting up Android Auto service signal connections...");

  // Connect Android Auto service signals to WebSocket broadcast methods
  connect(aaService, &AndroidAutoService::connectionStateChanged, this,
          [this](AndroidAutoService::ConnectionState state) {
            onAndroidAutoStateChanged(static_cast<int>(state));
          });
  connect(aaService, &AndroidAutoService::connected, this,
          [this](const AndroidAutoService::AndroidDevice& device) {
            QVariantMap deviceMap;
            deviceMap["serialNumber"] = device.serialNumber;
            deviceMap["manufacturer"] = device.manufacturer;
            deviceMap["model"] = device.model;
            deviceMap["androidVersion"] = device.androidVersion;
            deviceMap["connected"] = device.connected;
            onAndroidAutoConnected(deviceMap);
          });
  connect(aaService, &AndroidAutoService::disconnected, this,
          &WebSocketServer::onAndroidAutoDisconnected);
  connect(aaService, &AndroidAutoService::errorOccurred, this,
          &WebSocketServer::onAndroidAutoError);

  Logger::instance().info("[WebSocketServer] Android Auto service connections setup");
}

void WebSocketServer::onAndroidAutoStateChanged(int state) {
  Logger::instance().info(QString("[WebSocketServer] Android Auto state changed: %1").arg(state));
  QVariantMap payload;
  payload["state"] = state;

  // Convert state enum to string
  static const QStringList stateNames = {"DISCONNECTED",   "SEARCHING", "CONNECTING",
                                         "AUTHENTICATING", "SECURING",  "CONNECTED",
                                         "DISCONNECTING",  "ERROR"};

  if (state >= 0 && state < stateNames.size()) {
    payload["stateName"] = stateNames[state];
    Logger::instance().info(
        QString("[WebSocketServer] Broadcasting state: %1").arg(stateNames[state]));
  }

  broadcastEvent("android-auto/status/state-changed", payload);
}

void WebSocketServer::onAndroidAutoConnected(const QVariantMap& device) {
  QVariantMap payload;
  payload["device"] = device;
  payload["connected"] = true;
  broadcastEvent("android-auto/status/connected", payload);
}

void WebSocketServer::onAndroidAutoDisconnected() {
  QVariantMap payload;
  payload["connected"] = false;
  broadcastEvent("android-auto/status/disconnected", payload);
}

void WebSocketServer::onAndroidAutoError(const QString& error) {
  QVariantMap payload;
  payload["error"] = error;
  broadcastEvent("android-auto/status/error", payload);
}

bool WebSocketServer::validateMessage(const QJsonObject& obj, QString& error) const {
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

bool WebSocketServer::validateServiceCommand(const QString& command, QString& error) const {
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

void WebSocketServer::sendError(QWebSocket* client, const QString& message) const {
  if (!client) {
    return;
  }

  QJsonObject errorObj;
  errorObj["type"] = "error";
  errorObj["message"] = message;
  QJsonDocument doc(errorObj);
  client->sendTextMessage(doc.toJson(QJsonDocument::Compact));
  Logger::instance().debug(QString("[WebSocketServer] Sent error to client: %1").arg(message));
}

void WebSocketServer::handleUnsubscribe(QWebSocket* client, const QString& topic) {
  if (!m_subscriptions.contains(client)) {
    Logger::instance().warning("[WebSocketServer] Unsubscribe from unknown client");
    sendError(client, QStringLiteral("client_not_found"));
    return;
  }

  if (!m_subscriptions[client].contains(topic)) {
    Logger::instance().debug(QString("[WebSocketServer] Client not subscribed to: %1").arg(topic));
    sendError(client, QStringLiteral("not_subscribed"));
    return;
  }

  m_subscriptions[client].removeOne(topic);
  Logger::instance().info(QString("[WebSocketServer] Client unsubscribed from topic: %1").arg(topic));
}
