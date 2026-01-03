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
#include <QDateTime>
#include <QElapsedTimer>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTranslator>

#include "Theme.h"
#include "WebSocketClient.h"
#include "build_info.h"

int main(int argc, char* argv[]) {
  // Start timing for cold-start performance measurement
  QElapsedTimer startupTimer;
  startupTimer.start();
  const qint64 startTimestampMs = QDateTime::currentMSecsSinceEpoch();

  QGuiApplication app(argc, argv);
  QGuiApplication::setApplicationName("Crankshaft UI");
  QGuiApplication::setApplicationVersion("0.1.0");

  // Log startup initiation with timestamp
  qInfo() << "[STARTUP]" << startTimestampMs << "ms: UI main() entry";
  qInfo() << "[STARTUP]" << startupTimer.elapsed() << "ms elapsed: QGuiApplication created";

  // Parse command line arguments
  QCommandLineParser parser;
  parser.setApplicationDescription("Crankshaft Automotive Infotainment UI");
  parser.addHelpOption();
  parser.addVersionOption();

  QCommandLineOption serverOption(QStringList() << "s" << "server", "WebSocket server URL",
                                  "server", "ws://localhost:8080");
  parser.addOption(serverOption);

  QCommandLineOption languageOption(QStringList() << "l" << "language",
                                    "UI language (en-GB, de-DE)", "language", "en-GB");
  parser.addOption(languageOption);

  parser.process(app);

  qInfo() << "[STARTUP]" << startupTimer.elapsed() << "ms elapsed: Command line parsed";

  // Set up translator (heap-allocated for proper lifetime)
  QTranslator* translator = new QTranslator(&app);
  QString currentLanguage = parser.value(languageOption);
  QString translationFile = QString(":/i18n/ui_%1").arg(QString(currentLanguage).replace('-', '_'));
  if (translator->load(translationFile)) {
    app.installTranslator(translator);
    qDebug() << "[STARTUP]" << startupTimer.elapsed()
             << "ms elapsed: Loaded translation:" << translationFile;
  } else {
    qWarning() << "[STARTUP]" << startupTimer.elapsed()
               << "ms elapsed: Failed to load translation:" << translationFile;
  }

  // Create WebSocket client
  QString serverUrl = parser.value(serverOption);
  qInfo() << "[STARTUP]" << startupTimer.elapsed()
          << "ms elapsed: Creating WebSocket client for:" << serverUrl;
  WebSocketClient* wsClient = new WebSocketClient(QUrl(serverUrl));

  // Subscribe to common topics
  wsClient->subscribe("ui/*");
  wsClient->subscribe("system/*");
  qInfo() << "[STARTUP]" << startupTimer.elapsed() << "ms elapsed: WebSocket client created";

  // Create QML engine
  qInfo() << "[STARTUP]" << startupTimer.elapsed() << "ms elapsed: Creating QML engine";
  QQmlApplicationEngine engine;

  // Create Theme instance
  Theme* theme = new Theme(&app);

  // Set context properties (Theme as global object, not singleton)
  qInfo() << "[STARTUP]" << startupTimer.elapsed() << "ms elapsed: Setting context properties";
  engine.rootContext()->setContextProperty("Theme", theme);
  engine.rootContext()->setContextProperty("wsClient", wsClient);
  engine.rootContext()->setContextProperty("currentLanguage", currentLanguage);

  // Expose build info to QML
  engine.rootContext()->setContextProperty("buildTimestamp",
                                           QString::fromUtf8(CRANKSHAFT_BUILD_TIMESTAMP));
  engine.rootContext()->setContextProperty("buildCommitShort",
                                           QString::fromUtf8(CRANKSHAFT_GIT_COMMIT_SHORT));
  engine.rootContext()->setContextProperty("buildCommitLong",
                                           QString::fromUtf8(CRANKSHAFT_GIT_COMMIT_LONG));
  engine.rootContext()->setContextProperty("buildBranch", QString::fromUtf8(CRANKSHAFT_GIT_BRANCH));

  qInfo() << "[STARTUP]" << startupTimer.elapsed() << "ms elapsed: UI Build:"
          << QString::fromUtf8(CRANKSHAFT_BUILD_TIMESTAMP)
          << "commit(short):" << QString::fromUtf8(CRANKSHAFT_GIT_COMMIT_SHORT)
          << "branch:" << QString::fromUtf8(CRANKSHAFT_GIT_BRANCH);

  // Handle language change events
  QObject::connect(wsClient, &WebSocketClient::eventReceived,
                   [&, translator](const QString& topic, const QVariantMap& payload) {
                     if (topic == "ui/language/changed") {
                       QString newLanguage = payload.value("language").toString();
                       qDebug() << "Language change requested:" << newLanguage;

                       // Remove old translator
                       app.removeTranslator(translator);

                       // Load new translator (convert hyphens to underscores for file path)
                       QString translationFile =
                           QString(":/i18n/ui_%1").arg(QString(newLanguage).replace('-', '_'));
                       if (translator->load(translationFile)) {
                         app.installTranslator(translator);
                         qDebug() << "Translation loaded:" << translationFile;

                         // Update context property
                         currentLanguage = newLanguage;
                         engine.rootContext()->setContextProperty("currentLanguage",
                                                                  currentLanguage);

                         // Trigger retranslation
                         engine.retranslate();
                         qDebug() << "UI retranslated";
                       } else {
                         qWarning() << "Failed to load translation:" << translationFile;
                       }
                     }
                   });

  // Load the module
  qInfo() << "[STARTUP]" << startupTimer.elapsed() << "ms elapsed: Loading QML module";
  engine.loadFromModule("Crankshaft", "Main");
  qInfo() << "[STARTUP]" << startupTimer.elapsed() << "ms elapsed: QML module loaded";

  qInfo() << "[STARTUP] READY - Total UI startup time:" << startupTimer.elapsed() << "ms";

  return app.exec();
}
