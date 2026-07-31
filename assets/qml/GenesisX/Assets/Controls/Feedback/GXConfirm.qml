// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

import QtQuick
import Qt5Compat.GraphicalEffects

Item {
    id: root

    property alias backgroundColor: confirmBox.color
    property alias radius: confirmBox.radius
    property alias confirmBoxWidth: confirmBox.width
    property alias borderWidth: confirmBox.border.width
    property alias titleSize: title.font.pixelSize
    property alias title: title.text

    property bool flipGradient: false
    property bool useGradient: true
    property bool isConfirmed: false
    property bool isCanceled: false

    property real borderOpacity: 0.25

    property color backgroundTopColor: "#1a4f76"
    property color backgroundBottomColor: "#041326"
    property color borderColor: "#041326"
    property color titleColor: "#FFFFFF"

    Rectangle {
        id: confirmBox
        anchors.centerIn: parent
        width: (parent.width > 500) ? 300 : parent.width * 0.8
        height: title.contentHeight + buttonBox.height + 40
        radius: 10
        color: "#0c2a46"
        border.color: Qt.rgba(
            Qt.rgba(borderColor.r, borderColor.g, borderColor.b).r,
            Qt.rgba(borderColor.r, borderColor.g, borderColor.b).g,
            Qt.rgba(borderColor.r, borderColor.g, borderColor.b).b,
            borderOpacity
        )
        border.width: 1

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

        Text {
            id: title

            anchors {
                top: parent.top
                topMargin: 10
                bottomMargin: 10
            }

            width: parent.width
            height: contentHeight
            wrapMode: Text.WordWrap
            color: root.titleColor
            font.pixelSize: 20
            font.bold: true
            horizontalAlignment: Text.AlignHCenter

            text: "Confirm"
        }

        Rectangle {
            id: buttonBox
            anchors {
                bottom: parent.bottom
                left: parent.left
                right: parent.right
                margins: 10
            }
            height: ok.height
            color: "transparent"

            GXButton {
                id: ok
                anchors {
                    top: parent.top
                    right: cancel.left
                    rightMargin: 20
                }
                useGradient: false
                borderShadow: false
                backgroundColor: "#FFFFFF"
                textColor: "#000000"

                text: "Ok"
            }

            MouseArea {
                anchors.fill: ok
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    isConfirmed = true
                }
            }

            GXButton {
                id: cancel
                anchors {
                    top: parent.top
                    right: parent.right
                }
                useGradient: false
                borderShadow: false
                backgroundColor: "#FFFFFF"
                textColor: "#000000"

                text: "Cancel"
            }

            MouseArea {
                anchors.fill: cancel
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    isCanceled = true
                }
            }
        }
    }
}
