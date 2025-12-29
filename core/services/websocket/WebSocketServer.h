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

#include <QList>
#include <QObject>
#include <QWebSocket>
#include <QWebSocketServer>

// Forward declarations
class ServiceManager;

#include "../android_auto/AndroidAutoService.h"

class WebSocketServer : public QObject {
  Q_OBJECT

 public:
  explicit WebSocketServer(quint16 port, QObject* parent = nullptr);
  ~WebSocketServer() override;

  void broadcastEvent(const QString& topic, const QVariantMap& payload);
  [[nodiscard]] bool isListening() const;

  // ServiceManager integration
  void setServiceManager(ServiceManager* serviceManager);
  void initializeServiceConnections();  // Call after services are started

 private slots:
  void onNewConnection();
  void onTextMessageReceived(const QString& message);
  void onClientDisconnected();

  // Android Auto service events
  void onAndroidAutoStateChanged(int state);
  void onAndroidAutoConnected(const QVariantMap& device);
  void onAndroidAutoDisconnected();
  void onAndroidAutoError(const QString& error);

 private:
  void handleSubscribe(QWebSocket* client, const QString& topic);
  void handlePublish(const QString& topic, const QVariantMap& payload);
  void handleServiceCommand(QWebSocket* client, const QString& command, const QVariantMap& params);
  [[nodiscard]] bool topicMatches(const QString& topic, const QString& pattern) const;
  void setupAndroidAutoConnections();

  QWebSocketServer* m_server;
  QList<QWebSocket*> m_clients;
  QMap<QWebSocket*, QStringList> m_subscriptions;
  ServiceManager* m_serviceManager;
};
