// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

import QtQuick
import QtQuick.Controls
import GenesisX.Core.Navigation 1.0

Item {
    id: root

    property var router: Router
    signal viewChanged(url component, var params)

    Loader {
        id: loader
        anchors.fill: parent
        source: Router.currentComponent
        active: Router.currentComponent.toString().length > 0
        onLoaded: {
            if (item && Router.currentParams) {
                for (var k in Router.currentParams) {
                    if (item.hasOwnProperty(k)) item[k] = Router.currentParams[k]
                }
            }
            root.viewChanged(source, Router.currentParams)
        }
    }

    Connections {
        target: Router

        function onCurrentChanged() {
            loader.source = Router.currentComponent
        }
    }
}
