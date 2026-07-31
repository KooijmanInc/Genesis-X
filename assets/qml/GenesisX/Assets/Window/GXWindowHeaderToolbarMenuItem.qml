// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

import QtQuick

import ".."

Item {
    id: menuItem

    signal clicked()

    property string text: ""
    property bool itemEnabled: true

    readonly property bool hovered: mouseArea.containsMouse
    readonly property bool pressed: mouseArea.pressed

    implicitWidth: 220
    implicitHeight: 30

    Rectangle {
        anchors.fill: parent

        radius: Style.radiusSmall

        color: {
            if (!menuItem.enabled)
                return "transparent"

            if (menuItem.pressed)
                return Style.textPrimary

            if (menuItem.hovered)
                return Style.textSecondary

            return "transparent"
        }
    }

    Text {
        anchors.left: parent.left
        anchors.leftMargin: Style.spacingSmall
        anchors.verticalCenter: parent.verticalCenter

        text: menuItem.text

        color: menuItem.itemEnabled
               ? Style.textPrimary
               : Style.textDisabled
    }

    MouseArea {
        id: mouseArea

        anchors.fill: parent

        enabled: menuItem.itemEnabled
        hoverEnabled: true

        onClicked: menuItem.clicked()
    }
}