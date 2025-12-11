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

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>

#include "services/android_auto/AndroidAutoService.h"
#include "services/config/ConfigService.h"
#include "services/eventbus/EventBus.h"
#include "services/logging/Logger.h"
#include "services/websocket/WebSocketServer.h"

int main(int argc, char* argv[]) {
  QCoreApplication app(argc, argv);
  QCoreApplication::setApplicationName("Crankshaft Core");
  QCoreApplication::setApplicationVersion("0.1.0");

  // Parse command line arguments
  QCommandLineParser parser;
  parser.setApplicationDescription("Crankshaft Automotive Infotainment Core");
  parser.addHelpOption();
  parser.addVersionOption();

  QCommandLineOption portOption(QStringList() << "p" << "port", "WebSocket server port", "port",
                                "8080");
  parser.addOption(portOption);

  QCommandLineOption configOption(QStringList() << "c" << "config", "Configuration file path",
                                  "config", "../config/crankshaft.json");
  parser.addOption(configOption);

  parser.process(app);

  // Initialise logger
  Logger::instance().setLevel(Logger::Level::Info);
  Logger::instance().info("Starting Crankshaft Core...");

  // Load configuration
  QString configPath = parser.value(configOption);
  if (!ConfigService::instance().load(configPath)) {
    Logger::instance().warning("Using default configuration");
  }

  // Get port from config or command line
  quint16 port = parser.value(portOption).toUInt();
  if (port == 0) {
    port = ConfigService::instance().get("core.websocket.port", 8080).toUInt();
  }

  // Initialise services
  Logger::instance().info("Initialising core services...");
  EventBus::instance();  // Initialise event bus
  Logger::instance().info("Event bus initialised");

  // Create WebSocket server
  WebSocketServer server(port);
  if (!server.isListening()) {
    Logger::instance().error("Failed to start WebSocket server on port " + QString::number(port));
    return 1;
  }
  Logger::instance().info("WebSocket server listening on port " + QString::number(port));

  // Connect EventBus to WebSocket server (broadcasts all events)
  QObject::connect(&EventBus::instance(), &EventBus::messagePublished, &server,
                   &WebSocketServer::broadcastEvent);

  Logger::instance().info("Crankshaft Core started successfully");
  Logger::instance().info("Core services ready");

  return app.exec();
}
