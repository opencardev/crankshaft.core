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

#include <QSqlDatabase>
#include <QStandardPaths>
#include <QString>
#include <QTest>
#include <QTemporaryDir>

#include "../core/services/preferences/PreferencesService.h"

class TestSettingsPersistence : public QObject {
  Q_OBJECT

 private:
  QTemporaryDir m_tempDir;
  PreferencesService* m_preferencesService{nullptr};

 private slots:
  void initTestCase() {
    // Verify temp directory created
    QVERIFY(m_tempDir.isValid());
  }

  void init() {
    // Create fresh PreferencesService instance for each test
    const QString dbPath = m_tempDir.path() + QStringLiteral("/test_prefs.db");
    m_preferencesService = new PreferencesService(dbPath, this);
    QVERIFY(m_preferencesService->initialize());
  }

  void cleanup() {
    if (m_preferencesService) {
      m_preferencesService->deleteLater();
      m_preferencesService = nullptr;
    }
  }

  // Test 1: Theme preference saved and retrieved
  void testThemePreferencePersistence() {
    QVERIFY(!m_preferencesService->contains(QStringLiteral("ui.theme.dark")));

    // Set dark theme
    QVERIFY(m_preferencesService->set(QStringLiteral("ui.theme.dark"), true));

    // Verify in-memory cache
    QVERIFY(m_preferencesService->get(QStringLiteral("ui.theme.dark")).toBool() == true);

    // Create new instance to verify persistence
    const QString dbPath = m_tempDir.path() + QStringLiteral("/test_prefs.db");
    PreferencesService newService(dbPath, nullptr);
    QVERIFY(newService.initialize());

    // Verify value persisted to database
    QVERIFY(newService.get(QStringLiteral("ui.theme.dark")).toBool() == true);
  }

  // Test 2: Theme toggle light mode
  void testThemeToggleLightMode() {
    // Set to dark initially
    QVERIFY(m_preferencesService->set(QStringLiteral("ui.theme.dark"), true));
    QVERIFY(m_preferencesService->get(QStringLiteral("ui.theme.dark")).toBool() == true);

    // Toggle to light
    QVERIFY(m_preferencesService->set(QStringLiteral("ui.theme.dark"), false));
    QVERIFY(m_preferencesService->get(QStringLiteral("ui.theme.dark")).toBool() == false);

    // Verify persistence of light mode
    const QString dbPath = m_tempDir.path() + QStringLiteral("/test_prefs.db");
    PreferencesService newService(dbPath, nullptr);
    QVERIFY(newService.initialize());
    QVERIFY(newService.get(QStringLiteral("ui.theme.dark")).toBool() == false);
  }

  // Test 3: Locale preference persistence
  void testLocalePreferencePersistence() {
    QVERIFY(!m_preferencesService->contains(QStringLiteral("ui.locale")));

    // Set locale to de-DE
    QVERIFY(m_preferencesService->set(QStringLiteral("ui.locale"), QStringLiteral("de-DE")));

    // Verify in cache
    QCOMPARE(m_preferencesService->get(QStringLiteral("ui.locale")).toString(),
             QStringLiteral("de-DE"));

    // Verify persistence
    const QString dbPath = m_tempDir.path() + QStringLiteral("/test_prefs.db");
    PreferencesService newService(dbPath, nullptr);
    QVERIFY(newService.initialize());
    QCOMPARE(newService.get(QStringLiteral("ui.locale")).toString(), QStringLiteral("de-DE"));
  }

  // Test 4: Locale fallback to default
  void testLocaleDefaultFallback() {
    QVERIFY(!m_preferencesService->contains(QStringLiteral("ui.locale")));

    // Get non-existent locale with default fallback
    const QString defaultLocale = QStringLiteral("en-GB");
    const QString locale =
        m_preferencesService->get(QStringLiteral("ui.locale"), defaultLocale).toString();

    QCOMPARE(locale, defaultLocale);
  }

  // Test 5: Multiple preferences in same session
  void testMultiplePreferencesInSession() {
    // Set multiple preferences
    QVERIFY(m_preferencesService->set(QStringLiteral("ui.theme.dark"), true));
    QVERIFY(m_preferencesService->set(QStringLiteral("ui.locale"), QStringLiteral("de-DE")));
    QVERIFY(m_preferencesService->set(QStringLiteral("audio.volume"), 80));
    QVERIFY(
        m_preferencesService->set(QStringLiteral("media.lastDirectory"), QStringLiteral("/home/music")));

    // Verify all values
    QVERIFY(m_preferencesService->get(QStringLiteral("ui.theme.dark")).toBool() == true);
    QCOMPARE(m_preferencesService->get(QStringLiteral("ui.locale")).toString(),
             QStringLiteral("de-DE"));
    QCOMPARE(m_preferencesService->get(QStringLiteral("audio.volume")).toInt(), 80);
    QCOMPARE(m_preferencesService->get(QStringLiteral("media.lastDirectory")).toString(),
             QStringLiteral("/home/music"));
  }

  // Test 6: Multiple preferences persist across sessions
  void testMultiplePreferencesPersistence() {
    // Set multiple preferences
    QVERIFY(m_preferencesService->set(QStringLiteral("ui.theme.dark"), true));
    QVERIFY(m_preferencesService->set(QStringLiteral("ui.locale"), QStringLiteral("fr-FR")));
    QVERIFY(m_preferencesService->set(QStringLiteral("audio.volume"), 65));

    // Create new service instance
    const QString dbPath = m_tempDir.path() + QStringLiteral("/test_prefs.db");
    PreferencesService newService(dbPath, nullptr);
    QVERIFY(newService.initialize());

    // Verify all values persisted
    QVERIFY(newService.get(QStringLiteral("ui.theme.dark")).toBool() == true);
    QCOMPARE(newService.get(QStringLiteral("ui.locale")).toString(), QStringLiteral("fr-FR"));
    QCOMPARE(newService.get(QStringLiteral("audio.volume")).toInt(), 65);
  }

  // Test 7: Preference removal
  void testPreferenceRemoval() {
    QVERIFY(m_preferencesService->set(QStringLiteral("ui.theme.dark"), true));
    QVERIFY(m_preferencesService->contains(QStringLiteral("ui.theme.dark")));

    // Remove preference
    QVERIFY(m_preferencesService->remove(QStringLiteral("ui.theme.dark")));

    // Verify removal
    QVERIFY(!m_preferencesService->contains(QStringLiteral("ui.theme.dark")));

    // Verify persistence of removal
    const QString dbPath = m_tempDir.path() + QStringLiteral("/test_prefs.db");
    PreferencesService newService(dbPath, nullptr);
    QVERIFY(newService.initialize());
    QVERIFY(!newService.contains(QStringLiteral("ui.theme.dark")));
  }

  // Test 8: Clear all preferences
  void testClearAllPreferences() {
    // Set multiple preferences
    QVERIFY(m_preferencesService->set(QStringLiteral("ui.theme.dark"), true));
    QVERIFY(m_preferencesService->set(QStringLiteral("ui.locale"), QStringLiteral("es-ES")));
    QVERIFY(m_preferencesService->set(QStringLiteral("audio.volume"), 75));

    // Verify preferences exist
    QVERIFY(m_preferencesService->contains(QStringLiteral("ui.theme.dark")));
    QVERIFY(m_preferencesService->contains(QStringLiteral("ui.locale")));
    QVERIFY(m_preferencesService->contains(QStringLiteral("audio.volume")));

    // Clear all
    QVERIFY(m_preferencesService->clear());

    // Verify cleared
    QVERIFY(!m_preferencesService->contains(QStringLiteral("ui.theme.dark")));
    QVERIFY(!m_preferencesService->contains(QStringLiteral("ui.locale")));
    QVERIFY(!m_preferencesService->contains(QStringLiteral("audio.volume")));

    // Verify persistence of clear
    const QString dbPath = m_tempDir.path() + QStringLiteral("/test_prefs.db");
    PreferencesService newService(dbPath, nullptr);
    QVERIFY(newService.initialize());
    QVERIFY(!newService.contains(QStringLiteral("ui.theme.dark")));
  }

  // Test 9: Get all keys
  void testGetAllKeys() {
    QVERIFY(m_preferencesService->clear());

    // Set various preferences
    QVERIFY(m_preferencesService->set(QStringLiteral("ui.theme.dark"), true));
    QVERIFY(m_preferencesService->set(QStringLiteral("ui.locale"), QStringLiteral("it-IT")));
    QVERIFY(m_preferencesService->set(QStringLiteral("audio.volume"), 70));

    // Get all keys
    QStringList keys = m_preferencesService->allKeys();

    QCOMPARE(keys.size(), 3);
    QVERIFY(keys.contains(QStringLiteral("ui.theme.dark")));
    QVERIFY(keys.contains(QStringLiteral("ui.locale")));
    QVERIFY(keys.contains(QStringLiteral("audio.volume")));
  }

  // Test 10: Theme preference survives multiple restarts
  void testThemeMultipleRestarts() {
    // Initial set: dark mode
    QVERIFY(m_preferencesService->set(QStringLiteral("ui.theme.dark"), true));

    const QString dbPath = m_tempDir.path() + QStringLiteral("/test_prefs.db");

    // First "restart" - verify dark mode
    {
      PreferencesService service1(dbPath, nullptr);
      QVERIFY(service1.initialize());
      QVERIFY(service1.get(QStringLiteral("ui.theme.dark")).toBool() == true);

      // Toggle to light
      QVERIFY(service1.set(QStringLiteral("ui.theme.dark"), false));
    }

    // Second "restart" - verify light mode
    {
      PreferencesService service2(dbPath, nullptr);
      QVERIFY(service2.initialize());
      QVERIFY(service2.get(QStringLiteral("ui.theme.dark")).toBool() == false);

      // Toggle back to dark
      QVERIFY(service2.set(QStringLiteral("ui.theme.dark"), true));
    }

    // Third "restart" - verify back to dark
    {
      PreferencesService service3(dbPath, nullptr);
      QVERIFY(service3.initialize());
      QVERIFY(service3.get(QStringLiteral("ui.theme.dark")).toBool() == true);
    }
  }
};

QTEST_MAIN(TestSettingsPersistence)
#include "test_settings_persistence.moc"
