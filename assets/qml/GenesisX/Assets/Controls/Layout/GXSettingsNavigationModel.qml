import QtQuick

ListModel {
    id: navigationModel

    ListElement {
        key: "appearanceBehavior"
        label: qsTr("genesisx.settings.navigation.appearanceBehavior")
        depth: 0
        expandable: true
        expanded: false
        visibleItem: true
        page: "appearanceBehavior"
    }

    ListElement {
        key: "appearance"
        label: qsTr("genesisx.settings.navigation.appearance")
        depth: 1
        expandable: false
        expanded: false
        visibleItem: false
        page: "appearance"
    }
}
