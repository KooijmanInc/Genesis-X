// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

import QtQuick

Item {
    id: root

    property var tabs: []
    property int currentIndex: 0

    Loader {
        id: loader
        anchors.fill: parent
        asynchronous: true

        source: (root.tabs.length > 0 && root.currentIndex >= 0)
                ? root.tabs[root.currentIndex].qml
                : ""

        // optional: pass shared context to panels
        onLoaded: {
            // Example: If your panels expect `editorState`, you can inject it:
            // if (loader.item && loader.item.hasOwnProperty("editorState")) loader.item.editorState = root.editorState
        }
    }
}
