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

#include "WebSocketClient.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>

WebSocketClient::WebSocketClient(const QUrl& url, QObject* parent) : QObject(parent), m_socket(new QWebSocket()), m_url(url) {
  connect(m_socket, &QWebSocket::connected, this, &WebSocketClient::onConnected);
  connect(m_socket, &QWebSocket::disconnected, this, &WebSocketClient::onDisconnected);
  connect(m_socket, &QWebSocket::textMessageReceived, this, &WebSocketClient::onTextMessageReceived);
  connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::errorOccurred), this,
          &WebSocketClient::onError);

  qDebug() << "Connecting to" << url;
  m_socket->open(url);
}

bool WebSocketClient::isConnected() const { return m_socket->state() == QAbstractSocket::ConnectedState; }

void WebSocketClient::subscribe(const QString& topic) {
  if (!m_subscriptions.contains(topic)) {
    m_subscriptions.append(topic);
  }

  if (isConnected()) {
    QJsonObject obj;
    obj["type"] = "subscribe";
    obj["topic"] = topic;

    QJsonDocument doc(obj);
    m_socket->sendTextMessage(doc.toJson(QJsonDocument::Compact));
    qDebug() << "Subscribed to topic:" << topic;
  }
}

void WebSocketClient::unsubscribe(const QString& topic) {
  m_subscriptions.removeAll(topic);

  if (isConnected()) {
    QJsonObject obj;
    obj["type"] = "unsubscribe";
    obj["topic"] = topic;

    QJsonDocument doc(obj);
    m_socket->sendTextMessage(doc.toJson(QJsonDocument::Compact));
    qDebug() << "Unsubscribed from topic:" << topic;
  }
}

void WebSocketClient::publish(const QString& topic, const QVariantMap& payload) {
  if (!isConnected()) {
    qWarning() << "Cannot publish: not connected";
    return;
  }

  QJsonObject obj;
  obj["type"] = "publish";
  obj["topic"] = topic;
  obj["payload"] = QJsonObject::fromVariantMap(payload);

  QJsonDocument doc(obj);
  m_socket->sendTextMessage(doc.toJson(QJsonDocument::Compact));
  qDebug() << "Published to topic:" << topic;
}

void WebSocketClient::onConnected() {
  qDebug() << "WebSocket connected";
  emit connectedChanged();

  // Re-subscribe to all topics
  for (const auto& topic : std::as_const(m_subscriptions)) {
    subscribe(topic);
  }
}

void WebSocketClient::onDisconnected() {
  qDebug() << "WebSocket disconnected";
  emit connectedChanged();

  if (m_reconnectOnDisconnect) {
    QTimer::singleShot(2000, this, &WebSocketClient::reconnect);
  }
}

void WebSocketClient::onTextMessageReceived(const QString& message) {
  QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
  if (!doc.isObject()) {
    qWarning() << "Invalid JSON message received";
    return;
  }

  QJsonObject obj = doc.object();
  QString type = obj.value("type").toString();

  if (type == "event") {
    QString topic = obj.value("topic").toString();
    QVariantMap payload = obj.value("payload").toObject().toVariantMap();
    emit eventReceived(topic, payload);
  }
}

void WebSocketClient::onError(QAbstractSocket::SocketError error) {
  QString errorString = m_socket->errorString();
  qWarning() << "WebSocket error:" << error << errorString;
  emit errorOccurred(errorString);
}

void WebSocketClient::reconnect() {
  qDebug() << "Attempting to reconnect...";
  m_socket->open(m_url);
}
