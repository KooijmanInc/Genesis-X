// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

import QtQuick

Item {
    id: root

    property var tabs: []
    property int currentIndex: 0
    signal tabActivated(int index)

    height: 44

    Rectangle {
        anchors.fill: parent
        color: "#151922"
    }

    Flickable {
        anchors.fill: parent
        contentWidth: row.implicitWidth + 20
        contentHeight: height
        clip: true

        Row {
            id: row
            spacing: 8
            anchors.verticalCenter: parent.verticalCenter
            x: 10

            Repeater {
                model: root.tabs.length

                GXTabButton {
                    text: root.tabs[index].title
                    active: index === root.currentIndex
                    onClicked: root.tabActivated(index)
                }
            }
        }
    }
}
