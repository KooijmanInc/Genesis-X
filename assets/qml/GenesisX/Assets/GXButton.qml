// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects

Control {
    id: root

    property alias text: label.text
    property alias textFont: label.font.family
    property alias textSize: label.font.pixelSize

    property int radius : height / 2

    property color textColor: "#8AEFFF"
    property color backgroundColor: "#1a4f76"
    property color backgroundTopColor: "#1a4f76"
    property color backgroundBottomColor: "#041326"
    property color borderColor: "#041326"

    property bool bold: false
    property bool pressed: false
    property bool useGradient: true
    property bool borderShadow: true

    property real borderOpacity: 0.25

    implicitWidth: label.implicitWidth + 40
    implicitHeight: 50

    contentItem: Text {
        id: label
        font.pixelSize: 16
        font.bold: bold
        anchors.fill: parent
        horizontalAlignment: Qt.AlignHCenter
        verticalAlignment: Qt.AlignVCenter
        color: root.textColor
        text: "Button"
    }

    background: Rectangle {
        anchors.fill: parent
        radius: root.radius
        color: root.backgroundColor

        gradient: Gradient {
            GradientStop {
                position: -0.65
                color: useGradient === true ? root.backgroundTopColor : root.backgroundColor
            }
            GradientStop {
                position: 0.95
                color: useGradient === true ? root.backgroundBottomColor : root.backgroundColor
            }
        }

        layer.enabled: borderShadow
        layer.effect: DropShadow {
            transparentBorder: true
            horizontalOffset: 0
            verticalOffset: 0
            radius: 18
            samples: 32
            color: root.borderColor
            spread: 0.1
        }

        border.color: Qt.rgba(
            Qt.rgba(borderColor.r, borderColor.g, borderColor.b).r,
            Qt.rgba(borderColor.r, borderColor.g, borderColor.b).g,
            Qt.rgba(borderColor.r, borderColor.g, borderColor.b).b,
            borderOpacity
        )
        border.width: 1
    }

    states: [
        State {
            name: "pressed"
            when: root.pressed
            PropertyChanges { target: root; scale: 0.97 }
            PropertyChanges { target: label; opacity: 0.7 }
        }
    ]

    transitions: Transition {
        NumberAnimation { properties: "scale,opacity"; duration: 120; easing.type: Easing.OutCubic }
    }
}
