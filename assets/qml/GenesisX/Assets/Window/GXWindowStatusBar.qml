// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

import QtQuick

Rectangle {
    id: statusBar

    required property Window window

    property color backgroundColor: "#26282c"
    property int borderRadius: 8

    implicitHeight: 22

    color: backgroundColor

    bottomLeftRadius: statusBar.borderRadius
    bottomRightRadius: statusBar.borderRadius

    clip: true
}
