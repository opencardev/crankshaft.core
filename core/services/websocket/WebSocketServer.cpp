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
#include <QJsonDocument>
#include <QJsonObject>

#include "EventBus.h"
#include "Logger.h"

WebSocketServer::WebSocketServer(quint16 port, QObject* parent)
    : QObject(parent), m_server(new QWebSocketServer("CrankshaftCore", QWebSocketServer::NonSecureMode, this)) {
  Logger::instance().info(QString("Initializing WebSocket server on port %1...").arg(port));
  
  if (m_server->listen(QHostAddress::Any, port)) {
    Logger::instance().info(QString("WebSocket server listening on port %1").arg(port));
    connect(m_server, &QWebSocketServer::newConnection, this, &WebSocketServer::onNewConnection);
  } else {
    Logger::instance().error(QString("Failed to start WebSocket server on port %1: %2").arg(port).arg(m_server->errorString()));
  }
}

WebSocketServer::~WebSocketServer() {
  m_server->close();
  qDeleteAll(m_clients);
}

bool WebSocketServer::isListening() const { return m_server->isListening(); }

void WebSocketServer::onNewConnection() {
  QWebSocket* client = m_server->nextPendingConnection();

  Logger::instance().info(QString("New WebSocket connection from %1").arg(client->peerAddress().toString()));

  connect(client, &QWebSocket::textMessageReceived, this, &WebSocketServer::onTextMessageReceived);
  connect(client, &QWebSocket::disconnected, this, &WebSocketServer::onClientDisconnected);

  m_clients.append(client);
  m_subscriptions[client] = QStringList();
}

void WebSocketServer::onTextMessageReceived(const QString& message) {
  QWebSocket* client = qobject_cast<QWebSocket*>(sender());
  if (!client) return;

  QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
  if (!doc.isObject()) {
    Logger::instance().warning("Received invalid JSON message");
    return;
  }

  QJsonObject obj = doc.object();
  QString type = obj.value("type").toString();
  QString topic = obj.value("topic").toString();

  if (type == "subscribe") {
    handleSubscribe(client, topic);
  } else if (type == "publish") {
    QVariantMap payload = obj.value("payload").toObject().toVariantMap();
    handlePublish(topic, payload);
  }
}

void WebSocketServer::onClientDisconnected() {
  QWebSocket* client = qobject_cast<QWebSocket*>(sender());
  if (client) {
    Logger::instance().info(QString("Client disconnected: %1").arg(client->peerAddress().toString()));
    m_clients.removeOne(client);
    m_subscriptions.remove(client);
    client->deleteLater();
  }
}

void WebSocketServer::handleSubscribe(QWebSocket* client, const QString& topic) {
  if (!m_subscriptions[client].contains(topic)) {
    m_subscriptions[client].append(topic);
    Logger::instance().info(QString("Client subscribed to topic: %1").arg(topic));
  }
}

void WebSocketServer::handlePublish(const QString& topic, const QVariantMap& payload) {
  EventBus::instance().publish(topic, payload);
}

void WebSocketServer::broadcastEvent(const QString& topic, const QVariantMap& payload) {
  QJsonObject obj;
  obj["type"] = "event";
  obj["topic"] = topic;
  obj["payload"] = QJsonObject::fromVariantMap(payload);
  obj["timestamp"] = QDateTime::currentSecsSinceEpoch();

  QJsonDocument doc(obj);
  QString message = doc.toJson(QJsonDocument::Compact);

  for (auto* client : std::as_const(m_clients)) {
    bool shouldSend = false;
    for (const QString& subscription : std::as_const(m_subscriptions[client])) {
      if (topicMatches(topic, subscription)) {
        shouldSend = true;
        break;
      }
    }
    if (shouldSend) {
      client->sendTextMessage(message);
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

  return false;
}
