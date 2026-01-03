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

#include <QByteArray>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDateTime>
#include <QElapsedTimer>
#include <QString>
#include <aasdk/Common/ModernLogger.hpp>

#include "services/android_auto/AndroidAutoService.h"
#include "services/config/ConfigService.h"
#include "services/eventbus/EventBus.h"
#include "services/logging/Logger.h"
#include "services/profile/ProfileManager.h"
#include "services/service_manager/ServiceManager.h"
#include "services/websocket/WebSocketServer.h"
#if defined(__has_include)
#if __has_include("build_info.h")
#include "build_info.h"
#elif __has_include("../cmake/build_info.h")
#include "../cmake/build_info.h"
#else
#define CRANKSHAFT_BUILD_TIMESTAMP ""
#define CRANKSHAFT_GIT_COMMIT_SHORT "unknown"
#define CRANKSHAFT_GIT_COMMIT_LONG "unknown"
#define CRANKSHAFT_GIT_BRANCH "unknown"
#endif
#else
#include "build_info.h"
#endif

int main(int argc, char* argv[]) {
  // Start timing for cold-start performance measurement
  QElapsedTimer startupTimer;
  startupTimer.start();
  const qint64 startTimestampMs = QDateTime::currentMSecsSinceEpoch();

  QCoreApplication app(argc, argv);
  QCoreApplication::setApplicationName("Crankshaft Core");
  QCoreApplication::setApplicationVersion("0.1.0");

  // Log startup initiation with timestamp
  qInfo() << "[STARTUP]" << startTimestampMs << "ms: Core main() entry";

  // Quick argv scan for legacy/early CLI parsing: allow --verbose-usb or -v
  bool verboseUsbArgPresent = false;
  for (int i = 1; i < argc; ++i) {
    const QByteArray arg = QByteArray::fromRawData(argv[i], static_cast<int>(strlen(argv[i])));
    if (arg == "--verbose-usb" || arg == "-v") {
      verboseUsbArgPresent = true;
      break;
    }
  }

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

  // Long-only option to avoid short-name conflicts with existing options
  QCommandLineOption verboseUsbOption(
      QStringList() << "verbose-usb",
      "Enable verbose AASDK USB logging (or use env AASDK_VERBOSE_USB=1)");
  parser.addOption(verboseUsbOption);

  parser.process(app);

  // Enable AASDK verbose USB logging if requested via env var, CLI option or raw argv
  bool verboseUsb = false;
  const QByteArray ev = qgetenv("AASDK_VERBOSE_USB");
  if (!ev.isEmpty()) {
    QByteArray lower = ev.toLower();
    verboseUsb = (lower == "1" || lower == "true" || lower == "yes");
  }
  if (!verboseUsb && (parser.isSet(verboseUsbOption) || verboseUsbArgPresent)) {
    verboseUsb = true;
  }

  if (verboseUsb) {
    try {
      aasdk::common::ModernLogger::getInstance().setVerboseUsb(true);
    } catch (...) {
      // best effort - do not fail startup if logger not available
    }
  }

  // Initialise logger
  Logger::instance().setLevel(Logger::Level::Info);
  Logger::instance().info(
      QString("[STARTUP] %1ms elapsed: Starting Crankshaft Core...")
          .arg(startupTimer.elapsed()));

  // Log build details
  Logger::instance().info(
      QString("Build timestamp: %1, commit(short): %2, commit(long): %3, branch: %4")
          .arg(QString::fromUtf8(CRANKSHAFT_BUILD_TIMESTAMP))
          .arg(QString::fromUtf8(CRANKSHAFT_GIT_COMMIT_SHORT))
          .arg(QString::fromUtf8(CRANKSHAFT_GIT_COMMIT_LONG))
          .arg(QString::fromUtf8(CRANKSHAFT_GIT_BRANCH)));

  // Load configuration
  QString configPath = parser.value(configOption);
  if (!ConfigService::instance().load(configPath)) {
    Logger::instance().warning("Using default configuration");
  }
  Logger::instance().info(QString("[STARTUP] %1ms elapsed: Configuration loaded")
                              .arg(startupTimer.elapsed()));

  // Get port from config or command line
  quint16 port = parser.value(portOption).toUInt();
  if (port == 0) {
    port = ConfigService::instance().get("core.websocket.port", 8080).toUInt();
  }

  // Initialise services
  Logger::instance().info(
      QString("[STARTUP] %1ms elapsed: Initialising core services...")
          .arg(startupTimer.elapsed()));
  EventBus::instance();  // Initialise event bus
  Logger::instance().info(QString("[STARTUP] %1ms elapsed: Event bus initialised")
                              .arg(startupTimer.elapsed()));

  // Initialise ProfileManager
  Logger::instance().info(
      QString("[STARTUP] %1ms elapsed: Initialising ProfileManager...")
          .arg(startupTimer.elapsed()));
  QString profileConfigDir = ConfigService::instance()
                                 .get("core.profile.configDir", "/etc/crankshaft/profiles")
                                 .toString();
  ProfileManager profileManager(profileConfigDir);

  if (!profileManager.loadProfiles()) {
    Logger::instance().warning("Failed to load profiles, using default profiles");
  }

  HostProfile activeProfile = profileManager.getActiveHostProfile();
  Logger::instance().info(
      QString("[STARTUP] %1ms elapsed: Active host profile: %2 (%3)")
          .arg(startupTimer.elapsed())
          .arg(activeProfile.name, activeProfile.id));

  // Create WebSocket server
  Logger::instance().info(QString("[STARTUP] %1ms elapsed: Creating WebSocket server...")
                              .arg(startupTimer.elapsed()));
  WebSocketServer server(port);
  if (!server.isListening()) {
    Logger::instance().error("Failed to start WebSocket server on port " + QString::number(port));
    return 1;
  }
  Logger::instance().info(
      QString("[STARTUP] %1ms elapsed: WebSocket server listening on port %2")
          .arg(startupTimer.elapsed())
          .arg(port));

  // Connect EventBus to WebSocket server (broadcasts all events)
  QObject::connect(&EventBus::instance(), &EventBus::messagePublished, &server,
                   &WebSocketServer::broadcastEvent);

  // Create ServiceManager and start services
  Logger::instance().info(
      QString("[STARTUP] %1ms elapsed: Initialising ServiceManager...")
          .arg(startupTimer.elapsed()));
  ServiceManager serviceManager(&profileManager, &app);

  // Register ServiceManager with WebSocketServer for remote control
  server.setServiceManager(&serviceManager);

  Logger::instance().info(QString("[STARTUP] %1ms elapsed: Starting services based on profile...")
                              .arg(startupTimer.elapsed()));
  if (!serviceManager.startAllServices()) {
    Logger::instance().warning("No services started successfully");
  }
  Logger::instance().info(QString("[STARTUP] %1ms elapsed: Service initialisation complete")
                              .arg(startupTimer.elapsed()));

  // Initialize WebSocket connections to services (after services are started)
  server.initializeServiceConnections();

  Logger::instance().info(
      QString("[STARTUP] %1ms elapsed: Crankshaft Core started successfully")
          .arg(startupTimer.elapsed()));
  Logger::instance().info(
      QString("[STARTUP] READY - Total startup time: %1ms").arg(startupTimer.elapsed()));

  return app.exec();
}
