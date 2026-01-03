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

#include "SessionStore.h"

#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>

#include "../logging/Logger.h"

SessionStore::SessionStore(const QString& dbPath, QObject* parent)
    : QObject(parent),
      m_db(std::make_unique<QSqlDatabase>(QSqlDatabase::addDatabase("QSQLITE", "session_store"))),
      m_dbPath(dbPath.isEmpty()
                   ? QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
                         "/session.db"
                   : dbPath) {
  Logger::instance().info(QString("[SessionStore] Initialized with database: %1").arg(m_dbPath));
}

SessionStore::~SessionStore() = default;

bool SessionStore::initialize() {
  m_db->setDatabaseName(m_dbPath);

  if (!m_db->open()) {
    Logger::instance().error(
        QString("[SessionStore] Failed to open database: %1").arg(m_db->lastError().text()));
    return false;
  }

  if (!createSchema()) {
    Logger::instance().error("[SessionStore] Failed to create schema");
    m_db->close();
    return false;
  }

  Logger::instance().info("[SessionStore] Initialized successfully");
  return true;
}

bool SessionStore::createSchema() {
  QSqlQuery query(*m_db);

  // Create android_devices table
  if (!query.exec(QStringLiteral(
          "CREATE TABLE IF NOT EXISTS android_devices ("
          "  id TEXT PRIMARY KEY,"
          "  model TEXT NOT NULL,"
          "  android_version TEXT,"
          "  connection_type TEXT CHECK(connection_type IN ('wired', 'wireless')),"
          "  paired INTEGER NOT NULL DEFAULT 0,"
          "  last_seen INTEGER NOT NULL,"
          "  capabilities TEXT"
          ")"))) {
    Logger::instance().error(
        QString("[SessionStore] Failed to create android_devices table: %1")
            .arg(query.lastError().text()));
    return false;
  }

  // Create sessions table
  if (!query.exec(QStringLiteral(
          "CREATE TABLE IF NOT EXISTS sessions ("
          "  id TEXT PRIMARY KEY,"
          "  device_id TEXT NOT NULL,"
          "  state TEXT CHECK(state IN ('negotiating', 'active', 'suspended', 'ended', 'error')),"
          "  started_at INTEGER NOT NULL,"
          "  ended_at INTEGER,"
          "  last_heartbeat INTEGER NOT NULL,"
          "  FOREIGN KEY (device_id) REFERENCES android_devices(id)"
          ")"))) {
    Logger::instance().error(
        QString("[SessionStore] Failed to create sessions table: %1")
            .arg(query.lastError().text()));
    return false;
  }

  return true;
}

bool SessionStore::createDevice(const QString& deviceId, const QVariantMap& deviceInfo) {
  QSqlQuery query(*m_db);
  query.prepare(
      QStringLiteral("INSERT INTO android_devices "
                     "(id, model, android_version, connection_type, paired, last_seen, capabilities) "
                     "VALUES (?, ?, ?, ?, ?, ?, ?)"));

  query.addBindValue(deviceId);
  query.addBindValue(deviceInfo.value("model", "").toString());
  query.addBindValue(deviceInfo.value("androidVersion", "").toString());
  query.addBindValue(deviceInfo.value("connectionType", "wired").toString());
  query.addBindValue(deviceInfo.value("paired", false).toBool() ? 1 : 0);
  query.addBindValue(QDateTime::currentSecsSinceEpoch());

  const QJsonDocument capabilitiesDoc = QJsonDocument::fromVariant(deviceInfo.value("capabilities"));
  query.addBindValue(QString::fromUtf8(capabilitiesDoc.toJson(QJsonDocument::Compact)));

  if (!query.exec()) {
    Logger::instance().error(QString("[SessionStore] Failed to create device %1: %2")
                                 .arg(deviceId, query.lastError().text()));
    return false;
  }

  Logger::instance().info(QString("[SessionStore] Created device: %1").arg(deviceId));
  return true;
}

QVariantMap SessionStore::getDevice(const QString& deviceId) const {
  QSqlQuery query(*m_db);
  query.prepare(
      QStringLiteral("SELECT model, android_version, connection_type, paired, last_seen, "
                     "capabilities FROM android_devices WHERE id = ?"));
  query.addBindValue(deviceId);

  if (!query.exec() || !query.next()) {
    Logger::instance().warning(QString("[SessionStore] Device not found: %1").arg(deviceId));
    return QVariantMap();
  }

  QVariantMap device;
  device["id"] = deviceId;
  device["model"] = query.value(0).toString();
  device["androidVersion"] = query.value(1).toString();
  device["connectionType"] = query.value(2).toString();
  device["paired"] = query.value(3).toInt() == 1;
  device["lastSeen"] = query.value(4).toLongLong();

  const QJsonDocument capabilitiesDoc = QJsonDocument::fromJson(query.value(5).toByteArray());
  device["capabilities"] = capabilitiesDoc.toVariant();

  return device;
}

QList<QVariantMap> SessionStore::getAllDevices() const {
  QList<QVariantMap> devices;
  QSqlQuery query(*m_db);

  if (!query.exec(QStringLiteral(
          "SELECT id, model, android_version, connection_type, paired, last_seen, "
          "capabilities FROM android_devices"))) {
    Logger::instance().error(
        QString("[SessionStore] Failed to fetch all devices: %1").arg(query.lastError().text()));
    return devices;
  }

  while (query.next()) {
    QVariantMap device;
    device["id"] = query.value(0).toString();
    device["model"] = query.value(1).toString();
    device["androidVersion"] = query.value(2).toString();
    device["connectionType"] = query.value(3).toString();
    device["paired"] = query.value(4).toInt() == 1;
    device["lastSeen"] = query.value(5).toLongLong();

    const QJsonDocument capabilitiesDoc = QJsonDocument::fromJson(query.value(6).toByteArray());
    device["capabilities"] = capabilitiesDoc.toVariant();

    devices.append(device);
  }

  return devices;
}

bool SessionStore::updateDeviceLastSeen(const QString& deviceId) {
  QSqlQuery query(*m_db);
  query.prepare(
      QStringLiteral("UPDATE android_devices SET last_seen = ? WHERE id = ?"));
  query.addBindValue(QDateTime::currentSecsSinceEpoch());
  query.addBindValue(deviceId);

  if (!query.exec()) {
    Logger::instance().error(QString("[SessionStore] Failed to update device last_seen: %1")
                                 .arg(query.lastError().text()));
    return false;
  }

  return true;
}

bool SessionStore::deleteDevice(const QString& deviceId) {
  QSqlQuery query(*m_db);
  query.prepare(QStringLiteral("DELETE FROM android_devices WHERE id = ?"));
  query.addBindValue(deviceId);

  if (!query.exec()) {
    Logger::instance().error(QString("[SessionStore] Failed to delete device: %1")
                                 .arg(query.lastError().text()));
    return false;
  }

  Logger::instance().info(QString("[SessionStore] Deleted device: %1").arg(deviceId));
  return true;
}

bool SessionStore::createSession(const QString& sessionId, const QString& deviceId,
                                 const QString& initialState) {
  const qint64 now = QDateTime::currentSecsSinceEpoch();

  QSqlQuery query(*m_db);
  query.prepare(
      QStringLiteral("INSERT INTO sessions "
                     "(id, device_id, state, started_at, last_heartbeat) "
                     "VALUES (?, ?, ?, ?, ?)"));

  query.addBindValue(sessionId);
  query.addBindValue(deviceId);
  query.addBindValue(initialState);
  query.addBindValue(now);
  query.addBindValue(now);

  if (!query.exec()) {
    Logger::instance().error(QString("[SessionStore] Failed to create session: %1")
                                 .arg(query.lastError().text()));
    return false;
  }

  Logger::instance().info(QString("[SessionStore] Created session: %1").arg(sessionId));
  return true;
}

QVariantMap SessionStore::getSession(const QString& sessionId) const {
  QSqlQuery query(*m_db);
  query.prepare(
      QStringLiteral("SELECT device_id, state, started_at, ended_at, last_heartbeat "
                     "FROM sessions WHERE id = ?"));
  query.addBindValue(sessionId);

  if (!query.exec() || !query.next()) {
    Logger::instance().warning(QString("[SessionStore] Session not found: %1").arg(sessionId));
    return QVariantMap();
  }

  QVariantMap session;
  session["id"] = sessionId;
  session["deviceId"] = query.value(0).toString();
  session["state"] = query.value(1).toString();
  session["startedAt"] = query.value(2).toLongLong();
  session["endedAt"] = query.value(3).toLongLong();
  session["lastHeartbeat"] = query.value(4).toLongLong();

  return session;
}

QVariantMap SessionStore::getSessionByDevice(const QString& deviceId) const {
  QSqlQuery query(*m_db);
  query.prepare(
      QStringLiteral("SELECT id, state, started_at, ended_at, last_heartbeat "
                     "FROM sessions WHERE device_id = ? AND state != 'ended'"));
  query.addBindValue(deviceId);

  if (!query.exec() || !query.next()) {
    Logger::instance().debug(
        QString("[SessionStore] No active session for device: %1").arg(deviceId));
    return QVariantMap();
  }

  QVariantMap session;
  session["id"] = query.value(0).toString();
  session["deviceId"] = deviceId;
  session["state"] = query.value(1).toString();
  session["startedAt"] = query.value(2).toLongLong();
  session["endedAt"] = query.value(3).toLongLong();
  session["lastHeartbeat"] = query.value(4).toLongLong();

  return session;
}

bool SessionStore::updateSessionState(const QString& sessionId, const QString& newState) {
  QSqlQuery query(*m_db);
  query.prepare(QStringLiteral("UPDATE sessions SET state = ? WHERE id = ?"));
  query.addBindValue(newState);
  query.addBindValue(sessionId);

  if (!query.exec()) {
    Logger::instance().error(QString("[SessionStore] Failed to update session state: %1")
                                 .arg(query.lastError().text()));
    return false;
  }

  Logger::instance().info(
      QString("[SessionStore] Session %1 state changed to: %2").arg(sessionId, newState));
  return true;
}

bool SessionStore::updateSessionHeartbeat(const QString& sessionId) {
  QSqlQuery query(*m_db);
  query.prepare(
      QStringLiteral("UPDATE sessions SET last_heartbeat = ? WHERE id = ?"));
  query.addBindValue(QDateTime::currentSecsSinceEpoch());
  query.addBindValue(sessionId);

  if (!query.exec()) {
    Logger::instance().error(QString("[SessionStore] Failed to update session heartbeat: %1")
                                 .arg(query.lastError().text()));
    return false;
  }

  return true;
}

bool SessionStore::endSession(const QString& sessionId) {
  QSqlQuery query(*m_db);
  query.prepare(
      QStringLiteral("UPDATE sessions SET state = 'ended', ended_at = ? WHERE id = ?"));
  query.addBindValue(QDateTime::currentSecsSinceEpoch());
  query.addBindValue(sessionId);

  if (!query.exec()) {
    Logger::instance().error(QString("[SessionStore] Failed to end session: %1")
                                 .arg(query.lastError().text()));
    return false;
  }

  Logger::instance().info(QString("[SessionStore] Session ended: %1").arg(sessionId));
  return true;
}
