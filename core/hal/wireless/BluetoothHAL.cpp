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

#include "BluetoothHAL.h"
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusArgument>
#include <QDBusPendingCall>
#include <QDBusPendingReply>
#include <QDBusMetaType>
#include <QDebug>

// BlueZ DBus constants
static const char* BLUEZ_SERVICE = "org.bluez";
static const char* BLUEZ_ADAPTER_INTERFACE = "org.bluez.Adapter1";
static const char* BLUEZ_DEVICE_INTERFACE = "org.bluez.Device1";
static const char* BLUEZ_MEDIA_INTERFACE = "org.bluez.Media1";
static const char* BLUEZ_MEDIA_PLAYER_INTERFACE = "org.bluez.MediaPlayer1";
static const char* DBUS_PROPERTIES_INTERFACE = "org.freedesktop.DBus.Properties";
static const char* DBUS_OBJECT_MANAGER_INTERFACE = "org.freedesktop.DBus.ObjectManager";

class BluetoothHAL::BluetoothHALPrivate {
public:
    QDBusConnection dbusConnection;
    QString adapterPath;
    bool enabled = false;
    bool discovering = false;
    QList<BluetoothDevice> devices;
    QString connectedDevice;
    
    BluetoothHALPrivate() : dbusConnection(QDBusConnection::systemBus()) {
        qDBusRegisterMetaType<QMap<QString, QVariantMap>>();
    }
    
    bool findAdapter();
    QList<BluetoothDevice> parseDevices();
    BluetoothDevice parseDevice(const QString& devicePath, const QVariantMap& properties);
    BluetoothDevice::DeviceType guessDeviceType(const QStringList& uuids);
};

bool BluetoothHAL::BluetoothHALPrivate::findAdapter() {
    QDBusInterface manager(BLUEZ_SERVICE, "/", 
                          DBUS_OBJECT_MANAGER_INTERFACE, dbusConnection);
    
    if (!manager.isValid()) {
        qWarning() << "BlueZ ObjectManager not available:" << manager.lastError().message();
        return false;
    }
    
    QDBusReply<QMap<QDBusObjectPath, QMap<QString, QVariantMap>>> reply = 
        manager.call("GetManagedObjects");
    
    if (!reply.isValid()) {
        qWarning() << "Failed to get managed objects:" << reply.error().message();
        return false;
    }
    
    QMap<QDBusObjectPath, QMap<QString, QVariantMap>> objects = reply.value();
    
    for (auto it = objects.constBegin(); it != objects.constEnd(); ++it) {
        if (it.value().contains(BLUEZ_ADAPTER_INTERFACE)) {
            adapterPath = it.key().path();
            qDebug() << "Found Bluetooth adapter:" << adapterPath;
            return true;
        }
    }
    
    qWarning() << "No Bluetooth adapter found";
    return false;
}

QList<BluetoothDevice> BluetoothHAL::BluetoothHALPrivate::parseDevices() {
    QList<BluetoothDevice> result;
    
    QDBusInterface manager(BLUEZ_SERVICE, "/", 
                          DBUS_OBJECT_MANAGER_INTERFACE, dbusConnection);
    
    if (!manager.isValid()) {
        return result;
    }
    
    QDBusReply<QMap<QDBusObjectPath, QMap<QString, QVariantMap>>> reply = 
        manager.call("GetManagedObjects");
    
    if (!reply.isValid()) {
        return result;
    }
    
    QMap<QDBusObjectPath, QMap<QString, QVariantMap>> objects = reply.value();
    
    for (auto it = objects.constBegin(); it != objects.constEnd(); ++it) {
        if (it.value().contains(BLUEZ_DEVICE_INTERFACE)) {
            QString devicePath = it.key().path();
            QVariantMap properties = it.value()[BLUEZ_DEVICE_INTERFACE];
            BluetoothDevice device = parseDevice(devicePath, properties);
            if (!device.address.isEmpty()) {
                result.append(device);
            }
        }
    }
    
    return result;
}

BluetoothDevice BluetoothHAL::BluetoothHALPrivate::parseDevice(const QString& devicePath, 
                                                                const QVariantMap& properties) {
    BluetoothDevice device;
    device.path = devicePath;
    
    if (properties.contains("Name")) {
        device.name = properties["Name"].toString();
    }
    
    if (properties.contains("Address")) {
        device.address = properties["Address"].toString();
    }
    
    if (properties.contains("Paired")) {
        device.paired = properties["Paired"].toBool();
    }
    
    if (properties.contains("Connected")) {
        device.connected = properties["Connected"].toBool();
    }
    
    if (properties.contains("Trusted")) {
        device.trusted = properties["Trusted"].toBool();
    }
    
    if (properties.contains("RSSI")) {
        device.rssi = properties["RSSI"].toInt();
        // Convert RSSI to signal strength percentage (rough estimate)
        // RSSI typically ranges from -100 (weak) to -50 (strong)
        device.signalStrength = qMax(0, qMin(100, (device.rssi + 100) * 2));
    }
    
    if (properties.contains("UUIDs")) {
        QStringList uuids;
        const QDBusArgument arg = properties["UUIDs"].value<QDBusArgument>();
        arg.beginArray();
        while (!arg.atEnd()) {
            QString uuid;
            arg >> uuid;
            uuids.append(uuid);
        }
        arg.endArray();
        
        device.type = guessDeviceType(uuids);
    }
    
    return device;
}

BluetoothDevice::DeviceType BluetoothHAL::BluetoothHALPrivate::guessDeviceType(const QStringList& uuids) {
    // Common Bluetooth service UUIDs
    const QString AUDIO_SINK_UUID = "0000110b-0000-1000-8000-00805f9b34fb";
    const QString HANDSFREE_UUID = "0000111e-0000-1000-8000-00805f9b34fb";
    const QString HID_UUID = "00001124-0000-1000-8000-00805f9b34fb";
    const QString AVRCP_UUID = "0000110e-0000-1000-8000-00805f9b34fb";
    
    bool hasAudio = false;
    bool hasInput = false;
    
    for (const QString& uuid : uuids) {
        QString lowerUuid = uuid.toLower();
        if (lowerUuid.contains(AUDIO_SINK_UUID) || 
            lowerUuid.contains(HANDSFREE_UUID) || 
            lowerUuid.contains(AVRCP_UUID)) {
            hasAudio = true;
        }
        if (lowerUuid.contains(HID_UUID)) {
            hasInput = true;
        }
    }
    
    if (hasAudio && hasInput) {
        return BluetoothDevice::DeviceType::AudioAndInput;
    } else if (hasAudio) {
        return BluetoothDevice::DeviceType::Audio;
    } else if (hasInput) {
        return BluetoothDevice::DeviceType::Input;
    }
    
    return BluetoothDevice::DeviceType::Unknown;
}

BluetoothHAL::BluetoothHAL(QObject* parent)
    : QObject(parent)
    , d(new BluetoothHALPrivate()) {
    
    if (!d->dbusConnection.isConnected()) {
        qCritical() << "Cannot connect to system DBus";
        return;
    }
    
    // Find Bluetooth adapter
    if (d->findAdapter()) {
        // Check if adapter is powered
        QDBusInterface adapter(BLUEZ_SERVICE, d->adapterPath,
                              BLUEZ_ADAPTER_INTERFACE, d->dbusConnection);
        
        if (adapter.isValid()) {
            QVariant poweredVar = adapter.property("Powered");
            if (poweredVar.isValid()) {
                d->enabled = poweredVar.toBool();
            }
        }
        
        // Connect to InterfacesAdded signal for new devices
        d->dbusConnection.connect(
            BLUEZ_SERVICE,
            "/",
            DBUS_OBJECT_MANAGER_INTERFACE,
            "InterfacesAdded",
            this,
            SLOT(onInterfacesAdded(QDBusObjectPath, QMap<QString, QVariantMap>))
        );
        
        // Connect to InterfacesRemoved signal for removed devices
        d->dbusConnection.connect(
            BLUEZ_SERVICE,
            "/",
            DBUS_OBJECT_MANAGER_INTERFACE,
            "InterfacesRemoved",
            this,
            SLOT(onInterfacesRemoved(QDBusObjectPath, QStringList))
        );
        
        qDebug() << "Bluetooth HAL initialized successfully";
    } else {
        qWarning() << "Bluetooth HAL initialization failed - no adapter found";
    }
}

BluetoothHAL::~BluetoothHAL() {
    delete d;
}

bool BluetoothHAL::isEnabled() const {
    return d->enabled;
}

bool BluetoothHAL::setEnabled(bool enabled) {
    if (d->adapterPath.isEmpty()) {
        return false;
    }
    
    QDBusInterface adapter(BLUEZ_SERVICE, d->adapterPath,
                          BLUEZ_ADAPTER_INTERFACE, d->dbusConnection);
    
    if (!adapter.isValid()) {
        qWarning() << "Bluetooth adapter interface not available";
        return false;
    }
    
    adapter.setProperty("Powered", enabled);
    
    d->enabled = enabled;
    emit enabledChanged(enabled);
    qDebug() << "Bluetooth" << (enabled ? "enabled" : "disabled");
    
    return true;
}

bool BluetoothHAL::startDiscovery() {
    if (d->adapterPath.isEmpty()) {
        qWarning() << "No Bluetooth adapter available";
        return false;
    }
    
    QDBusInterface adapter(BLUEZ_SERVICE, d->adapterPath,
                          BLUEZ_ADAPTER_INTERFACE, d->dbusConnection);
    
    if (!adapter.isValid()) {
        qWarning() << "Bluetooth adapter interface not available";
        return false;
    }
    
    // Start discovery
    QDBusPendingCall pendingCall = adapter.asyncCall("StartDiscovery");
    QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(pendingCall, this);
    
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher]() {
        QDBusPendingReply<> reply = *watcher;
        if (reply.isError()) {
            qWarning() << "Failed to start discovery:" << reply.error().message();
            emit errorOccurred("Failed to start discovery: " + reply.error().message());
        } else {
            d->discovering = true;
            emit discoveryStateChanged(true);
            qDebug() << "Bluetooth discovery started";
        }
        watcher->deleteLater();
    });
    
    return true;
}

bool BluetoothHAL::stopDiscovery() {
    if (d->adapterPath.isEmpty()) {
        return false;
    }
    
    QDBusInterface adapter(BLUEZ_SERVICE, d->adapterPath,
                          BLUEZ_ADAPTER_INTERFACE, d->dbusConnection);
    
    if (!adapter.isValid()) {
        return false;
    }
    
    adapter.asyncCall("StopDiscovery");
    
    d->discovering = false;
    emit discoveryStateChanged(false);
    qDebug() << "Bluetooth discovery stopped";
    
    return true;
}

QList<BluetoothDevice> BluetoothHAL::getDevices() const {
    return d->devices;
}

bool BluetoothHAL::pairDevice(const QString& deviceAddress) {
    QString devicePath = findDevicePath(deviceAddress);
    if (devicePath.isEmpty()) {
        qWarning() << "Device not found:" << deviceAddress;
        return false;
    }
    
    QDBusInterface device(BLUEZ_SERVICE, devicePath,
                         BLUEZ_DEVICE_INTERFACE, d->dbusConnection);
    
    if (!device.isValid()) {
        qWarning() << "Device interface not available";
        return false;
    }
    
    // Pair device
    QDBusPendingCall pendingCall = device.asyncCall("Pair");
    QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(pendingCall, this);
    
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, deviceAddress, watcher]() {
        QDBusPendingReply<> reply = *watcher;
        if (reply.isError()) {
            qWarning() << "Failed to pair device:" << reply.error().message();
            emit pairingFailed(deviceAddress, reply.error().message());
        } else {
            qDebug() << "Device paired successfully:" << deviceAddress;
            emit devicePaired(deviceAddress);
            updateDeviceList();
        }
        watcher->deleteLater();
    });
    
    return true;
}

bool BluetoothHAL::connectDevice(const QString& deviceAddress) {
    QString devicePath = findDevicePath(deviceAddress);
    if (devicePath.isEmpty()) {
        qWarning() << "Device not found:" << deviceAddress;
        return false;
    }
    
    QDBusInterface device(BLUEZ_SERVICE, devicePath,
                         BLUEZ_DEVICE_INTERFACE, d->dbusConnection);
    
    if (!device.isValid()) {
        qWarning() << "Device interface not available";
        return false;
    }
    
    // Connect device
    QDBusPendingCall pendingCall = device.asyncCall("Connect");
    QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(pendingCall, this);
    
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, deviceAddress, watcher]() {
        QDBusPendingReply<> reply = *watcher;
        if (reply.isError()) {
            qWarning() << "Failed to connect device:" << reply.error().message();
            emit connectionFailed(deviceAddress, reply.error().message());
        } else {
            d->connectedDevice = deviceAddress;
            qDebug() << "Device connected successfully:" << deviceAddress;
            emit deviceConnected(deviceAddress);
            updateDeviceList();
        }
        watcher->deleteLater();
    });
    
    return true;
}

bool BluetoothHAL::disconnectDevice(const QString& deviceAddress) {
    QString devicePath = findDevicePath(deviceAddress);
    if (devicePath.isEmpty()) {
        return false;
    }
    
    QDBusInterface device(BLUEZ_SERVICE, devicePath,
                         BLUEZ_DEVICE_INTERFACE, d->dbusConnection);
    
    if (!device.isValid()) {
        return false;
    }
    
    device.asyncCall("Disconnect");
    
    if (d->connectedDevice == deviceAddress) {
        d->connectedDevice.clear();
    }
    
    emit deviceDisconnected(deviceAddress);
    qDebug() << "Device disconnected:" << deviceAddress;
    
    return true;
}

bool BluetoothHAL::removeDevice(const QString& deviceAddress) {
    QString devicePath = findDevicePath(deviceAddress);
    if (devicePath.isEmpty()) {
        return false;
    }
    
    QDBusInterface adapter(BLUEZ_SERVICE, d->adapterPath,
                          BLUEZ_ADAPTER_INTERFACE, d->dbusConnection);
    
    if (!adapter.isValid()) {
        return false;
    }
    
    adapter.asyncCall("RemoveDevice", QVariant::fromValue(QDBusObjectPath(devicePath)));
    
    qDebug() << "Device removed:" << deviceAddress;
    updateDeviceList();
    
    return true;
}

QString BluetoothHAL::getConnectedDevice() const {
    return d->connectedDevice;
}

void BluetoothHAL::updateDeviceList() {
    d->devices = d->parseDevices();
    emit devicesUpdated(d->devices);
    qDebug() << "Found" << d->devices.size() << "Bluetooth devices";
}

QString BluetoothHAL::findDevicePath(const QString& deviceAddress) const {
    for (const BluetoothDevice& device : d->devices) {
        if (device.address == deviceAddress) {
            return device.path;
        }
    }
    
    // Try to construct path
    QString cleanAddress = deviceAddress;
    cleanAddress.replace(":", "_");
    return d->adapterPath + "/dev_" + cleanAddress;
}

void BluetoothHAL::onInterfacesAdded(const QDBusObjectPath& objectPath, 
                                     const QMap<QString, QVariantMap>& interfaces) {
    if (interfaces.contains(BLUEZ_DEVICE_INTERFACE)) {
        qDebug() << "New Bluetooth device added:" << objectPath.path();
        updateDeviceList();
    }
}

void BluetoothHAL::onInterfacesRemoved(const QDBusObjectPath& objectPath, 
                                       const QStringList& interfaces) {
    if (interfaces.contains(BLUEZ_DEVICE_INTERFACE)) {
        qDebug() << "Bluetooth device removed:" << objectPath.path();
        updateDeviceList();
    }
}
