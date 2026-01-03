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

#ifndef EXTENSIONMANAGER_H
#define EXTENSIONMANAGER_H

#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMap>
#include <QProcess>
#include <QTemporaryDir>

class ExtensionManager : public QObject {
    Q_OBJECT

public:
    struct ExtensionInfo {
        QString id;
        QString name;
        QString version;
        QString entrypoint;
        QStringList permissions;
        QStringList backgroundServices;
        QString installDir;
        qint64 pid;  // Process ID if running
        bool isRunning;
    };

    explicit ExtensionManager(const QString &extensionsDir = QString(), QObject *parent = nullptr);
    ~ExtensionManager();

    // Install or update an extension from a JSON manifest
    // Returns: error message (empty if successful)
    Q_INVOKABLE QString installExtension(const QString &manifestJson, const QString &targetDir);

    // Uninstall an extension
    Q_INVOKABLE QString uninstallExtension(const QString &extensionId);

    // Start an extension process
    Q_INVOKABLE QString startExtension(const QString &extensionId);

    // Stop an extension process
    Q_INVOKABLE QString stopExtension(const QString &extensionId);

    // Restart an extension process
    Q_INVOKABLE QString restartExtension(const QString &extensionId);

    // List all installed extensions
    Q_INVOKABLE QJsonArray listExtensions();

    // Get extension info
    Q_INVOKABLE QJsonObject getExtensionInfo(const QString &extensionId);

    // Check if extension has a specific permission
    Q_INVOKABLE bool hasPermission(const QString &extensionId, const QString &permission);

    // Get all extensions with a specific permission
    Q_INVOKABLE QJsonArray getExtensionsWithPermission(const QString &permission);

signals:
    void extensionInstalled(const QString &extensionId);
    void extensionUninstalled(const QString &extensionId);
    void extensionStarted(const QString &extensionId, qint64 pid);
    void extensionStopped(const QString &extensionId);
    void extensionCrashed(const QString &extensionId, int exitCode);
    void extensionError(const QString &extensionId, const QString &error);
    void permissionDenied(const QString &extensionId, const QString &permission);

private slots:
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessError(QProcess::ProcessError error);

private:
    // Parse extension manifest from JSON
    ExtensionInfo parseManifest(const QJsonObject &manifest, const QString &installDir);

    // Validate manifest against schema
    bool validateManifest(const QJsonObject &manifest);

    // Set up cgroup v2 resource limits for extension process
    bool setupCgroupLimits(qint64 pid, const QString &extensionId);

    // Start supervised process with error handling
    qint64 startProcess(const QString &extensionId, const ExtensionInfo &info);

    // Stop process gracefully with fallback to kill
    bool stopProcess(qint64 pid);

    // Apply resource limits to cgroup
    bool applyCgroupLimits(const QString &cgroupPath, qint64 cpuQuotaUs, qint64 memoryLimitBytes);

    QString m_extensionsDir;
    QMap<QString, ExtensionInfo> m_extensions;
    QMap<QString, QProcess *> m_processes;
    QMap<QProcess *, QString> m_processToExtension;  // Reverse mapping for signal handlers

    // Cgroup resource limits (can be configured)
    static constexpr qint64 EXTENSION_CPU_QUOTA_US = 500000;  // 0.5s per 1s
    static constexpr qint64 EXTENSION_MEMORY_LIMIT = 512 * 1024 * 1024;  // 512 MB
};

#endif  // EXTENSIONMANAGER_H
