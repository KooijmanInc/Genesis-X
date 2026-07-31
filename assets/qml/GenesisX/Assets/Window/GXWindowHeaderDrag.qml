// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

import QtQuick
import QtQuick.Layouts

Item {
    id: dragArea

    required property Window window

    Layout.fillWidth: true
    Layout.fillHeight: true

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton

        onPressed: mouse => {
            dragArea.window.startSystemMove()
            mouse.accepted = true
        }

        onDoubleClicked: {
            if (dragArea.window.visibility === Window.Maximized) {
                dragArea.window.showNormal()
            } else {
                dragArea.window.showMaximized()
            }
        }
    }
}
