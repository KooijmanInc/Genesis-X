// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects

Control {
    id: root

    property alias title: titleLabel.text
    property alias titleOpacity: titleLabel.opacity

    property int cardPadding: 20

    property real cardWidth: 320
    property real cardHeight: 0
    property real cardRadius: 24
    property real borderWidth: 1
    property real titleLabelSize: 20
    property real borderOpacity: 0.25

    property bool useTitle: true
    property bool titleLabelBold: true
    property bool useGradient: true
    property bool flipGradient: false
    property bool contentFills: false

    property color titleLabelColor: "#FFFFFF"
    property color backgroundColor: "#0c2a46"
    property color backgroundTopColor: "#1a4f76"
    property color backgroundBottomColor: "#041326"
    property color borderColor: "#041326"

    default property alias content: slot.data

    implicitWidth: root.cardWidth
    implicitHeight: cardHeight !== 0 ? cardHeight : (titleLabel.implicitHeight + cardPadding + slot.implicitHeight + (cardPadding * 2) + (useTitle === false ? -20 : 0))

    onUseTitleChanged: {
        titleLabel.height = 0
        // root.implicitHeight = root.implicitHeight - 10
    }

    background: Rectangle {
        id: cardBackground
        radius: root.cardRadius
        color: root.backgroundColor
        border.color: Qt.rgba(
            Qt.rgba(borderColor.r, borderColor.g, borderColor.b).r,
            Qt.rgba(borderColor.r, borderColor.g, borderColor.b).g,
            Qt.rgba(borderColor.r, borderColor.g, borderColor.b).b,
            borderOpacity
        )
        border.width: root.borderWidth

        gradient: Gradient {
            GradientStop {
                position: flipGradient === true ? 0 : -0.65
                color: useGradient === true ? (flipGradient === true ? root.backgroundBottomColor : root.backgroundTopColor) : root.backgroundColor
            }
            GradientStop {
                position: flipGradient === true ? 1.65 : 0.95
                color: useGradient === true ? (flipGradient === true ? root.backgroundTopColor : root.backgroundBottomColor) : root.backgroundColor
            }
        }

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
        anchors.margins: root.cardPadding

        Text {
            id: titleLabel
            visible: useTitle
            text: "Card Title"
            font.pixelSize: root.titleLabelSize
            font.bold: root.titleLabelBold
            color: root.titleLabelColor
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.right: parent.right
            width: parent.width
            wrapMode: Text.WordWrap
            horizontalAlignment: Qt.AlignHCenter
        }

        function calcHeight() {
            var h = 0
            for (var i = 0; i < slot.children.length; i++) {
                var c = slot.children[i]
                if (c.visible) {
                    // if child has implicitHeight, use it
                    if (c.implicitHeight > 0)
                        h += c.implicitHeight
                    else if (c.height > 0)
                        h += c.height

                    // add spacing if using a Column inside
                    if (i < slot.children.length - 1)
                        h += 8
                }
            }
            return h
        }

        Item {
            id: slot
            anchors.topMargin: 8
            anchors.top: titleLabel.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: contentFills ? parent.bottom : undefined
            implicitHeight: contentFills ? calcHeight() : childrenRect.height
            // implicitHeight: {
            //     var h = 0
            //     for (var i = 0; i < slot.children.length; i++) {
            //         var c = slot.children[i]
            //         if (c.visible) {
            //             // if child has implicitHeight, use it
            //             if (c.implicitHeight > 0)
            //                 h += c.implicitHeight
            //             else if (c.height > 0)
            //                 h += c.height

            //             // add spacing if using a Column inside
            //             if (i < slot.children.length - 1)
            //                 h += 8
            //         }
            //     }
            //     return h
            // }
        }
    }
}
