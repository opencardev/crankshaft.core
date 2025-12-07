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

#include <catch2/catch_all.hpp>

#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSignalSpy>
#include <QTest>
#include <QWebSocket>

#include "EventBus.h"
#include "WebSocketServer.h"

TEST_CASE("WebSocketServer starts and stops", "[websocket]") {
  int argc = 0;
  char* argv[] = {nullptr};
  QCoreApplication app(argc, argv);

  WebSocketServer server(8081);
  REQUIRE(server.isListening() == true);
}

TEST_CASE("WebSocketServer client connection", "[websocket]") {
  int argc = 0;
  char* argv[] = {nullptr};
  QCoreApplication app(argc, argv);

  WebSocketServer server(8082);
  QWebSocket client;

  QSignalSpy connectedSpy(&client, &QWebSocket::connected);

  client.open(QUrl("ws://localhost:8082"));

  REQUIRE(connectedSpy.wait(1000));
  REQUIRE(client.state() == QAbstractSocket::ConnectedState);

  client.close();
}

TEST_CASE("WebSocketServer subscribe message", "[websocket]") {
  int argc = 0;
  char* argv[] = {nullptr};
  QCoreApplication app(argc, argv);

  WebSocketServer server(8083);
  QWebSocket client;

  QSignalSpy connectedSpy(&client, &QWebSocket::connected);
  client.open(QUrl("ws://localhost:8083"));
  REQUIRE(connectedSpy.wait(1000));

  // Send subscribe message
  QJsonObject subscribeMsg;
  subscribeMsg["type"] = "subscribe";
  subscribeMsg["topic"] = "test/topic";

  QJsonDocument doc(subscribeMsg);
  client.sendTextMessage(doc.toJson(QJsonDocument::Compact));

  QTest::qWait(100);  // Allow time for processing

  // Server should have registered the subscription (no exception thrown)
  REQUIRE(client.state() == QAbstractSocket::ConnectedState);

  client.close();
}

TEST_CASE("WebSocketServer publish and broadcast", "[websocket]") {
  int argc = 0;
  char* argv[] = {nullptr};
  QCoreApplication app(argc, argv);

  WebSocketServer server(8084);
  QWebSocket client;

  QSignalSpy connectedSpy(&client, &QWebSocket::connected);
  QSignalSpy messageSpy(&client, &QWebSocket::textMessageReceived);

  client.open(QUrl("ws://localhost:8084"));
  REQUIRE(connectedSpy.wait(1000));

  // Subscribe to topic
  QJsonObject subscribeMsg;
  subscribeMsg["type"] = "subscribe";
  subscribeMsg["topic"] = "test/*";

  client.sendTextMessage(QJsonDocument(subscribeMsg).toJson(QJsonDocument::Compact));
  QTest::qWait(100);

  // Broadcast event
  QVariantMap payload;
  payload["message"] = "Hello World";
  server.broadcastEvent("test/event", payload);

  REQUIRE(messageSpy.wait(1000));
  REQUIRE(messageSpy.count() == 1);

  QString receivedMessage = messageSpy.at(0).at(0).toString();
  QJsonDocument receivedDoc = QJsonDocument::fromJson(receivedMessage.toUtf8());
  QJsonObject receivedObj = receivedDoc.object();

  REQUIRE(receivedObj["type"].toString() == "event");
  REQUIRE(receivedObj["topic"].toString() == "test/event");
  REQUIRE(receivedObj["payload"].toObject()["message"].toString() == "Hello World");

  client.close();
}

TEST_CASE("WebSocketServer multiple clients", "[websocket]") {
  int argc = 0;
  char* argv[] = {nullptr};
  QCoreApplication app(argc, argv);

  WebSocketServer server(8085);

  QWebSocket client1, client2;
  QSignalSpy connected1(&client1, &QWebSocket::connected);
  QSignalSpy connected2(&client2, &QWebSocket::connected);
  QSignalSpy message1(&client1, &QWebSocket::textMessageReceived);
  QSignalSpy message2(&client2, &QWebSocket::textMessageReceived);

  // Connect clients sequentially to avoid race conditions
  client1.open(QUrl("ws://localhost:8085"));
  REQUIRE(connected1.wait(2000));
  
  client2.open(QUrl("ws://localhost:8085"));
  REQUIRE(connected2.wait(2000));

  // Both subscribe to same topic
  QJsonObject subscribeMsg;
  subscribeMsg["type"] = "subscribe";
  subscribeMsg["topic"] = "broadcast/*";

  client1.sendTextMessage(QJsonDocument(subscribeMsg).toJson(QJsonDocument::Compact));
  client2.sendTextMessage(QJsonDocument(subscribeMsg).toJson(QJsonDocument::Compact));
  QTest::qWait(200);

  // Broadcast to both
  server.broadcastEvent("broadcast/test", {{"data", "test"}});

  // Check if messages already received or wait
  if (message1.count() == 0) {
    REQUIRE(message1.wait(2000));
  }
  if (message2.count() == 0) {
    REQUIRE(message2.wait(2000));
  }

  REQUIRE(message1.count() == 1);
  REQUIRE(message2.count() == 1);

  client1.close();
  client2.close();
}
