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

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Crankshaft

Page {
  id: profilesPage
  title: qsTr('Device Profiles')

  background: Rectangle {
    color: Theme.backgroundColor
  }

  ColumnLayout {
    anchors {
      fill: parent
      margins: 20
    }
    spacing: 20

    // Host Profiles Section
    Text {
      text: qsTr('Host Configuration')
      font.pixelSize: 20
      font.bold: true
      color: Theme.textColor
    }

    Rectangle {
      Layout.fillWidth: true
      height: 200
      color: Theme.backgroundColor
      border.color: Theme.accentColor
      border.width: 1
      radius: 8

      ColumnLayout {
        anchors {
          fill: parent
          margins: 15
        }
        spacing: 10

        Text {
          text: qsTr('Active Host: ') + 'Development Host'
          color: Theme.textColor
          font.pixelSize: 14
        }

        ComboBox {
          Layout.fillWidth: true
          model: ['Development Host', 'Production Host', 'Test Host']
          onCurrentIndexChanged: {
            // TODO: Update active host profile
          }
        }

        Button {
          Layout.fillWidth: true
          text: qsTr('Manage Host Profiles')
          onClicked: hostProfilesDialog.open()
        }
      }
    }

    // Vehicle Profiles Section
    Text {
      text: qsTr('Vehicle Configuration')
      font.pixelSize: 20
      font.bold: true
      color: Theme.textColor
    }

    Rectangle {
      Layout.fillWidth: true
      height: 200
      color: Theme.backgroundColor
      border.color: Theme.accentColor
      border.width: 1
      radius: 8

      ColumnLayout {
        anchors {
          fill: parent
          margins: 15
        }
        spacing: 10

        Text {
          text: qsTr('Active Vehicle: ') + 'Test Vehicle - Sedan'
          color: Theme.textColor
          font.pixelSize: 14
        }

        ComboBox {
          Layout.fillWidth: true
          model: ['Test Vehicle - Sedan', 'Test Vehicle - SUV', 'Test Vehicle - Truck']
          onCurrentIndexChanged: {
            // TODO: Update active vehicle profile
          }
        }

        Button {
          Layout.fillWidth: true
          text: qsTr('Manage Vehicle Profiles')
          onClicked: vehicleProfilesDialog.open()
        }
      }
    }

    // Device Management Section
    Text {
      text: qsTr('Device Configuration')
      font.pixelSize: 20
      font.bold: true
      color: Theme.textColor
    }

    Rectangle {
      Layout.fillWidth: true
      Layout.fillHeight: true
      color: Theme.backgroundColor
      border.color: Theme.accentColor
      border.width: 1
      radius: 8

      ListView {
        anchors {
          fill: parent
          margins: 10
        }
        spacing: 10
        clip: true

        model: [
          {
            name: 'CAN Bus',
            type: 'CAN_BUS',
            enabled: true,
            useMock: true,
            description: 'Vehicle data bus interface'
          },
          {
            name: 'GPS',
            type: 'GPS',
            enabled: true,
            useMock: true,
            description: 'Global positioning system'
          },
          {
            name: 'Bluetooth',
            type: 'BLUETOOTH',
            enabled: true,
            useMock: true,
            description: 'Wireless connectivity'
          },
          {
            name: 'WiFi',
            type: 'WIRELESS',
            enabled: true,
            useMock: true,
            description: 'Wireless networking'
          },
          {
            name: 'I2C Bus',
            type: 'I2C',
            enabled: true,
            useMock: true,
            description: 'Inter-integrated circuit bus'
          },
          {
            name: 'UART Serial',
            type: 'UART',
            enabled: true,
            useMock: true,
            description: 'Serial communication interface'
          }
        ]

        delegate: Rectangle {
          width: parent.width
          height: 80
          color: Theme.backgroundColor
          border.color: Theme.accentColor
          border.width: 1
          radius: 6

          RowLayout {
            anchors {
              fill: parent
              margins: 10
            }
            spacing: 15

            Column {
              Layout.fillWidth: true
              spacing: 5

              Text {
                text: modelData.name
                font.pixelSize: 14
                font.bold: true
                color: Theme.textColor
              }

              Text {
                text: modelData.description
                font.pixelSize: 12
                color: Theme.secondaryTextColor
              }

              Text {
                text: 'Type: ' + modelData.type
                font.pixelSize: 11
                color: Theme.secondaryTextColor
              }
            }

            Column {
              spacing: 8

              Switch {
                checked: modelData.enabled
                onToggled: {
                  // TODO: Toggle device enabled state
                }
              }

              Text {
                text: modelData.useMock ? 'Using Mock' : 'Real Hardware'
                font.pixelSize: 11
                color: Theme.secondaryTextColor
              }
            }

            Button {
              text: modelData.useMock ? 'Mock' : 'Real'
              onClicked: {
                // TODO: Toggle mock/real hardware
              }
            }
          }
        }
      }
    }

    Item {
      Layout.fillHeight: true
    }

    RowLayout {
      spacing: 10

      Button {
        Layout.fillWidth: true
        text: qsTr('Back')
        onClicked: parent.parent.parent.pop()
      }

      Button {
        Layout.fillWidth: true
        text: qsTr('Save & Restart')
        onClicked: {
          // TODO: Save profiles and restart application
        }
      }
    }
  }

  // Host Profiles Dialog
  Dialog {
    id: hostProfilesDialog
    title: qsTr('Host Profiles')
    anchors.centerIn: parent
    width: 400
    height: 300

    ColumnLayout {
      anchors.fill: parent
      spacing: 10

      ListView {
        Layout.fillWidth: true
        Layout.fillHeight: true
        clip: true

        model: ['Development Host', 'Production Host', 'Test Host']

        delegate: Rectangle {
          width: parent.width
          height: 50
          color: Theme.backgroundColor
          border.color: Theme.accentColor
          border.width: 1

          Text {
            anchors {
              fill: parent
              margins: 10
            }
            text: modelData
            color: Theme.textColor
            verticalAlignment: Text.AlignVCenter
          }

          MouseArea {
            anchors.fill: parent
            onClicked: {
              // TODO: Select profile
            }
          }
        }
      }

      RowLayout {
        spacing: 10

        Button {
          Layout.fillWidth: true
          text: qsTr('Add')
          onClicked: {
            // TODO: Add new host profile
          }
        }

        Button {
          Layout.fillWidth: true
          text: qsTr('Edit')
          onClicked: {
            // TODO: Edit selected profile
          }
        }

        Button {
          Layout.fillWidth: true
          text: qsTr('Delete')
          onClicked: {
            // TODO: Delete selected profile
          }
        }

        Button {
          text: qsTr('Close')
          onClicked: hostProfilesDialog.close()
        }
      }
    }
  }

  // Vehicle Profiles Dialog
  Dialog {
    id: vehicleProfilesDialog
    title: qsTr('Vehicle Profiles')
    anchors.centerIn: parent
    width: 400
    height: 300

    ColumnLayout {
      anchors.fill: parent
      spacing: 10

      ListView {
        Layout.fillWidth: true
        Layout.fillHeight: true
        clip: true

        model: ['Test Vehicle - Sedan', 'Test Vehicle - SUV', 'Test Vehicle - Truck']

        delegate: Rectangle {
          width: parent.width
          height: 50
          color: Theme.backgroundColor
          border.color: Theme.accentColor
          border.width: 1

          Text {
            anchors {
              fill: parent
              margins: 10
            }
            text: modelData
            color: Theme.textColor
            verticalAlignment: Text.AlignVCenter
          }

          MouseArea {
            anchors.fill: parent
            onClicked: {
              // TODO: Select profile
            }
          }
        }
      }

      RowLayout {
        spacing: 10

        Button {
          Layout.fillWidth: true
          text: qsTr('Add')
          onClicked: {
            // TODO: Add new vehicle profile
          }
        }

        Button {
          Layout.fillWidth: true
          text: qsTr('Edit')
          onClicked: {
            // TODO: Edit selected profile
          }
        }

        Button {
          Layout.fillWidth: true
          text: qsTr('Delete')
          onClicked: {
            // TODO: Delete selected profile
          }
        }

        Button {
          text: qsTr('Close')
          onClicked: vehicleProfilesDialog.close()
        }
      }
    }
  }
}
