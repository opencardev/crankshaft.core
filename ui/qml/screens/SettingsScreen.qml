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
import "../components"
import Crankshaft 1.0

Page {
    id: root
    
    property var stack: null
    property string deepLinkCategory: ""
    
    background: Rectangle {
        color: Theme.background
    }
    
    Component.onCompleted: {
        if (deepLinkCategory !== "") {
            // Select the category
            for (var i = 0; i < SettingsModel.categories.length; i++) {
                if (SettingsModel.categories[i].id === deepLinkCategory) {
                    categoryList.currentIndex = i
                    break
                }
            }
        }
    }
    
    // Header
    Rectangle {
        id: header
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 60
        color: Theme.surface
        z: 10
        
        RowLayout {
            anchors.fill: parent
            anchors.margins: Theme.spacingMd
            spacing: Theme.spacingMd
            
            AppButton {
                text: "⬅ " + Strings.buttonBack
                Layout.preferredWidth: implicitWidth
                onClicked: {
                    if (typeof stack !== 'undefined' && stack) {
                        stack.pop()
                    }
                }
            }
            
            Text {
                text: Strings.buttonSettings
                font.pixelSize: Theme.fontSizeHeading2
                font.bold: true
                color: Theme.textPrimary
                Layout.fillWidth: true
                elide: Text.ElideRight
            }
        }
    }
    
    // VSCode-style split view
    RowLayout {
        anchors.top: header.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        spacing: 0
        
        // Left sidebar - Category list
        Rectangle {
            Layout.preferredWidth: 200
            Layout.fillHeight: true
            color: Theme.surface
            
            ListView {
                id: categoryList
                anchors.fill: parent
                anchors.margins: Theme.spacingSm
                clip: true
                spacing: 2
                
                model: SettingsModel.categories
                
                delegate: Rectangle {
                    width: ListView.view.width
                    height: 44
                    color: ListView.isCurrentItem ? Theme.primary : "transparent"
                    radius: Theme.radiusSm
                    
                    MouseArea {
                        anchors.fill: parent
                        onClicked: categoryList.currentIndex = index
                    }
                    
                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: Theme.spacingSm
                        anchors.rightMargin: Theme.spacingSm
                        spacing: Theme.spacingSm
                        
                        Text {
                            text: modelData.icon || "•"
                            font.pixelSize: 18
                            color: ListView.isCurrentItem ? Theme.textOnPrimary : Theme.textPrimary
                        }
                        
                        Text {
                            text: modelData.name
                            font.pixelSize: Theme.fontSizeBody
                            color: ListView.isCurrentItem ? Theme.textOnPrimary : Theme.textPrimary
                            Layout.fillWidth: true
                        }
                    }
                }
            }
        }
        
        // Divider
        Rectangle {
            Layout.preferredWidth: 1
            Layout.fillHeight: true
            color: Theme.divider
        }
        
        // Right content area - Settings details
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            contentWidth: availableWidth
            clip: true
            
            ColumnLayout {
                width: parent.width
                spacing: 0
                
                // Category header
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 60
                    color: "transparent"
                    
                    Text {
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.leftMargin: Theme.spacingLg
                        text: categoryList.currentIndex >= 0 ? SettingsModel.categories[categoryList.currentIndex].name : ""
                        font.pixelSize: Theme.fontSizeHeading2
                        font.bold: true
                        color: Theme.textPrimary
                    }
                }
                
                // Settings list
                Repeater {
                    model: categoryList.currentIndex >= 0 ? SettingsModel.categories[categoryList.currentIndex].settings : []
                    
                    delegate: Loader {
                        Layout.fillWidth: true
                        Layout.leftMargin: Theme.spacingLg
                        Layout.rightMargin: Theme.spacingLg
                        Layout.bottomMargin: Theme.spacingMd
                        
                        property var setting: modelData
                        
                        sourceComponent: {
                            switch(modelData.type) {
                                case "toggle": return toggleSetting
                                case "select": return selectSetting
                                case "text": return textSetting
                                case "number": return numberSetting
                                case "slider": return sliderSetting
                                case "page": return pageSetting
                                case "info": return infoSetting
                                default: return null
                            }
                        }
                    }
                }
                
                Item { Layout.fillHeight: true }
            }
        }
    }
    
    // Setting Components
    Component {
        id: toggleSetting
        
        Rectangle {
            width: parent.width
            height: contentColumn.height + Theme.spacingMd * 2
            color: Theme.surface
            radius: Theme.radiusSm
            
            ColumnLayout {
                id: contentColumn
                anchors.left: parent.left
                anchors.right: toggleSwitch.left
                anchors.verticalCenter: parent.verticalCenter
                anchors.leftMargin: Theme.spacingMd
                anchors.rightMargin: Theme.spacingSm
                spacing: 4
                
                Text {
                    text: setting.label
                    font.pixelSize: Theme.fontSizeBody
                    color: Theme.textPrimary
                    Layout.fillWidth: true
                }
                
                Text {
                    text: setting.description || ""
                    font.pixelSize: Theme.fontSizeCaption
                    color: Theme.textSecondary
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                    visible: text !== ""
                }
            }
            
            Switch {
                id: toggleSwitch
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.rightMargin: Theme.spacingMd
                checked: setting.value || false
                onToggled: {
                    if (setting.onChange) {
                        setting.onChange(checked)
                    }
                }
            }
        }
    }
    
    Component {
        id: selectSetting
        
        Rectangle {
            width: parent.width
            height: contentColumn2.height + Theme.spacingMd * 2
            color: Theme.surface
            radius: Theme.radiusSm
            
            ColumnLayout {
                id: contentColumn2
                anchors.left: parent.left
                anchors.right: selectBox.left
                anchors.verticalCenter: parent.verticalCenter
                anchors.leftMargin: Theme.spacingMd
                anchors.rightMargin: Theme.spacingSm
                spacing: 4
                
                Text {
                    text: setting.label
                    font.pixelSize: Theme.fontSizeBody
                    color: Theme.textPrimary
                    Layout.fillWidth: true
                }
                
                Text {
                    text: setting.description || ""
                    font.pixelSize: Theme.fontSizeCaption
                    color: Theme.textSecondary
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                    visible: text !== ""
                }
            }
            
            ComboBox {
                id: selectBox
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.rightMargin: Theme.spacingMd
                width: 200
                
                model: setting.options || []
                textRole: "label"
                valueRole: "value"
                
                Component.onCompleted: {
                    for (var i = 0; i < model.length; i++) {
                        if (model[i].value === setting.value) {
                            currentIndex = i
                            break
                        }
                    }
                }
                
                onActivated: (index) => {
                    if (setting.onChange && model[index]) {
                        setting.onChange(model[index].value)
                    }
                }
            }
        }
    }
    
    Component {
        id: textSetting
        
        Rectangle {
            width: parent.width
            height: contentColumn3.height + Theme.spacingMd * 2
            color: Theme.surface
            radius: Theme.radiusSm
            
            ColumnLayout {
                id: contentColumn3
                anchors.left: parent.left
                anchors.right: textField.left
                anchors.verticalCenter: parent.verticalCenter
                anchors.leftMargin: Theme.spacingMd
                anchors.rightMargin: Theme.spacingSm
                spacing: 4
                
                Text {
                    text: setting.label
                    font.pixelSize: Theme.fontSizeBody
                    color: Theme.textPrimary
                    Layout.fillWidth: true
                }
                
                Text {
                    text: setting.description || ""
                    font.pixelSize: Theme.fontSizeCaption
                    color: Theme.textSecondary
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                    visible: text !== ""
                }
            }
            
            TextField {
                id: textField
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.rightMargin: Theme.spacingMd
                width: 200
                text: setting.value || ""
                placeholderText: setting.placeholder || ""
                onEditingFinished: {
                    if (setting.onChange) {
                        setting.onChange(text)
                    }
                }
            }
        }
    }
    
    Component {
        id: numberSetting
        
        Rectangle {
            width: parent.width
            height: contentColumn4.height + Theme.spacingMd * 2
            color: Theme.surface
            radius: Theme.radiusSm
            
            ColumnLayout {
                id: contentColumn4
                anchors.left: parent.left
                anchors.right: spinBox.left
                anchors.verticalCenter: parent.verticalCenter
                anchors.leftMargin: Theme.spacingMd
                anchors.rightMargin: Theme.spacingSm
                spacing: 4
                
                Text {
                    text: setting.label
                    font.pixelSize: Theme.fontSizeBody
                    color: Theme.textPrimary
                    Layout.fillWidth: true
                }
                
                Text {
                    text: setting.description || ""
                    font.pixelSize: Theme.fontSizeCaption
                    color: Theme.textSecondary
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                    visible: text !== ""
                }
            }
            
            SpinBox {
                id: spinBox
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.rightMargin: Theme.spacingMd
                from: setting.min || 0
                to: setting.max || 100
                value: setting.value || 0
                onValueModified: {
                    if (setting.onChange) {
                        setting.onChange(value)
                    }
                }
            }
        }
    }
    
    Component {
        id: sliderSetting
        
        Rectangle {
            width: parent.width
            height: sliderColumn.height + Theme.spacingMd * 2
            color: Theme.surface
            radius: Theme.radiusSm
            
            ColumnLayout {
                id: sliderColumn
                anchors.fill: parent
                anchors.margins: Theme.spacingMd
                spacing: Theme.spacingSm
                
                RowLayout {
                    Layout.fillWidth: true
                    
                    Text {
                        text: setting.label
                        font.pixelSize: Theme.fontSizeBody
                        color: Theme.textPrimary
                        Layout.fillWidth: true
                    }
                    
                    Text {
                        text: slider.value
                        font.pixelSize: Theme.fontSizeBody
                        font.bold: true
                        color: Theme.primary
                    }
                }
                
                Slider {
                    id: slider
                    Layout.fillWidth: true
                    from: setting.min || 0
                    to: setting.max || 100
                    stepSize: setting.step || 1
                    value: setting.value || 0
                    onMoved: {
                        if (setting.onChange) {
                            setting.onChange(value)
                        }
                    }
                }
                
                Text {
                    text: setting.description || ""
                    font.pixelSize: Theme.fontSizeCaption
                    color: Theme.textSecondary
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                    visible: text !== ""
                }
            }
        }
    }
    
    Component {
        id: pageSetting
        
        Rectangle {
            width: parent.width
            height: contentColumn5.height + Theme.spacingMd * 2
            color: Theme.surface
            radius: Theme.radiusSm
            
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    if (typeof stack === 'undefined' || !stack) return
                    
                    if (setting.pageComponent === "AndroidAutoSettingsPage") {
                        stack.push(androidautoSettingsScreen)
                    } else if (setting.pageComponent === "WiFiSettingsPage") {
                        stack.push(wifiSettingsPage)
                    } else if (setting.pageComponent === "BluetoothSettingsPage") {
                        stack.push(bluetoothSettingsPage)
                    }
                }
            }
            
            ColumnLayout {
                id: contentColumn5
                anchors.left: parent.left
                anchors.right: arrow.left
                anchors.verticalCenter: parent.verticalCenter
                anchors.leftMargin: Theme.spacingMd
                anchors.rightMargin: Theme.spacingSm
                spacing: 4
                
                Text {
                    text: setting.label
                    font.pixelSize: Theme.fontSizeBody
                    color: Theme.textPrimary
                    Layout.fillWidth: true
                }
                
                Text {
                    text: setting.description || ""
                    font.pixelSize: Theme.fontSizeCaption
                    color: Theme.textSecondary
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                    visible: text !== ""
                }
            }
            
            Text {
                id: arrow
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.rightMargin: Theme.spacingMd
                text: "›"
                font.pixelSize: 24
                color: Theme.textSecondary
            }
        }
    }
    
    Component {
        id: infoSetting
        
        Rectangle {
            width: parent.width
            height: contentColumn6.height + Theme.spacingMd * 2
            color: Theme.surface
            radius: Theme.radiusSm
            
            ColumnLayout {
                id: contentColumn6
                anchors.left: parent.left
                anchors.right: valueText.left
                anchors.verticalCenter: parent.verticalCenter
                anchors.leftMargin: Theme.spacingMd
                anchors.rightMargin: Theme.spacingSm
                spacing: 4
                
                Text {
                    text: setting.label
                    font.pixelSize: Theme.fontSizeBody
                    color: Theme.textPrimary
                    Layout.fillWidth: true
                }
                
                Text {
                    text: setting.description || ""
                    font.pixelSize: Theme.fontSizeCaption
                    color: Theme.textSecondary
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                    visible: text !== ""
                }
            }
            
            Text {
                id: valueText
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.rightMargin: Theme.spacingMd
                text: setting.value || ""
                font.pixelSize: Theme.fontSizeBody
                color: Theme.textSecondary
            }
        }
    }
    
    // Page components for navigation
    Component {
        id: androidautoSettingsScreen
        AndroidAutoSettingsPage {}
    }
    
    Component {
        id: wifiSettingsPage
        WiFiSettingsPage {}
    }
    
    Component {
        id: bluetoothSettingsPage
        BluetoothSettingsPage {}
    }
}
