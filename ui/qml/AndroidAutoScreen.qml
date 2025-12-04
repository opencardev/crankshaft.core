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
import "Components"
import Crankshaft 1.0

Page {
  id: root
  property var stack: null
  Component.onCompleted: console.log("AndroidAutoScreen loaded")

  background: Rectangle { color: Theme.background }

  header: Rectangle {
    width: parent.width
    height: 80
    color: Theme.surface
    RowLayout {
      anchors.fill: parent
      anchors.margins: Theme.spacingMd
      AppButton {
        text: "\u2190 " + Strings.buttonBack
        onClicked: {
          if (stack) {
            stack.pop()
          } else if (StackView.view) {
            StackView.view.pop()
          } else {
            console.warn("AndroidAutoScreen: No stack available to pop")
          }
        }
      }
      Text {
        text: Strings.aaTitle
        font.pixelSize: Theme.fontSizeHeading2
        font.bold: true
        color: Theme.textPrimary
        Layout.fillWidth: true
      }
    }
  }

  ScrollView {
    anchors.fill: parent
    contentWidth: availableWidth
    ColumnLayout {
      width: parent.width
      spacing: Theme.spacingMd
      Rectangle {
        Layout.fillWidth: true
        Layout.margins: Theme.spacingMd
        height: contentCol.height + Theme.spacingLg * 2
        color: Theme.surface
        radius: Theme.radiusMd
        ColumnLayout {
          id: contentCol
          anchors.centerIn: parent
          width: parent.width - Theme.spacingLg * 2
          spacing: Theme.spacingMd
          Text {
            text: Strings.labelStatus
            font.pixelSize: Theme.fontSizeHeading3
            font.bold: true
            color: Theme.textPrimary
          }
          Text {
            text: Strings.statusNotConnected
            font.pixelSize: Theme.fontSizeBody
            color: Theme.textSecondary
          }
          RowLayout {
            Layout.fillWidth: true
            spacing: Theme.spacingMd
            AppButton {
              text: Strings.aaStart
              onClicked: {
                // Example: Publish an event to core to start AA
                  console.log("AndroidAutoScreen: start requested")
                wsClient.publish("androidauto/start", { })
              }
            }
            AppButton {
              text: Strings.aaStop
              onClicked: {
                  console.log("AndroidAutoScreen: stop requested")
                wsClient.publish("androidauto/stop", { })
              }
            }
          }
        }
      }
    }
  }
}