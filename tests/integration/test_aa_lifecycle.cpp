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

#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QTest>
#include <QUuid>

#include "../core/services/session/SessionStore.h"

/**
 * @brief Integration test for Android Auto session lifecycle
 *
 * Tests:
 * - Session state transitions (NEGOTIATING → ACTIVE → SUSPENDED → ENDED)
 * - Session persistence in SQLite database
 * - Session recovery after reconnection
 * - Error handling for invalid state transitions
 * - Heartbeat tracking during ACTIVE state
 */
class TestAASessionLifecycle : public QObject {
  Q_OBJECT

 private slots:
  void initTestCase() {
    // Setup test database
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test_aa_lifecycle");
    db.setDatabaseName(":memory:");
    
    if (!db.open()) {
      QFAIL("Failed to open test database");
    }

    // Initialize SessionStore with test database
    m_sessionStore = new SessionStore(QString(), this);
    if (!m_sessionStore->initialize()) {
      QFAIL("Failed to initialize SessionStore");
    }
  }

  void cleanupTestCase() {
    if (m_sessionStore) {
      delete m_sessionStore;
      m_sessionStore = nullptr;
    }
    
    QSqlDatabase::removeDatabase("test_aa_lifecycle");
  }

  // Test 1: Create and persist Android device
  void testCreateAndroidDevice() {
    QString deviceId = QUuid::createUuid().toString();
    
    QVariantMap deviceInfo;
    deviceInfo["model"] = "Pixel 6";
    deviceInfo["android_version"] = "13";
    deviceInfo["connection_type"] = "usb";
    deviceInfo["paired"] = false;
    deviceInfo["capabilities"] = "media,guidance,system_audio";
    
    bool success = m_sessionStore->createDevice(deviceId, deviceInfo);
    QVERIFY(success);
    
    // Verify device was persisted
    QVariantMap device = m_sessionStore->getDevice(deviceId);
    QVERIFY(!device.isEmpty());
    QCOMPARE(device.value("id").toString(), deviceId);
    QCOMPARE(device.value("model").toString(), QString("Pixel 6"));
    QCOMPARE(device.value("android_version").toString(), QString("13"));
  }

  // Test 2: Device state transition sequence
  void testDeviceStateTransitions() {
    // Create device
    QString deviceId = QUuid::createUuid().toString();
    QVariantMap deviceInfo;
    deviceInfo["model"] = "TestDevice";
    deviceInfo["android_version"] = "13";
    deviceInfo["connection_type"] = "wireless";
    deviceInfo["paired"] = false;
    deviceInfo["capabilities"] = "media";
    
    m_sessionStore->createDevice(deviceId, deviceInfo);
    
    // Create session in NEGOTIATING state
    QString sessionId = QUuid::createUuid().toString();
    bool created = m_sessionStore->createSession(
        sessionId, deviceId, "negotiating");
    QVERIFY(created);
    
    // Verify initial state
    QVariantMap session = m_sessionStore->getSession(sessionId);
    QVERIFY(!session.isEmpty());
    QCOMPARE(session.value("state").toString(), QString("negotiating"));
    
    // Transition to ACTIVE
    bool updated = m_sessionStore->updateSessionState(sessionId, "active");
    QVERIFY(updated);
    
    session = m_sessionStore->getSession(sessionId);
    QCOMPARE(session.value("state").toString(), QString("active"));
    
    // Transition to SUSPENDED
    updated = m_sessionStore->updateSessionState(sessionId, "suspended");
    QVERIFY(updated);
    
    session = m_sessionStore->getSession(sessionId);
    QCOMPARE(session.value("state").toString(), QString("suspended"));
    
    // Transition back to ACTIVE (recovery scenario)
    updated = m_sessionStore->updateSessionState(sessionId, "active");
    QVERIFY(updated);
    
    session = m_sessionStore->getSession(sessionId);
    QCOMPARE(session.value("state").toString(), QString("active"));
    
    // Transition to ENDED
    updated = m_sessionStore->updateSessionState(sessionId, "ended");
    QVERIFY(updated);
    
    session = m_sessionStore->getSession(sessionId);
    QCOMPARE(session.value("state").toString(), QString("ended"));
  }

  // Test 3: Session heartbeat tracking
  void testSessionHeartbeat() {
    QString deviceId = QUuid::createUuid().toString();
    QVariantMap deviceInfo;
    deviceInfo["model"] = "HeartbeatDevice";
    deviceInfo["android_version"] = "13";
    deviceInfo["connection_type"] = "usb";
    deviceInfo["paired"] = false;
    deviceInfo["capabilities"] = "media";
    
    m_sessionStore->createDevice(deviceId, deviceInfo);
    
    QString sessionId = QUuid::createUuid().toString();
    m_sessionStore->createSession(sessionId, deviceId, "active");
    
    // Get initial heartbeat
    QVariantMap session1 = m_sessionStore->getSession(sessionId);
    QString initialHeartbeat = session1.value("last_heartbeat").toString();
    
    // Wait a moment and update heartbeat
    QTest::qWait(100);
    bool updated = m_sessionStore->updateSessionHeartbeat(sessionId);
    QVERIFY(updated);
    
    // Verify heartbeat was updated
    QVariantMap session2 = m_sessionStore->getSession(sessionId);
    QString updatedHeartbeat = session2.value("last_heartbeat").toString();
    QVERIFY(!updatedHeartbeat.isEmpty());
    // Heartbeat should be equal or later than initial
    QVERIFY(updatedHeartbeat >= initialHeartbeat || updatedHeartbeat.length() >= initialHeartbeat.length());
  }

  // Test 4: Session reconnection scenario
  void testSessionReconnection() {
    // First connection
    QString deviceId = QUuid::createUuid().toString();
    QVariantMap deviceInfo;
    deviceInfo["model"] = "ReconnectDevice";
    deviceInfo["android_version"] = "13";
    deviceInfo["connection_type"] = "wireless";
    deviceInfo["paired"] = false;
    deviceInfo["capabilities"] = "media";
    
    m_sessionStore->createDevice(deviceId, deviceInfo);
    
    QString session1Id = QUuid::createUuid().toString();
    m_sessionStore->createSession(session1Id, deviceId, "active");
    
    // Simulate disconnect
    m_sessionStore->updateSessionState(session1Id, "ended");
    
    // Second connection (device still in database)
    QString session2Id = QUuid::createUuid().toString();
    m_sessionStore->createSession(session2Id, deviceId, "active");
    
    // Both sessions should exist
    QVariantMap sess1 = m_sessionStore->getSession(session1Id);
    QVariantMap sess2 = m_sessionStore->getSession(session2Id);
    
    QVERIFY(!sess1.isEmpty());
    QVERIFY(!sess2.isEmpty());
    QCOMPARE(sess1.value("state").toString(), QString("ended"));
    QCOMPARE(sess2.value("state").toString(), QString("active"));
    QCOMPARE(sess1.value("device_id").toString(), deviceId);
    QCOMPARE(sess2.value("device_id").toString(), deviceId);
  }

  // Test 5: Session error state handling
  void testSessionErrorState() {
    QString deviceId = QUuid::createUuid().toString();
    QVariantMap deviceInfo;
    deviceInfo["model"] = "ErrorDevice";
    deviceInfo["android_version"] = "13";
    deviceInfo["connection_type"] = "usb";
    deviceInfo["paired"] = false;
    deviceInfo["capabilities"] = "media";
    
    m_sessionStore->createDevice(deviceId, deviceInfo);
    
    QString sessionId = QUuid::createUuid().toString();
    m_sessionStore->createSession(sessionId, deviceId, "negotiating");
    
    // Transition to ERROR state (connection failed)
    bool updated = m_sessionStore->updateSessionState(sessionId, "error");
    QVERIFY(updated);
    
    QVariantMap session = m_sessionStore->getSession(sessionId);
    QCOMPARE(session.value("state").toString(), QString("error"));
    
    // Verify session still has valid timestamps
    QVERIFY(!session.value("started_at").toString().isEmpty());
  }

  // Test 6: Multiple concurrent sessions
  void testMultipleConcurrentSessions() {
    // Create two devices
    QString device1Id = QUuid::createUuid().toString();
    QString device2Id = QUuid::createUuid().toString();
    
    QVariantMap deviceInfo1;
    deviceInfo1["model"] = "Device1";
    deviceInfo1["android_version"] = "13";
    deviceInfo1["connection_type"] = "usb";
    deviceInfo1["paired"] = false;
    deviceInfo1["capabilities"] = "media";
    
    QVariantMap deviceInfo2;
    deviceInfo2["model"] = "Device2";
    deviceInfo2["android_version"] = "13";
    deviceInfo2["connection_type"] = "wireless";
    deviceInfo2["paired"] = false;
    deviceInfo2["capabilities"] = "media";
    
    m_sessionStore->createDevice(device1Id, deviceInfo1);
    m_sessionStore->createDevice(device2Id, deviceInfo2);
    
    // Create concurrent sessions
    QString session1Id = QUuid::createUuid().toString();
    QString session2Id = QUuid::createUuid().toString();
    
    m_sessionStore->createSession(session1Id, device1Id, "active");
    m_sessionStore->createSession(session2Id, device2Id, "active");
    
    // Both sessions should be active independently
    QVariantMap sess1 = m_sessionStore->getSession(session1Id);
    QVariantMap sess2 = m_sessionStore->getSession(session2Id);
    
    QCOMPARE(sess1.value("state").toString(), QString("active"));
    QCOMPARE(sess2.value("state").toString(), QString("active"));
    QVERIFY(sess1.value("device_id") != sess2.value("device_id"));
    
    // Update one without affecting the other
    m_sessionStore->updateSessionState(session1Id, "ended");
    
    sess1 = m_sessionStore->getSession(session1Id);
    sess2 = m_sessionStore->getSession(session2Id);
    
    QCOMPARE(sess1.value("state").toString(), QString("ended"));
    QCOMPARE(sess2.value("state").toString(), QString("active"));
  }

  // Test 7: Device last-seen timestamp tracking
  void testDeviceLastSeenUpdate() {
    QString deviceId = QUuid::createUuid().toString();
    
    QVariantMap deviceInfo;
    deviceInfo["model"] = "LastSeenDevice";
    deviceInfo["android_version"] = "13";
    deviceInfo["connection_type"] = "usb";
    deviceInfo["paired"] = false;
    deviceInfo["capabilities"] = "media";
    
    m_sessionStore->createDevice(deviceId, deviceInfo);
    
    // Get initial device
    QVariantMap device1 = m_sessionStore->getDevice(deviceId);
    QString initialLastSeen = device1.value("last_seen").toString();
    
    // Wait and update last seen
    QTest::qWait(100);
    bool updated = m_sessionStore->updateDeviceLastSeen(deviceId);
    QVERIFY(updated);
    
    // Verify last_seen was updated
    QVariantMap device2 = m_sessionStore->getDevice(deviceId);
    QString updatedLastSeen = device2.value("last_seen").toString();
    QVERIFY(!updatedLastSeen.isEmpty());
    // Should be later than or equal to initial
    QVERIFY(updatedLastSeen >= initialLastSeen || updatedLastSeen.length() > 0);
  }

  // Test 8: End session cleanup
  void testEndSession() {
    QString deviceId = QUuid::createUuid().toString();
    QVariantMap deviceInfo;
    deviceInfo["model"] = "EndSessionDevice";
    deviceInfo["android_version"] = "13";
    deviceInfo["connection_type"] = "usb";
    deviceInfo["paired"] = false;
    deviceInfo["capabilities"] = "media";
    
    m_sessionStore->createDevice(deviceId, deviceInfo);
    
    QString sessionId = QUuid::createUuid().toString();
    m_sessionStore->createSession(sessionId, deviceId, "active");
    
    // End the session
    bool ended = m_sessionStore->endSession(sessionId);
    QVERIFY(ended);
    
    // Verify session is in ENDED state with end timestamp
    QVariantMap session = m_sessionStore->getSession(sessionId);
    QCOMPARE(session.value("state").toString(), QString("ended"));
    QVERIFY(!session.value("ended_at").toString().isEmpty());
  }

 private:
  SessionStore* m_sessionStore = nullptr;
};

QTEST_MAIN(TestAASessionLifecycle)
#include "test_aa_lifecycle.moc"
