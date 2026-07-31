// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

import QtQuick
import QtQuick.Controls

import ".."

Popup {
    id: menuPopup

    signal closeRequested()

    default property alias menuItems: menuContent.data

    property int menuWidth: 220
    property int contentPadding: 6

    width: menuWidth
    height: backgroundItem.implicitHeight + contentPadding * 2

    padding: 0
    margins: 0

    modal: false
    focus: true

    closePolicy: Popup.CloseOnEscape
                 | Popup.CloseOnPressOutsideParent

    background: Rectangle {
        id: backgroundItem

        implicitHeight: menuContent.implicitHeight
                        + menuPopup.contentPadding * 2

        color: Style.backgroundSecondary
        border.color: Style.borderColorPrimary
        border.width: 1
        radius: Style.radiusSmall
    }

    contentItem: Column {
        id: menuContent

        x: menuPopup.contentPadding
        y: menuPopup.contentPadding

        width: menuPopup.width
               - menuPopup.contentPadding * 2

        spacing: 2

        // Menu items will be added here later.
    }
}