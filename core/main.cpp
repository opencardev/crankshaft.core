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

#include "services/config/ConfigService.h"
#include "services/eventbus/EventBus.h"
#include "services/logging/Logger.h"
#include "services/websocket/WebSocketServer.h"
#include "hal/HALManager.h"
#include "hal/VehicleHAL.h"
#include "hal/HostHAL.h"

int main(int argc, char* argv[]) {
  QCoreApplication app(argc, argv);
  QCoreApplication::setApplicationName("Crankshaft Core");
  QCoreApplication::setApplicationVersion("0.1.0");

  // Parse command line arguments
  QCommandLineParser parser;
  parser.setApplicationDescription("Crankshaft Automotive Infotainment Core");
  parser.addHelpOption();
  parser.addVersionOption();

  QCommandLineOption portOption(QStringList() << "p" << "port", "WebSocket server port", "port", "8080");
  parser.addOption(portOption);

  QCommandLineOption configOption(QStringList() << "c" << "config", "Configuration file path", "config",
                                  "../config/crankshaft.json");
  parser.addOption(configOption);

  parser.process(app);

  // Initialize logger
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

  // Initialize HAL Manager (creates default mock HALs)
  HALManager &halManager = HALManager::instance();
  halManager.initialize(true);  // true = use default mock HALs
  Logger::instance().info("HAL Manager initialized with mock devices");

  // Connect HAL Manager vehicle property changes to EventBus
  QObject::connect(&halManager,
                   static_cast<void (HALManager::*)(VehiclePropertyType, const QVariant &)>(&HALManager::vehiclePropertyChanged),
                   [](VehiclePropertyType type, const QVariant &value) {
                     QString propertyName = VehicleHAL::propertyTypeToString(type);
                     QVariantMap payload;
                     payload["value"] = value;
                     EventBus::instance().publish("hal/vehicle/" + propertyName, payload);
                     Logger::instance().debug("Vehicle property updated: " + propertyName);
                   });

  // Connect HAL Manager host property changes to EventBus
  QObject::connect(&halManager,
                   static_cast<void (HALManager::*)(HostPropertyType, const QVariant &)>(&HALManager::hostPropertyChanged),
                   [](HostPropertyType type, const QVariant &value) {
                     QString propertyName = HostHAL::propertyTypeToString(type);
                     QVariantMap payload;
                     payload["value"] = value;
                     EventBus::instance().publish("hal/host/" + propertyName, payload);
                     Logger::instance().debug("Host property updated: " + propertyName);
                   });

  // Connect HAL errors to logger
  QObject::connect(&halManager, &HALManager::errorOccurred,
                   [](const QString &message) {
                     Logger::instance().error("HAL error: " + message);
                   });

  // Create WebSocket server
  WebSocketServer server(port);
  if (!server.isListening()) {
    Logger::instance().error("Failed to start server");
    return 1;
  }

  // Connect EventBus to WebSocket server (broadcasts all events including HAL)
  QObject::connect(&EventBus::instance(), &EventBus::messagePublished, &server, &WebSocketServer::broadcastEvent);

  Logger::instance().info("Crankshaft Core started successfully");

  return app.exec();
}
