// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

import QtQuick

import ".."

Rectangle {
    id: control

    required property string symbol

    property bool closeButton: false
    property color foregroundColor: "#e6e6e6"
    property color hoverColor: Style.buttonFrameHoverPrimary
    property color pressedColor: "#484c52"
    property color closeHoverColor: "#c42b1c"
    property color closePressedColor: "#a82014"
    property color textColor: Style.textPrimary

    property real brightness: 1.0

    property int verticalOffset: 0

    signal clicked()

    implicitWidth: 46
    implicitHeight: 26

    color: {
        if (mouseArea.pressed) {
            return closeButton ? closePressedColor : pressedColor
        }

        if (mouseArea.containsMouse) {
            return closeButton ? closeHoverColor : hoverColor
        }

        return "transparent"
    }

    GXIcon {
        source: symbol
        brightness: control.brightness

        iconColor: control.textColor

        anchors.centerIn: parent
    }

    MouseArea {
        id: mouseArea

        anchors.fill: parent

        hoverEnabled: true
        acceptedButtons: Qt.LeftButton

        onClicked: {control.clicked()
        }
    }
}
