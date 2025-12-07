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
pragma Singleton
import QtQuick

QtObject {
  // Common labels/sections
  readonly property string appTitle: qsTr("Crankshaft")
  readonly property string buttonBack: qsTr("Back")
  readonly property string buttonSettings: qsTr("Settings")

  readonly property string sectionAppearance: qsTr("Appearance")
  readonly property string sectionLanguage: qsTr("Language")
  readonly property string sectionConnection: qsTr("Connection")
  readonly property string labelDarkMode: qsTr("Dark Mode")
  readonly property string labelLanguage: qsTr("Language")
  readonly property string labelStatus: qsTr("Status")

  // Languages
  readonly property string langEnGb: qsTr("English (GB)")
  readonly property string langDeDe: qsTr("Deutsch (DE)")

  // Connection statuses
  readonly property string statusConnected: qsTr("Connected")
  readonly property string statusDisconnected: qsTr("Disconnected")
  readonly property string statusNotConnected: qsTr("Not connected")

  // Home screen
  readonly property string homeWelcome: qsTr("Welcome to Crankshaft MVP")
  readonly property string cardNavigationTitle: qsTr("Navigation")
  readonly property string cardNavigationDesc: qsTr("GPS and route planning")
  readonly property string cardMediaTitle: qsTr("Media")
  readonly property string cardMediaDesc: qsTr("Music and audio")
  readonly property string cardPhoneTitle: qsTr("Phone")
  readonly property string cardPhoneDesc: qsTr("Calls and contacts")
  readonly property string cardSystemTitle: qsTr("System")
  readonly property string cardSystemDesc: qsTr("Settings and info")
  readonly property string cardAndroidAutoTitle: qsTr("Android Auto")
  readonly property string cardAndroidAutoDesc: qsTr("Connect your phone")

  // Android Auto page
  readonly property string aaTitle: qsTr("Android Auto")
  readonly property string aaStart: qsTr("Start Android Auto")
  readonly property string aaStop: qsTr("Stop Android Auto")
}
