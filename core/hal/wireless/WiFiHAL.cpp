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

#include "WiFiHAL.h"

#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QDBusPendingReply>
#include <QDBusReply>
#include <QDebug>
#include <QTimer>
#include <QUuid>

// NetworkManager DBus constants
static const char* NM_DBUS_SERVICE = "org.freedesktop.NetworkManager";
static const char* NM_DBUS_PATH = "/org/freedesktop/NetworkManager";
static const char* NM_DBUS_INTERFACE = "org.freedesktop.NetworkManager";
static const char* NM_DBUS_INTERFACE_DEVICE = "org.freedesktop.NetworkManager.Device";
static const char* NM_DBUS_INTERFACE_DEVICE_WIRELESS =
    "org.freedesktop.NetworkManager.Device.Wireless";
static const char* NM_DBUS_INTERFACE_ACCESS_POINT = "org.freedesktop.NetworkManager.AccessPoint";
static const char* NM_DBUS_INTERFACE_SETTINGS = "org.freedesktop.NetworkManager.Settings";
static const char* NM_DBUS_INTERFACE_CONNECTION =
    "org.freedesktop.NetworkManager.Settings.Connection";

// NetworkManager device types
enum NMDeviceType {
  NM_DEVICE_TYPE_UNKNOWN = 0,
  NM_DEVICE_TYPE_ETHERNET = 1,
  NM_DEVICE_TYPE_WIFI = 2,
  NM_DEVICE_TYPE_BLUETOOTH = 5,
};

// NetworkManager device states
enum NMDeviceState {
  NM_DEVICE_STATE_UNKNOWN = 0,
  NM_DEVICE_STATE_UNMANAGED = 10,
  NM_DEVICE_STATE_UNAVAILABLE = 20,
  NM_DEVICE_STATE_DISCONNECTED = 30,
  NM_DEVICE_STATE_PREPARE = 40,
  NM_DEVICE_STATE_CONFIG = 50,
  NM_DEVICE_STATE_NEED_AUTH = 60,
  NM_DEVICE_STATE_IP_CONFIG = 70,
  NM_DEVICE_STATE_IP_CHECK = 80,
  NM_DEVICE_STATE_SECONDARIES = 90,
  NM_DEVICE_STATE_ACTIVATED = 100,
  NM_DEVICE_STATE_DEACTIVATING = 110,
  NM_DEVICE_STATE_FAILED = 120,
};

class WiFiHAL::WiFiHALPrivate {
 public:
  QDBusConnection dbusConnection;
  QString wirelessDevicePath;
  bool enabled = false;
  bool scanning = false;
  QList<WiFiNetwork> networks;
  QString connectedSSID;

  WiFiHALPrivate() : dbusConnection(QDBusConnection::systemBus()) {}

  bool findWirelessDevice();
  QList<WiFiNetwork> parseAccessPoints(const QList<QDBusObjectPath>& apPaths);
  WiFiNetwork parseAccessPoint(const QString& apPath);
  int getSignalQuality(int signalStrength);
};

bool WiFiHAL::WiFiHALPrivate::findWirelessDevice() {
  QDBusInterface nm(NM_DBUS_SERVICE, NM_DBUS_PATH, NM_DBUS_INTERFACE, dbusConnection);

  if (!nm.isValid()) {
    qWarning() << "NetworkManager DBus interface not available:" << nm.lastError().message();
    return false;
  }

  QDBusReply<QList<QDBusObjectPath>> reply = nm.call("GetDevices");
  if (!reply.isValid()) {
    qWarning() << "Failed to get devices from NetworkManager:" << reply.error().message();
    return false;
  }

  QList<QDBusObjectPath> devices = reply.value();
  for (const QDBusObjectPath& devicePath : devices) {
    QDBusInterface device(NM_DBUS_SERVICE, devicePath.path(), NM_DBUS_INTERFACE_DEVICE,
                          dbusConnection);

    if (!device.isValid()) {
      continue;
    }

    QVariant deviceTypeVar = device.property("DeviceType");
    if (deviceTypeVar.isValid() && deviceTypeVar.toInt() == NM_DEVICE_TYPE_WIFI) {
      wirelessDevicePath = devicePath.path();
      qDebug() << "Found wireless device:" << wirelessDevicePath;
      return true;
    }
  }

  qWarning() << "No wireless device found";
  return false;
}

QList<WiFiNetwork> WiFiHAL::WiFiHALPrivate::parseAccessPoints(
    const QList<QDBusObjectPath>& apPaths) {
  QList<WiFiNetwork> result;

  for (const QDBusObjectPath& apPath : apPaths) {
    WiFiNetwork network = parseAccessPoint(apPath.path());
    if (!network.ssid.isEmpty()) {
      result.append(network);
    }
  }

  return result;
}

WiFiNetwork WiFiHAL::WiFiHALPrivate::parseAccessPoint(const QString& apPath) {
  WiFiNetwork network;

  QDBusInterface ap(NM_DBUS_SERVICE, apPath, NM_DBUS_INTERFACE_ACCESS_POINT, dbusConnection);

  if (!ap.isValid()) {
    return network;
  }

  // Get SSID
  QVariant ssidVar = ap.property("Ssid");
  if (ssidVar.isValid()) {
    QByteArray ssidBytes = ssidVar.toByteArray();
    network.ssid = QString::fromUtf8(ssidBytes);
  }

  // Get signal strength (0-100)
  QVariant strengthVar = ap.property("Strength");
  if (strengthVar.isValid()) {
    network.signalStrength = strengthVar.toInt();
  }

  // Get security flags
  QVariant flagsVar = ap.property("Flags");
  QVariant wpaFlagsVar = ap.property("WpaFlags");
  QVariant rsnFlagsVar = ap.property("RsnFlags");

  bool hasPrivacy = flagsVar.isValid() && (flagsVar.toUInt() & 0x1);
  bool hasWPA = wpaFlagsVar.isValid() && wpaFlagsVar.toUInt() != 0;
  bool hasRSN = rsnFlagsVar.isValid() && rsnFlagsVar.toUInt() != 0;

  if (hasRSN) {
    network.security = WiFiSecurity::WPA2_PSK;
  } else if (hasWPA) {
    network.security = WiFiSecurity::WPA_PSK;
  } else if (hasPrivacy) {
    network.security = WiFiSecurity::WEP;
  } else {
    network.security = WiFiSecurity::Open;
  }

  // Get frequency/channel
  QVariant frequencyVar = ap.property("Frequency");
  if (frequencyVar.isValid()) {
    uint frequency = frequencyVar.toUInt();
    // Rough channel estimation
    // Channel can be derived from frequency if needed
  }

  network.connected = false;

  return network;
}

int WiFiHAL::WiFiHALPrivate::getSignalQuality(int signalStrength) {
  // Convert 0-100 signal strength to quality percentage
  if (signalStrength >= 80) return 100;
  if (signalStrength >= 60) return 75;
  if (signalStrength >= 40) return 50;
  if (signalStrength >= 20) return 25;
  return 10;
}

WiFiHAL::WiFiHAL(QObject* parent) : QObject(parent), d(new WiFiHALPrivate()) {
  if (!d->dbusConnection.isConnected()) {
    qCritical() << "Cannot connect to system DBus";
    return;
  }

  // Find wireless device
  if (d->findWirelessDevice()) {
    d->enabled = true;

    // Connect to PropertyChanged signal for device state changes
    d->dbusConnection.connect(NM_DBUS_SERVICE, d->wirelessDevicePath,
                              "org.freedesktop.DBus.Properties", "PropertiesChanged", this,
                              SLOT(onDevicePropertiesChanged(QString, QVariantMap, QStringList)));

    qDebug() << "WiFi HAL initialized successfully";
  } else {
    qWarning() << "WiFi HAL initialization failed - no device found";
  }
}

WiFiHAL::~WiFiHAL() {
  delete d;
}

bool WiFiHAL::isEnabled() const {
  return d->enabled;
}

bool WiFiHAL::setEnabled(bool enabled) {
  if (d->wirelessDevicePath.isEmpty()) {
    return false;
  }

  QDBusInterface nm(NM_DBUS_SERVICE, NM_DBUS_PATH, NM_DBUS_INTERFACE, d->dbusConnection);

  if (!nm.isValid()) {
    qWarning() << "NetworkManager interface not available";
    return false;
  }

  // Enable/disable wireless globally
  nm.setProperty("WirelessEnabled", enabled);

  d->enabled = enabled;
  emit enabledChanged(enabled);
  qDebug() << "WiFi" << (enabled ? "enabled" : "disabled");

  return true;
}

bool WiFiHAL::startScan() {
  if (d->wirelessDevicePath.isEmpty()) {
    qWarning() << "No wireless device available";
    return false;
  }

  QDBusInterface wireless(NM_DBUS_SERVICE, d->wirelessDevicePath, NM_DBUS_INTERFACE_DEVICE_WIRELESS,
                          d->dbusConnection);

  if (!wireless.isValid()) {
    qWarning() << "Wireless device interface not available";
    return false;
  }

  // Request scan
  QVariantMap options;
  QDBusPendingCall pendingCall = wireless.asyncCall("RequestScan", QVariant::fromValue(options));

  d->scanning = true;
  emit scanningChanged(true);

  // Wait a moment and then get results
  QTimer::singleShot(3000, this, [this]() {
    updateNetworkList();
    d->scanning = false;
    emit scanningChanged(false);
  });

  qDebug() << "WiFi scan started";
  return true;
}

QList<WiFiNetwork> WiFiHAL::getAvailableNetworks() const {
  return d->networks;
}

bool WiFiHAL::connectToNetwork(const QString& ssid, const QString& password,
                               WiFiSecurity security) {
  if (d->wirelessDevicePath.isEmpty()) {
    return false;
  }

  QDBusInterface nm(NM_DBUS_SERVICE, NM_DBUS_PATH, NM_DBUS_INTERFACE, d->dbusConnection);
  QDBusInterface settings(NM_DBUS_SERVICE, "/org/freedesktop/NetworkManager/Settings",
                          NM_DBUS_INTERFACE_SETTINGS, d->dbusConnection);

  if (!nm.isValid() || !settings.isValid()) {
    qWarning() << "NetworkManager interfaces not available";
    return false;
  }

  // Build connection settings
  QVariantMap connection;
  connection["id"] = ssid;
  connection["type"] = "802-11-wireless";
  connection["uuid"] = QUuid::createUuid().toString().remove('{').remove('}');

  QVariantMap wireless;
  wireless["ssid"] = ssid.toUtf8();
  wireless["mode"] = "infrastructure";

  QVariantMap ipv4;
  ipv4["method"] = "auto";

  QVariantMap ipv6;
  ipv6["method"] = "auto";

  QVariantMap connectionSettings;
  connectionSettings["connection"] = connection;
  connectionSettings["802-11-wireless"] = wireless;
  connectionSettings["ipv4"] = ipv4;
  connectionSettings["ipv6"] = ipv6;

  // Add security settings
  if (security != WiFiSecurity::Open && !password.isEmpty()) {
    QVariantMap wirelessSecurity;

    switch (security) {
      case WiFiSecurity::WEP:
        wirelessSecurity["key-mgmt"] = "none";
        wirelessSecurity["wep-key0"] = password;
        wirelessSecurity["wep-key-type"] = 1;  // ASCII/Hex
        break;
      case WiFiSecurity::WPA_PSK:
      case WiFiSecurity::WPA2_PSK:
        wirelessSecurity["key-mgmt"] = "wpa-psk";
        wirelessSecurity["psk"] = password;
        break;
      case WiFiSecurity::WPA3_SAE:
        wirelessSecurity["key-mgmt"] = "sae";
        wirelessSecurity["psk"] = password;
        break;
      default:
        break;
    }

    connectionSettings["802-11-wireless-security"] = wirelessSecurity;
  }

  // Add connection
  QDBusPendingReply<QDBusObjectPath> addReply =
      settings.asyncCall("AddConnection", QVariant::fromValue(connectionSettings));
  addReply.waitForFinished();

  if (addReply.isError()) {
    qWarning() << "Failed to add connection:" << addReply.error().message();
    return false;
  }

  QDBusObjectPath connectionPath = addReply.value();

  // Activate connection
  QDBusPendingReply<QDBusObjectPath> activateReply =
      nm.asyncCall("ActivateConnection", QVariant::fromValue(connectionPath),
                   QVariant::fromValue(QDBusObjectPath(d->wirelessDevicePath)),
                   QVariant::fromValue(QDBusObjectPath("/")));

  activateReply.waitForFinished();

  if (activateReply.isError()) {
    qWarning() << "Failed to activate connection:" << activateReply.error().message();
    return false;
  }

  d->connectedSSID = ssid;
  qDebug() << "Connecting to WiFi network:" << ssid;
  emit connectionStateChanged(ConnectionState::Connecting);

  return true;
}

bool WiFiHAL::disconnect() {
  if (d->wirelessDevicePath.isEmpty()) {
    return false;
  }

  QDBusInterface device(NM_DBUS_SERVICE, d->wirelessDevicePath, NM_DBUS_INTERFACE_DEVICE,
                        d->dbusConnection);

  if (!device.isValid()) {
    return false;
  }

  device.asyncCall("Disconnect");

  d->connectedSSID.clear();
  emit connectionStateChanged(ConnectionState::Disconnected);
  qDebug() << "Disconnected from WiFi";

  return true;
}

QString WiFiHAL::getConnectedSSID() const {
  return d->connectedSSID;
}

int WiFiHAL::getSignalStrength() const {
  // Get current connection's signal strength
  for (const WiFiNetwork& network : d->networks) {
    if (network.ssid == d->connectedSSID && network.connected) {
      return network.signalStrength;
    }
  }
  return 0;
}

QString WiFiHAL::getIPAddress() const {
  if (d->wirelessDevicePath.isEmpty()) {
    return QString();
  }

  QDBusInterface device(NM_DBUS_SERVICE, d->wirelessDevicePath, NM_DBUS_INTERFACE_DEVICE,
                        d->dbusConnection);

  if (!device.isValid()) {
    return QString();
  }

  // Get IP4Config object path
  QVariant ip4ConfigVar = device.property("Ip4Config");
  if (!ip4ConfigVar.isValid()) {
    return QString();
  }

  QString ip4ConfigPath = ip4ConfigVar.value<QDBusObjectPath>().path();
  if (ip4ConfigPath == "/") {
    return QString();  // Not configured
  }

  QDBusInterface ip4Config(NM_DBUS_SERVICE, ip4ConfigPath,
                           "org.freedesktop.NetworkManager.IP4Config", d->dbusConnection);

  if (!ip4Config.isValid()) {
    return QString();
  }

  // Get address data
  QVariant addressDataVar = ip4Config.property("AddressData");
  if (addressDataVar.isValid()) {
    QDBusArgument arg = addressDataVar.value<QDBusArgument>();
    arg.beginArray();
    while (!arg.atEnd()) {
      QVariantMap addressMap;
      arg >> addressMap;
      if (addressMap.contains("address")) {
        return addressMap["address"].toString();
      }
    }
    arg.endArray();
  }

  return QString();
}

void WiFiHAL::updateNetworkList() {
  if (d->wirelessDevicePath.isEmpty()) {
    return;
  }

  QDBusInterface wireless(NM_DBUS_SERVICE, d->wirelessDevicePath, NM_DBUS_INTERFACE_DEVICE_WIRELESS,
                          d->dbusConnection);

  if (!wireless.isValid()) {
    return;
  }

  // Get access points
  QDBusReply<QList<QDBusObjectPath>> reply = wireless.call("GetAccessPoints");
  if (!reply.isValid()) {
    qWarning() << "Failed to get access points:" << reply.error().message();
    return;
  }

  d->networks = d->parseAccessPoints(reply.value());

  // Mark connected network
  if (!d->connectedSSID.isEmpty()) {
    for (WiFiNetwork& network : d->networks) {
      if (network.ssid == d->connectedSSID) {
        network.connected = true;
        break;
      }
    }
  }

  emit networksUpdated(d->networks);
  qDebug() << "Found" << d->networks.size() << "WiFi networks";
}

void WiFiHAL::onDevicePropertiesChanged(const QString& interface,
                                        const QVariantMap& changedProperties,
                                        const QStringList& invalidatedProperties) {
  Q_UNUSED(interface)
  Q_UNUSED(invalidatedProperties)

  if (changedProperties.contains("State")) {
    int state = changedProperties["State"].toInt();

    switch (state) {
      case NM_DEVICE_STATE_ACTIVATED:
        emit connectionStateChanged(ConnectionState::Connected);
        updateNetworkList();
        break;
      case NM_DEVICE_STATE_DISCONNECTED:
      case NM_DEVICE_STATE_FAILED:
        emit connectionStateChanged(ConnectionState::Disconnected);
        d->connectedSSID.clear();
        break;
      case NM_DEVICE_STATE_PREPARE:
      case NM_DEVICE_STATE_CONFIG:
      case NM_DEVICE_STATE_NEED_AUTH:
      case NM_DEVICE_STATE_IP_CONFIG:
        emit connectionStateChanged(ConnectionState::Connecting);
        break;
      default:
        break;
    }
  }
}
