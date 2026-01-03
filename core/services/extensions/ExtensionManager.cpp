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

#include "ExtensionManager.h"

#include <signal.h>

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonParseError>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>

ExtensionManager::ExtensionManager(const QString &extensionsDir, QObject *parent)
    : QObject(parent) {
  if (extensionsDir.isEmpty()) {
    m_extensionsDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) +
                      QStringLiteral("/crankshaft/extensions");
  } else {
    m_extensionsDir = extensionsDir;
  }

  // Create extensions directory if it doesn't exist
  QDir dir(m_extensionsDir);
  if (!dir.exists()) {
    dir.mkpath(m_extensionsDir);
  }

  // Load existing extensions
  QDir extensionsPath(m_extensionsDir);
  QStringList subdirs = extensionsPath.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

  for (const QString &subdir : subdirs) {
    QString manifestPath =
        m_extensionsDir + QStringLiteral("/") + subdir + QStringLiteral("/manifest.json");
    QFile manifestFile(manifestPath);
    if (manifestFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
      QJsonParseError parseError;
      QJsonDocument doc = QJsonDocument::fromJson(manifestFile.readAll(), &parseError);
      manifestFile.close();

      if (!doc.isNull() && doc.isObject()) {
        ExtensionInfo info =
            parseManifest(doc.object(), m_extensionsDir + QStringLiteral("/") + subdir);
        if (!info.id.isEmpty()) {
          m_extensions.insert(info.id, info);
        }
      }
    }
  }
}

ExtensionManager::~ExtensionManager() {
  // Stop all running extensions
  for (auto it = m_processes.begin(); it != m_processes.end(); ++it) {
    QProcess *process = it.value();
    if (process && process->state() == QProcess::Running) {
      process->terminate();
      if (!process->waitForFinished(3000)) {
        process->kill();
      }
    }
    delete process;
  }
}

ExtensionManager::ExtensionInfo ExtensionManager::parseManifest(const QJsonObject &manifest,
                                                                const QString &installDir) {
  ExtensionInfo info;
  info.id = manifest.value(QStringLiteral("id")).toString();
  info.name = manifest.value(QStringLiteral("name")).toString();
  info.version = manifest.value(QStringLiteral("version")).toString();
  info.entrypoint = manifest.value(QStringLiteral("entrypoint")).toString();
  info.installDir = installDir;
  info.isRunning = false;
  info.pid = -1;

  // Parse permissions
  QJsonArray permissionsArray = manifest.value(QStringLiteral("permissions")).toArray();
  for (const QJsonValue &perm : permissionsArray) {
    info.permissions.append(perm.toString());
  }

  // Parse background services
  QJsonArray servicesArray = manifest.value(QStringLiteral("background_services")).toArray();
  for (const QJsonValue &svc : servicesArray) {
    info.backgroundServices.append(svc.toString());
  }

  return info;
}

bool ExtensionManager::validateManifest(const QJsonObject &manifest) {
  // Validate required fields
  if (!manifest.contains(QStringLiteral("id")) || !manifest.contains(QStringLiteral("name")) ||
      !manifest.contains(QStringLiteral("version")) ||
      !manifest.contains(QStringLiteral("entrypoint")) ||
      !manifest.contains(QStringLiteral("permissions"))) {
    return false;
  }

  // Validate id format (alphanumeric, hyphen, underscore, dot)
  QString id = manifest.value(QStringLiteral("id")).toString();
  QRegularExpression idRegex(QStringLiteral("^[a-z0-9_.-]+$"));
  if (!idRegex.match(id).hasMatch()) {
    return false;
  }

  // Validate permissions array
  QJsonArray permissions = manifest.value(QStringLiteral("permissions")).toArray();
  QStringList allowedPermissions{QStringLiteral("ui.tile"),         QStringLiteral("media.source"),
                                 QStringLiteral("service.control"), QStringLiteral("network"),
                                 QStringLiteral("storage"),         QStringLiteral("diagnostics")};
  for (const QJsonValue &perm : permissions) {
    if (!allowedPermissions.contains(perm.toString())) {
      return false;
    }
  }

  return true;
}

QString ExtensionManager::installExtension(const QString &manifestJson, const QString &targetDir) {
  QJsonParseError parseError;
  QJsonDocument doc = QJsonDocument::fromJson(manifestJson.toUtf8(), &parseError);

  if (doc.isNull()) {
    return QStringLiteral("Failed to parse manifest: %1").arg(parseError.errorString());
  }

  if (!doc.isObject()) {
    return QStringLiteral("Manifest must be a JSON object");
  }

  if (!validateManifest(doc.object())) {
    return QStringLiteral("Manifest validation failed");
  }

  QString id = doc.object().value(QStringLiteral("id")).toString();
  QString installPath =
      targetDir.isEmpty() ? m_extensionsDir + QStringLiteral("/") + id : targetDir;

  // Create install directory
  QDir dir;
  if (!dir.mkpath(installPath)) {
    return QStringLiteral("Failed to create installation directory");
  }

  // Write manifest
  QFile manifestFile(installPath + QStringLiteral("/manifest.json"));
  if (!manifestFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
    return QStringLiteral("Failed to write manifest file");
  }
  manifestFile.write(manifestJson.toUtf8());
  manifestFile.close();

  // Parse and store extension info
  ExtensionInfo info = parseManifest(doc.object(), installPath);
  m_extensions.insert(id, info);

  emit extensionInstalled(id);
  return QString();  // Empty string indicates success
}

QString ExtensionManager::uninstallExtension(const QString &extensionId) {
  auto it = m_extensions.find(extensionId);
  if (it == m_extensions.end()) {
    return QStringLiteral("Extension not found: %1").arg(extensionId);
  }

  // Stop if running
  if (it.value().isRunning) {
    QString stopResult = stopExtension(extensionId);
    if (!stopResult.isEmpty()) {
      return stopResult;
    }
  }

  // Remove directory
  QDir dir(it.value().installDir);
  if (!dir.removeRecursively()) {
    return QStringLiteral("Failed to remove extension directory");
  }

  m_extensions.erase(it);
  emit extensionUninstalled(extensionId);
  return QString();
}

qint64 ExtensionManager::startProcess(const QString &extensionId, const ExtensionInfo &info) {
  QProcess *process = new QProcess();

  // Set working directory to extension install directory
  process->setWorkingDirectory(info.installDir);

  // Connect signals
  connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
          &ExtensionManager::onProcessFinished);
  connect(process, QOverload<QProcess::ProcessError>::of(&QProcess::errorOccurred), this,
          &ExtensionManager::onProcessError);

  // Start process
  process->start(info.entrypoint);

  if (!process->waitForStarted()) {
    emit extensionError(extensionId, QStringLiteral("Failed to start process"));
    delete process;
    return -1;
  }

  qint64 pid = process->processId();

  // Store process mapping
  m_processToExtension.insert(process, extensionId);
  m_processes.insert(extensionId, process);

  // Set up cgroup limits
  if (!setupCgroupLimits(pid, extensionId)) {
    qWarning() << "Failed to setup cgroup limits for extension:" << extensionId;
  }

  return pid;
}

QString ExtensionManager::startExtension(const QString &extensionId) {
  auto it = m_extensions.find(extensionId);
  if (it == m_extensions.end()) {
    return QStringLiteral("Extension not found: %1").arg(extensionId);
  }

  if (it.value().isRunning) {
    return QStringLiteral("Extension already running: %1").arg(extensionId);
  }

  qint64 pid = startProcess(extensionId, it.value());
  if (pid > 0) {
    it.value().isRunning = true;
    it.value().pid = pid;
    emit extensionStarted(extensionId, pid);
    return QString();
  }

  return QStringLiteral("Failed to start extension process");
}

bool ExtensionManager::stopProcess(qint64 pid) {
  if (pid <= 0) {
    return false;
  }

  // Try graceful terminate first
  if (kill(pid, SIGTERM) == 0) {
    // Wait a bit for graceful shutdown
    QProcess dummy;
    dummy.start(QStringLiteral("sh"), QStringList()
                                          << QStringLiteral("-c") << QStringLiteral("sleep 2"));
    dummy.waitForFinished();

    // Check if process is still running
    if (kill(pid, 0) != 0) {
      return true;  // Process terminated gracefully
    }
  }

  // Force kill if still running
  kill(pid, SIGKILL);
  return true;
}

QString ExtensionManager::stopExtension(const QString &extensionId) {
  auto extIt = m_extensions.find(extensionId);
  if (extIt == m_extensions.end()) {
    return QStringLiteral("Extension not found: %1").arg(extensionId);
  }

  if (!extIt.value().isRunning) {
    return QString();  // Not running, no error
  }

  auto procIt = m_processes.find(extensionId);
  if (procIt != m_processes.end()) {
    QProcess *process = procIt.value();
    process->terminate();
    if (!process->waitForFinished(3000)) {
      process->kill();
      process->waitForFinished();
    }
  } else {
    // Process not in our map, try killing by PID
    stopProcess(extIt.value().pid);
  }

  extIt.value().isRunning = false;
  extIt.value().pid = -1;

  emit extensionStopped(extensionId);
  return QString();
}

QString ExtensionManager::restartExtension(const QString &extensionId) {
  QString stopResult = stopExtension(extensionId);
  if (!stopResult.isEmpty()) {
    return stopResult;
  }

  return startExtension(extensionId);
}

QJsonArray ExtensionManager::listExtensions() {
  QJsonArray array;

  for (auto it = m_extensions.begin(); it != m_extensions.end(); ++it) {
    QJsonObject obj;
    obj.insert(QStringLiteral("id"), it.value().id);
    obj.insert(QStringLiteral("name"), it.value().name);
    obj.insert(QStringLiteral("version"), it.value().version);
    obj.insert(QStringLiteral("is_running"), it.value().isRunning);
    if (it.value().isRunning) {
      obj.insert(QStringLiteral("pid"), static_cast<qint64>(it.value().pid));
    }

    QJsonArray permsArray;
    for (const QString &perm : it.value().permissions) {
      permsArray.append(perm);
    }
    obj.insert(QStringLiteral("permissions"), permsArray);

    array.append(obj);
  }

  return array;
}

QJsonObject ExtensionManager::getExtensionInfo(const QString &extensionId) {
  auto it = m_extensions.find(extensionId);
  if (it == m_extensions.end()) {
    return QJsonObject();
  }

  QJsonObject obj;
  obj.insert(QStringLiteral("id"), it.value().id);
  obj.insert(QStringLiteral("name"), it.value().name);
  obj.insert(QStringLiteral("version"), it.value().version);
  obj.insert(QStringLiteral("entrypoint"), it.value().entrypoint);
  obj.insert(QStringLiteral("is_running"), it.value().isRunning);
  if (it.value().isRunning) {
    obj.insert(QStringLiteral("pid"), static_cast<qint64>(it.value().pid));
  }

  QJsonArray permsArray;
  for (const QString &perm : it.value().permissions) {
    permsArray.append(perm);
  }
  obj.insert(QStringLiteral("permissions"), permsArray);

  return obj;
}

bool ExtensionManager::hasPermission(const QString &extensionId, const QString &permission) {
  auto it = m_extensions.find(extensionId);
  if (it == m_extensions.end()) {
    return false;
  }

  return it.value().permissions.contains(permission);
}

QJsonArray ExtensionManager::getExtensionsWithPermission(const QString &permission) {
  QJsonArray array;

  for (auto it = m_extensions.begin(); it != m_extensions.end(); ++it) {
    if (it.value().permissions.contains(permission)) {
      QJsonObject obj;
      obj.insert(QStringLiteral("id"), it.value().id);
      obj.insert(QStringLiteral("name"), it.value().name);
      array.append(obj);
    }
  }

  return array;
}

bool ExtensionManager::setupCgroupLimits(qint64 pid, const QString &extensionId) {
  if (pid <= 0) {
    return false;
  }

  // cgroup v2 unified hierarchy path
  QString cgroupPath = QStringLiteral("/sys/fs/cgroup/crankshaft-extensions-%1").arg(extensionId);

  // Try to create cgroup
  QProcess mkdir;
  mkdir.start(QStringLiteral("mkdir"), QStringList() << QStringLiteral("-p") << cgroupPath);
  if (!mkdir.waitForFinished()) {
    return false;
  }

  // Add PID to cgroup
  QFile pidsFile(cgroupPath + QStringLiteral("/cgroup.procs"));
  if (!pidsFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
    return false;
  }
  pidsFile.write(QString::number(pid).toUtf8());
  pidsFile.close();

  // Apply resource limits
  return applyCgroupLimits(cgroupPath, EXTENSION_CPU_QUOTA_US, EXTENSION_MEMORY_LIMIT);
}

bool ExtensionManager::applyCgroupLimits(const QString &cgroupPath, qint64 cpuQuotaUs,
                                         qint64 memoryLimitBytes) {
  bool success = true;

  // Set CPU limits (cpu.max: quota period)
  QFile cpuFile(cgroupPath + QStringLiteral("/cpu.max"));
  if (cpuFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
    cpuFile.write(QString(QStringLiteral("%1 1000000")).arg(cpuQuotaUs).toUtf8());
    cpuFile.close();
  } else {
    success = false;
  }

  // Set memory limits
  QFile memFile(cgroupPath + QStringLiteral("/memory.max"));
  if (memFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
    memFile.write(QString::number(memoryLimitBytes).toUtf8());
    memFile.close();
  } else {
    success = false;
  }

  return success;
}

void ExtensionManager::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
  QProcess *process = qobject_cast<QProcess *>(sender());
  if (!process) {
    return;
  }

  auto it = m_processToExtension.find(process);
  if (it == m_processToExtension.end()) {
    return;
  }

  QString extensionId = it.value();

  // Update extension state
  auto extIt = m_extensions.find(extensionId);
  if (extIt != m_extensions.end()) {
    extIt.value().isRunning = false;
    extIt.value().pid = -1;
  }

  // Clean up process
  auto procIt = m_processes.find(extensionId);
  if (procIt != m_processes.end()) {
    delete procIt.value();
    m_processes.erase(procIt);
  }
  m_processToExtension.erase(it);

  // Emit signal based on exit status
  if (exitStatus == QProcess::CrashExit) {
    emit extensionCrashed(extensionId, exitCode);
  } else {
    emit extensionStopped(extensionId);
  }
}

void ExtensionManager::onProcessError(QProcess::ProcessError error) {
  QProcess *process = qobject_cast<QProcess *>(sender());
  if (!process) {
    return;
  }

  auto it = m_processToExtension.find(process);
  if (it == m_processToExtension.end()) {
    return;
  }

  QString extensionId = it.value();
  QString errorMsg;

  switch (error) {
    case QProcess::FailedToStart:
      errorMsg = QStringLiteral("Failed to start extension process");
      break;
    case QProcess::Crashed:
      errorMsg = QStringLiteral("Extension process crashed");
      break;
    case QProcess::Timedout:
      errorMsg = QStringLiteral("Extension process timed out");
      break;
    default:
      errorMsg = QStringLiteral("Unknown process error");
      break;
  }

  emit extensionError(extensionId, errorMsg);
}
