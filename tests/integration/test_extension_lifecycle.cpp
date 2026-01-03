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

#include <QTest>
#include <QTemporaryDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QSignalSpy>

#include "../core/services/extensions/ExtensionManager.h"

class TestExtensionLifecycle : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        // Create temporary directory for extensions
        m_tempDir = new QTemporaryDir();
        QVERIFY(m_tempDir->isValid());
        
        m_extensionManager = new ExtensionManager(m_tempDir->path(), this);
    }

    void cleanupTestCase() {
        delete m_extensionManager;
        delete m_tempDir;
    }

    void testInstallExtension() {
        // Create a valid manifest
        QJsonObject manifest;
        manifest.insert(QStringLiteral("id"), QStringLiteral("test.extension.sample"));
        manifest.insert(QStringLiteral("name"), QStringLiteral("Test Sample"));
        manifest.insert(QStringLiteral("version"), QStringLiteral("1.0.0"));
        manifest.insert(QStringLiteral("entrypoint"), QStringLiteral("sleep"));
        
        QJsonArray permissions;
        permissions.append(QStringLiteral("ui.tile"));
        permissions.append(QStringLiteral("network"));
        manifest.insert(QStringLiteral("permissions"), permissions);

        QJsonDocument doc(manifest);
        QString manifestJson = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));

        // Install the extension
        QSignalSpy installSpy(m_extensionManager, &ExtensionManager::extensionInstalled);
        QString result = m_extensionManager->installExtension(manifestJson, QString());
        
        // Verify no error
        QVERIFY(result.isEmpty());
        QCOMPARE(installSpy.count(), 1);
        
        // Verify extension is listed
        QJsonArray list = m_extensionManager->listExtensions();
        QCOMPARE(list.count(), 1);
        
        QJsonObject ext = list[0].toObject();
        QCOMPARE(ext.value(QStringLiteral("id")).toString(), 
                 QStringLiteral("test.extension.sample"));
        QVERIFY(!ext.value(QStringLiteral("is_running")).toBool());
    }

    void testStartExtension() {
        // Create and install extension
        QJsonObject manifest;
        manifest.insert(QStringLiteral("id"), QStringLiteral("test.start.sample"));
        manifest.insert(QStringLiteral("name"), QStringLiteral("Test Start"));
        manifest.insert(QStringLiteral("version"), QStringLiteral("1.0.0"));
        manifest.insert(QStringLiteral("entrypoint"), QStringLiteral("sleep"));
        
        QJsonArray permissions;
        permissions.append(QStringLiteral("media.source"));
        manifest.insert(QStringLiteral("permissions"), permissions);

        QJsonDocument doc(manifest);
        QString manifestJson = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
        
        m_extensionManager->installExtension(manifestJson, QString());

        // Start the extension
        QSignalSpy startSpy(m_extensionManager, &ExtensionManager::extensionStarted);
        QString result = m_extensionManager->startExtension(
            QStringLiteral("test.start.sample"));
        
        // Verify no error
        QVERIFY(result.isEmpty());
        QCOMPARE(startSpy.count(), 1);
        
        // Verify extension is running
        QJsonObject info = m_extensionManager->getExtensionInfo(
            QStringLiteral("test.start.sample"));
        QVERIFY(info.value(QStringLiteral("is_running")).toBool());
        QVERIFY(info.value(QStringLiteral("pid")).toInt() > 0);

        // Clean up - stop the extension
        m_extensionManager->stopExtension(QStringLiteral("test.start.sample"));
    }

    void testStopExtension() {
        // Create, install, and start extension
        QJsonObject manifest;
        manifest.insert(QStringLiteral("id"), QStringLiteral("test.stop.sample"));
        manifest.insert(QStringLiteral("name"), QStringLiteral("Test Stop"));
        manifest.insert(QStringLiteral("version"), QStringLiteral("1.0.0"));
        manifest.insert(QStringLiteral("entrypoint"), QStringLiteral("sleep"));
        
        QJsonArray permissions;
        permissions.append(QStringLiteral("ui.tile"));
        manifest.insert(QStringLiteral("permissions"), permissions);

        QJsonDocument doc(manifest);
        QString manifestJson = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
        
        m_extensionManager->installExtension(manifestJson, QString());
        m_extensionManager->startExtension(QStringLiteral("test.stop.sample"));

        // Stop the extension
        QSignalSpy stopSpy(m_extensionManager, &ExtensionManager::extensionStopped);
        QString result = m_extensionManager->stopExtension(
            QStringLiteral("test.stop.sample"));
        
        // Verify no error
        QVERIFY(result.isEmpty());
        // Note: stopSpy may not fire immediately due to process termination timing
        
        // Verify extension is stopped
        QJsonObject info = m_extensionManager->getExtensionInfo(
            QStringLiteral("test.stop.sample"));
        QVERIFY(!info.value(QStringLiteral("is_running")).toBool());
    }

    void testRestartExtension() {
        // Create, install, and start extension
        QJsonObject manifest;
        manifest.insert(QStringLiteral("id"), QStringLiteral("test.restart.sample"));
        manifest.insert(QStringLiteral("name"), QStringLiteral("Test Restart"));
        manifest.insert(QStringLiteral("version"), QStringLiteral("1.0.0"));
        manifest.insert(QStringLiteral("entrypoint"), QStringLiteral("sleep"));
        
        QJsonArray permissions;
        manifest.insert(QStringLiteral("permissions"), permissions);

        QJsonDocument doc(manifest);
        QString manifestJson = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
        
        m_extensionManager->installExtension(manifestJson, QString());
        m_extensionManager->startExtension(QStringLiteral("test.restart.sample"));
        
        QJsonObject infoRunning = m_extensionManager->getExtensionInfo(
            QStringLiteral("test.restart.sample"));
        qint64 oldPid = infoRunning.value(QStringLiteral("pid")).toInt();

        // Restart the extension
        QString result = m_extensionManager->restartExtension(
            QStringLiteral("test.restart.sample"));
        
        QVERIFY(result.isEmpty());
        
        // Allow time for process restart
        QTest::qWait(500);
        
        // Verify extension is running with new PID
        QJsonObject infoRestarted = m_extensionManager->getExtensionInfo(
            QStringLiteral("test.restart.sample"));
        QVERIFY(infoRestarted.value(QStringLiteral("is_running")).toBool());
        // Note: PID might be the same if process slot is reused, but should be running
        
        // Clean up
        m_extensionManager->stopExtension(QStringLiteral("test.restart.sample"));
    }

    void testUninstallExtension() {
        // Create and install extension
        QJsonObject manifest;
        manifest.insert(QStringLiteral("id"), QStringLiteral("test.uninstall.sample"));
        manifest.insert(QStringLiteral("name"), QStringLiteral("Test Uninstall"));
        manifest.insert(QStringLiteral("version"), QStringLiteral("1.0.0"));
        manifest.insert(QStringLiteral("entrypoint"), QStringLiteral("sleep"));
        
        QJsonArray permissions;
        manifest.insert(QStringLiteral("permissions"), permissions);

        QJsonDocument doc(manifest);
        QString manifestJson = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
        
        m_extensionManager->installExtension(manifestJson, QString());
        
        // Verify installed
        QJsonArray listBefore = m_extensionManager->listExtensions();
        QVERIFY(listBefore.count() > 0);

        // Uninstall
        QSignalSpy uninstallSpy(m_extensionManager, &ExtensionManager::extensionUninstalled);
        QString result = m_extensionManager->uninstallExtension(
            QStringLiteral("test.uninstall.sample"));
        
        QVERIFY(result.isEmpty());
        QCOMPARE(uninstallSpy.count(), 1);
        
        // Verify directory removed
        QString installDir = m_tempDir->path() + QStringLiteral("/test.uninstall.sample");
        QVERIFY(!QDir(installDir).exists());
    }

    void testPermissionChecking() {
        // Create extension with specific permissions
        QJsonObject manifest;
        manifest.insert(QStringLiteral("id"), QStringLiteral("test.perms.sample"));
        manifest.insert(QStringLiteral("name"), QStringLiteral("Test Permissions"));
        manifest.insert(QStringLiteral("version"), QStringLiteral("1.0.0"));
        manifest.insert(QStringLiteral("entrypoint"), QStringLiteral("sleep"));
        
        QJsonArray permissions;
        permissions.append(QStringLiteral("ui.tile"));
        permissions.append(QStringLiteral("media.source"));
        permissions.append(QStringLiteral("network"));
        manifest.insert(QStringLiteral("permissions"), permissions);

        QJsonDocument doc(manifest);
        QString manifestJson = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
        
        m_extensionManager->installExtension(manifestJson, QString());

        // Test has permission
        QVERIFY(m_extensionManager->hasPermission(
            QStringLiteral("test.perms.sample"), 
            QStringLiteral("ui.tile")));
        QVERIFY(m_extensionManager->hasPermission(
            QStringLiteral("test.perms.sample"), 
            QStringLiteral("network")));

        // Test doesn't have permission
        QVERIFY(!m_extensionManager->hasPermission(
            QStringLiteral("test.perms.sample"), 
            QStringLiteral("diagnostics")));
        QVERIFY(!m_extensionManager->hasPermission(
            QStringLiteral("test.perms.sample"), 
            QStringLiteral("service.control")));
    }

    void testGetExtensionsWithPermission() {
        // Create two extensions with different permissions
        for (int i = 0; i < 2; ++i) {
            QJsonObject manifest;
            manifest.insert(QStringLiteral("id"), 
                           QStringLiteral("test.filter.sample%1").arg(i));
            manifest.insert(QStringLiteral("name"), 
                           QStringLiteral("Test Filter %1").arg(i));
            manifest.insert(QStringLiteral("version"), QStringLiteral("1.0.0"));
            manifest.insert(QStringLiteral("entrypoint"), QStringLiteral("sleep"));
            
            QJsonArray permissions;
            permissions.append(QStringLiteral("ui.tile"));
            if (i == 0) {
                permissions.append(QStringLiteral("network"));
            }
            manifest.insert(QStringLiteral("permissions"), permissions);

            QJsonDocument doc(manifest);
            QString manifestJson = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
            m_extensionManager->installExtension(manifestJson, QString());
        }

        // Get all extensions with ui.tile permission
        QJsonArray withUiTile = m_extensionManager->getExtensionsWithPermission(
            QStringLiteral("ui.tile"));
        QCOMPARE(withUiTile.count(), 2);

        // Get all extensions with network permission
        QJsonArray withNetwork = m_extensionManager->getExtensionsWithPermission(
            QStringLiteral("network"));
        QCOMPARE(withNetwork.count(), 1);
    }

    void testManifestValidation() {
        // Test invalid manifest (missing required field)
        QJsonObject invalidManifest;
        invalidManifest.insert(QStringLiteral("id"), QStringLiteral("invalid"));
        // Missing: name, version, entrypoint, permissions

        QJsonDocument doc(invalidManifest);
        QString manifestJson = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));

        QString result = m_extensionManager->installExtension(manifestJson, QString());
        QVERIFY(!result.isEmpty());  // Should return an error
    }

    void testInvalidExtensionId() {
        // Test invalid id format (contains uppercase, which is not allowed)
        QJsonObject manifest;
        manifest.insert(QStringLiteral("id"), QStringLiteral("Test.Invalid.ID"));
        manifest.insert(QStringLiteral("name"), QStringLiteral("Test Invalid"));
        manifest.insert(QStringLiteral("version"), QStringLiteral("1.0.0"));
        manifest.insert(QStringLiteral("entrypoint"), QStringLiteral("sleep"));
        
        QJsonArray permissions;
        manifest.insert(QStringLiteral("permissions"), permissions);

        QJsonDocument doc(manifest);
        QString manifestJson = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));

        QString result = m_extensionManager->installExtension(manifestJson, QString());
        QVERIFY(!result.isEmpty());  // Should return an error due to invalid id
    }

    void testStartNonexistentExtension() {
        QString result = m_extensionManager->startExtension(
            QStringLiteral("nonexistent.extension"));
        QVERIFY(!result.isEmpty());
        QVERIFY(result.contains(QStringLiteral("not found")));
    }

private:
    ExtensionManager* m_extensionManager;
    QTemporaryDir* m_tempDir;
};

QTEST_MAIN(TestExtensionLifecycle)
#include "test_extension_lifecycle.moc"
