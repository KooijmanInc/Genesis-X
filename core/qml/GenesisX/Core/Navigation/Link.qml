// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

import QtQuick
import QtQuick.Controls
import GenesisX.Core 1.0

Control {
    id: link

    property string to
    property var params: ({})
    contentItem: Text { text: control.text; font.underline: true }
    hoverEnabled: true
    onActiveFocusChanged: Router.navigate(to, params)
}
