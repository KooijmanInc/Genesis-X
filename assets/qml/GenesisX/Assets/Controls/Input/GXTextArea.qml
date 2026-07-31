// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects

Control {
    id: root

    property alias text: textArea.text
    property alias placeholderText: placeholder.text

    property int textAreaRadius: 18
    property int textAreaPadding: 14

    property real textAreaWidth: 260
    property real textAreaHeight: 110
    property real placeholderTextSize: 10
    property real textAreaTextSize: 12

    property color backgroundColor: "#0c2a46"
    property color borderColor: "#041326"
    property color textAreaColor: "#FFFFFF"
    property color placeholderColor: "#3ee6ff"

    implicitWidth: textAreaWidth
    implicitHeight: textAreaHeight

    background: Rectangle {
        radius: root.textAreaRadius
        color: root.backgroundColor
        border.color: root.borderColor
        border.width: 1
        opacity: 0.95

        layer.enabled: true
        layer.effect: DropShadow {
            transparentBorder: true
            horizontalOffset: 0
            verticalOffset: 0
            radius: 18
            samples: 32
            color: root.borderColor
            spread: 0.1
        }
    }

    contentItem: Item {
        anchors.fill: parent
        anchors.margins: textAreaPadding

        Flickable {
            id: flick
            anchors.fill: parent
            clip: true
            contentWidth: flick.width
            contentHeight: textArea.paintedHeight

            boundsBehavior: Flickable.StopAtBounds

            TextArea {
                id: textArea
                width: flick.width
                textFormat: TextEdit.PlainText
                wrapMode: TextArea.Wrap
                color: root.textAreaColor
                font.pixelSize: root.textAreaTextSize
                cursorVisible: activeFocus
                selectByMouse: true
                padding: 0
                background: null

                onCursorRectangleChanged: {
                    // bottom of cursor in content coords
                    var bottom = cursorRectangle.y + cursorRectangle.height

                    // scroll down if caret goes below visible area
                    if (bottom - flick.contentY > flick.height) {
                        flick.contentY = bottom - flick.height
                    }

                    // scroll up if caret goes above visible area
                    if (cursorRectangle.y < flick.contentY) {
                        flick.contentY = cursorRectangle.y
                    }
                }
            }

            Text {
                id: placeholder
                anchors.fill: parent
                font.pixelSize: root.placeholderTextSize
                color: root.placeholderColor
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignTop
                visible: textArea.text.length === 0 && !textArea.activeFocus
            }
        }
    }
}
