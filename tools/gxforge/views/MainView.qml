// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

import QtQuick
import QtQuick.Controls

Window {
    id: mainView
    visible: true
    width: 1024
    height: 768

    title: qsTr("Genesis-X 3D model converter")

    StackView {
        id: contentView
        anchors.fill: parent
        initialItem: "qrc:/views/TabView.qml"
    }
}
