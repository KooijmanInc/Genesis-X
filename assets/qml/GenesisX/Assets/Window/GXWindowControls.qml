// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

import QtQuick
import QtQuick.Layouts

import ".."

RowLayout {
    id: controls

    required property Window window

    property color textColor: Style.textPrimary

    spacing: 0

    GXWindowControlButton {
        id: minimizeButton
        symbol: "qrc:/qt/qml/GenesisX/Assets/Icon/window-minimize.svg"
        brightness: window.active ? 1.0 : 0.5
        textColor: controls.textColor

        Layout.fillHeight: true

        onClicked: controls.window.showMinimized()
    }

    GXWindowControlButton {
        symbol: controls.window.visibility === Window.Maximized ? "qrc:/qt/qml/GenesisX/Assets/Icon/window-restore.svg" : "qrc:/qt/qml/GenesisX/Assets/Icon/window-maximize.svg"
        brightness: window.active ? 1.0 : 0.5
        textColor: controls.textColor

        Layout.fillHeight: true

        onClicked: {
            if (controls.window.visibility === Window.Maximized) {
                controls.window.showNormal()
            } else {
                controls.window.showMaximized()
            }
        }
    }

    GXWindowControlButton {
        symbol: "qrc:/qt/qml/GenesisX/Assets/Icon/window-close.svg"
        brightness: window.active ? 1.0 : 0.5
        textColor: controls.textColor

        closeButton: true

        Layout.fillHeight: true

        onClicked: controls.window.close()
    }
}
