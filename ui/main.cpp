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
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTranslator>

#include "Theme.h"
#include "WebSocketClient.h"

int main(int argc, char* argv[]) {
  QGuiApplication app(argc, argv);
  QGuiApplication::setApplicationName("Crankshaft UI");
  QGuiApplication::setApplicationVersion("0.1.0");

  // Parse command line arguments
  QCommandLineParser parser;
  parser.setApplicationDescription("Crankshaft Automotive Infotainment UI");
  parser.addHelpOption();
  parser.addVersionOption();

  QCommandLineOption serverOption(QStringList() << "s" << "server", "WebSocket server URL", "server",
                                  "ws://localhost:8080");
  parser.addOption(serverOption);

  QCommandLineOption languageOption(QStringList() << "l" << "language", "UI language (en-GB, de-DE)", "language",
                                    "en-GB");
  parser.addOption(languageOption);

  parser.process(app);

  // Set up translator (heap-allocated for proper lifetime)
  QTranslator* translator = new QTranslator(&app);
  QString currentLanguage = parser.value(languageOption);
  QString translationFile = QString(":/i18n/ui_%1").arg(QString(currentLanguage).replace('-', '_'));
  if (translator->load(translationFile)) {
    app.installTranslator(translator);
    qDebug() << "Loaded translation:" << translationFile;
  } else {
    qWarning() << "Failed to load translation:" << translationFile;
  }

  // Create WebSocket client
  QString serverUrl = parser.value(serverOption);
  WebSocketClient* wsClient = new WebSocketClient(QUrl(serverUrl));

  // Subscribe to common topics
  wsClient->subscribe("ui/*");
  wsClient->subscribe("system/*");

  // Create QML engine
  QQmlApplicationEngine engine;
  
  // Create Theme instance
  Theme* theme = new Theme(&app);
  
  // Set context properties (Theme as global object, not singleton)
  engine.rootContext()->setContextProperty("Theme", theme);
  engine.rootContext()->setContextProperty("wsClient", wsClient);
  engine.rootContext()->setContextProperty("currentLanguage", currentLanguage);

  // Handle language change events
  QObject::connect(wsClient, &WebSocketClient::eventReceived, [&, translator](const QString& topic, const QVariantMap& payload) {
    if (topic == "ui/language/changed") {
      QString newLanguage = payload.value("language").toString();
      qDebug() << "Language change requested:" << newLanguage;
      
      // Remove old translator
      app.removeTranslator(translator);
      
      // Load new translator (convert hyphens to underscores for file path)
      QString translationFile = QString(":/i18n/ui_%1").arg(QString(newLanguage).replace('-', '_'));
      if (translator->load(translationFile)) {
        app.installTranslator(translator);
        qDebug() << "Translation loaded:" << translationFile;
        
        // Update context property
        currentLanguage = newLanguage;
        engine.rootContext()->setContextProperty("currentLanguage", currentLanguage);
        
        // Trigger retranslation
        engine.retranslate();
        qDebug() << "UI retranslated";
      } else {
        qWarning() << "Failed to load translation:" << translationFile;
      }
    }
  });

  // Load the module
  engine.loadFromModule("Crankshaft", "Main");

  return app.exec();
}
