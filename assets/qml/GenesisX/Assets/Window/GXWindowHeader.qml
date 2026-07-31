// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

import QtQuick
import QtQuick.Layouts

import ".."

Rectangle {
    id: windowHeader

    required property Window window

    property string title: "title"
    property string icon

    property color backgroundColor: Style.chromeColor
    property color textColor: Style.textPrimary
    property color borderColor: Style.borderColorPrimary
    property int borderWidth: 1
    property int borderRadius: Style.radiusLarge

    property bool useToolbar: true

    property Component headerToolbar

    implicitHeight: 32

    color: backgroundColor

    topLeftRadius: windowHeader.borderRadius
    topRightRadius: windowHeader.borderRadius

    clip: true

    onBorderWidthChanged: {
        if (borderWidth === 1) {
            headerBorder.visible = true
        } else {
            headerBorder.visible = false
        }
    }

    RowLayout {
        id: layout

        anchors.fill: parent
        anchors.leftMargin: 8
        anchors.rightMargin: 8

        spacing: 0

        GXWindowIdentity {
            id: leftArea
            title: windowHeader.title

            iconSource: windowHeader.icon
            textColor: windowHeader.textColor

            Layout.preferredWidth: implicitWidth
            Layout.fillHeight: true
        }

        Loader {
            id: toolbarLoader

            active: windowHeader.useToolbar && windowHeader.headerToolbar !== null

            visible: active

            Layout.preferredWidth: item ? item.implicitWidth : 0
            Layout.preferredHeight: item ? item.implicitHeight : 0
            Layout.fillHeight: true

            sourceComponent: windowHeader.headerToolbar
        }

        GXWindowHeaderDrag {
            window: windowHeader.window

            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        GXWindowControls {
            id: controlArea

            window: windowHeader.window

            textColor: windowHeader.textColor

            Layout.preferredWidth: implicitWidth
            Layout.fillHeight: true
        }
    }

    Rectangle {
        id: headerBorder
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }

        height: windowHeader.borderWidth
        color: windowHeader.borderColor
        visible: windowHeader.borderWidth > 0
    }
}
