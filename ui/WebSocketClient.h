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

#include <QObject>
#include <QUrl>
#include <QVariantMap>
#include <QWebSocket>

class WebSocketClient : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool connected READ isConnected NOTIFY connectedChanged)

 public:
  explicit WebSocketClient(const QUrl& url, QObject* parent = nullptr);

  Q_INVOKABLE void subscribe(const QString& topic);
  Q_INVOKABLE void unsubscribe(const QString& topic);
  Q_INVOKABLE void publish(const QString& topic, const QVariantMap& payload);

  [[nodiscard]] bool isConnected() const;

 signals:
  void eventReceived(const QString& topic, const QVariantMap& payload);
  void connectedChanged();
  void errorOccurred(const QString& error);

 private slots:
  void onConnected();
  void onDisconnected();
  void onTextMessageReceived(const QString& message);
  void onError(QAbstractSocket::SocketError error);

 private:
  void reconnect();

  QWebSocket* m_socket;
  QUrl m_url;
  QStringList m_subscriptions;
  bool m_reconnectOnDisconnect{true};
};
