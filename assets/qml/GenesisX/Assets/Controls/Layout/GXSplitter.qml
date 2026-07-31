// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

import QtQuick

import "../.."

Item {
    id: splitter

    property int orientation: Qt.Horizontal

    property Component leading
    property Component trailing

    property real leadingSize: 220
    property real minimumLeadingSize: 100
    property real minimumTrailingSize: 100

    property real handleThickness: 4
    property color handleColor: Style.borderColorSecondary
    property color handleHoverColor: Style.borderColorHoverSecondary
    property color handlePressedColor: Style.borderColorPressedSecondary

    readonly property bool horizontal:
        splitter.orientation === Qt.Horizontal

    readonly property real availableSize:
        horizontal ? splitter.width : splitter.height

    readonly property real maximumLeadingSize: Math.max(
        minimumLeadingSize,
        availableSize - handleThickness - minimumTrailingSize
    )

    function clampLeadingSize(size) {
        return Math.max(
            minimumLeadingSize,
            Math.min(size, maximumLeadingSize)
        )
    }

    function setLeadingSize(size) {
        leadingSize = clampLeadingSize(size)
    }

    onWidthChanged: {
        if (horizontal)
            setLeadingSize(leadingSize)
    }

    onHeightChanged: {
        if (!horizontal)
            setLeadingSize(leadingSize)
    }

    Loader {
        id: leadingLoader

        anchors {
            top: parent.top
            left: parent.left
        }

        width: splitter.horizontal
            ? splitter.clampLeadingSize(splitter.leadingSize)
            : parent.width

        height: splitter.horizontal
            ? parent.height
            : splitter.clampLeadingSize(splitter.leadingSize)

        sourceComponent: splitter.leading
    }

    GXSplitterHandle {
        id: splitterHandle

        orientation: splitter.orientation
        thickness: splitter.handleThickness

        normalColor: splitter.handleColor
        hoverColor: splitter.handleHoverColor
        pressedColor: splitter.handlePressedColor

        anchors {
            top: splitter.horizontal
                ? parent.top
                : leadingLoader.bottom

            bottom: splitter.horizontal
                ? parent.bottom
                : undefined

            left: splitter.horizontal
                ? leadingLoader.right
                : parent.left

            right: splitter.horizontal
                ? undefined
                : parent.right
        }

        onDragStarted: function(pointerPosition) {
            dragStartPosition = splitter.horizontal
                ? pointerPosition.x
                : pointerPosition.y

            dragStartLeadingSize =
                splitter.clampLeadingSize(splitter.leadingSize)
        }

        onDragged: function(pointerPosition) {
            const currentPosition = splitter.horizontal
                ? pointerPosition.x
                : pointerPosition.y

            splitter.setLeadingSize(
                dragStartLeadingSize +
                currentPosition -
                dragStartPosition
            )
        }

        property real dragStartPosition: 0
        property real dragStartLeadingSize: 0
    }

    Loader {
        id: trailingLoader

        anchors {
            top: splitter.horizontal
                ? parent.top
                : splitterHandle.bottom

            left: splitter.horizontal
                ? splitterHandle.right
                : parent.left

            right: parent.right
            bottom: parent.bottom
        }

        sourceComponent: splitter.trailing
    }
}
