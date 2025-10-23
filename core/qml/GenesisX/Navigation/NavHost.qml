import QtQuick
import QtQuick.Controls
import GenesisX.Core 1.0

Item {
    id: root

    property alias router: Router
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
