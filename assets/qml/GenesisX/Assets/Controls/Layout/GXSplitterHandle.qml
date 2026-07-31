// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

import QtQuick

import "../../"

Rectangle {
    id: handle

    property int orientation: Qt.Horizontal
    property real thickness: 4

    property color normalColor: Style.borderColorSecondary
    property color hoverColor: Style.borderColorHoverSecondary
    property color pressedColor: Style.borderColorPressedSecondary

    readonly property bool horizontal:
        handle.orientation === Qt.Horizontal

    signal dragStarted(point pointerPosition)
    signal dragged(point pointerPosition)
    signal dragFinished()

    // width: horizontal ? thickness : parent.width
    // height: horizontal ? parent.height : thickness
    width: 6
    height: parent.height

    color: "transparent"

    Rectangle {
        anchors {
            centerIn: parent
        }

        width: handle.thickness
        height: parent.height
        color: dragArea.pressed
            ? pressedColor
            : dragArea.containsMouse
                ? hoverColor
                : normalColor
    }

    MouseArea {
        id: dragArea

        anchors.fill: parent

        hoverEnabled: true
        acceptedButtons: Qt.LeftButton

        cursorShape: handle.horizontal
            ? Qt.SizeHorCursor
            : Qt.SizeVerCursor

        onPressed: function(mouse) {
            const point = mapToItem(
                handle.parent,
                mouse.x,
                mouse.y
            )

            handle.dragStarted(point)
        }

        onPositionChanged: function(mouse) {
            if (!pressed)
                return

            const point = mapToItem(
                handle.parent,
                mouse.x,
                mouse.y
            )

            handle.dragged(point)
        }

        onReleased: handle.dragFinished()
        onCanceled: handle.dragFinished()
    }
}
