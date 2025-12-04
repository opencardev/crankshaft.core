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
  readonly property string appTitle: "Crankshaft"
  readonly property string buttonBack: "Back"
  readonly property string buttonSettings: "Settings"

  readonly property string sectionAppearance: "Appearance"
  readonly property string sectionLanguage: "Language"
  readonly property string sectionConnection: "Connection"
  readonly property string labelDarkMode: "Dark Mode"
  readonly property string labelLanguage: "Language"
  readonly property string labelStatus: "Status"

  // Languages
  readonly property string langEnGb: "English (GB)"
  readonly property string langDeDe: "Deutsch (DE)"

  // Connection statuses
  readonly property string statusConnected: "Connected"
  readonly property string statusDisconnected: "Disconnected"
  readonly property string statusNotConnected: "Not connected"

  // Home screen
  readonly property string homeWelcome: "Welcome to Crankshaft MVP"
  readonly property string cardNavigationTitle: "Navigation"
  readonly property string cardNavigationDesc: "GPS and route planning"
  readonly property string cardMediaTitle: "Media"
  readonly property string cardMediaDesc: "Music and audio"
  readonly property string cardPhoneTitle: "Phone"
  readonly property string cardPhoneDesc: "Calls and contacts"
  readonly property string cardSystemTitle: "System"
  readonly property string cardSystemDesc: "Settings and info"
  readonly property string cardAndroidAutoTitle: "Android Auto"
  readonly property string cardAndroidAutoDesc: "Connect your phone"

  // Android Auto page
  readonly property string aaTitle: "Android Auto"
  readonly property string aaStart: "Start Android Auto"
  readonly property string aaStop: "Stop Android Auto"
}
