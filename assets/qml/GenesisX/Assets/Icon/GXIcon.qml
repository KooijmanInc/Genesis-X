// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

import QtQuick
import QtQuick.Effects

Item {
    id: icon

    property url source

    property color iconColor: "#ffffff"

    property int size: 16

    property real iconRotation: 0
    property real iconOpacity: 1.0
    property real brightness: 1.0

    implicitWidth: icon.size
    implicitHeight: icon.size

    Image {
        id: image

        anchors.fill: parent

        source: icon.source

        sourceSize.width: icon.size
        sourceSize.height: icon.size

        fillMode: Image.PreserveAspectFit
        smooth: true
        mipmap: true

        visible: false

        rotation: icon.iconRotation
    }

    ShaderEffectSource {
        id: effectSource

        anchors.fill: image
        sourceItem: image
        hideSource: true
        live: true
    }

    MultiEffect {
        anchors.fill: image

        source: effectSource

        colorization: 1.0
        colorizationColor: icon.iconColor

        brightness: icon.brightness

        Behavior on brightness {
            NumberAnimation {
                duration: 120
                easing.type: Easing.OutQuad
            }
        }

        opacity: icon.iconOpacity
    }
}
