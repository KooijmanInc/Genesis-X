import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: settingsContent

    property string ff: "hallo"

    StackView {
        id: settingsStack

        anchors.fill: parent

        initialItem: GXAppearanceBehaviorPage {
            onOpenPage: page => settingsContent.openPage(page)
        }

        pushEnter: Transition {
            NumberAnimation {
                property: "x"
                from: settingsStack.width
                to: 0
                duration: 160
                easing.type: Easing.OutCubic
            }

            NumberAnimation {
                property: "opacity"
                from: 0
                to: 1
                duration: 160
            }
        }

        pushExit: Transition {
            NumberAnimation {
                property: "x"
                from: 0
                to: -settingsStack.width * 0.15
                duration: 160
                easing.type: Easing.OutCubic
            }

            NumberAnimation {
                property: "opacity"
                from: 1
                to: 0.65
                duration: 160
            }
        }

        popEnter: Transition {
            NumberAnimation {
                property: "x"
                from: -settingsStack.width * 0.15
                to: 0
                duration: 160
                easing.type: Easing.OutCubic
            }

            NumberAnimation {
                property: "opacity"
                from: 0.65
                to: 1
                duration: 160
            }
        }

        popExit: Transition {
            NumberAnimation {
                property: "x"
                from: 0
                to: settingsStack.width
                duration: 160
                easing.type: Easing.OutCubic
            }

            NumberAnimation {
                property: "opacity"
                from: 1
                to: 0
                duration: 160
            }
        }
    }

    function openPage(page) {
        if (page !== settingsStack.currentItem.objectName) {
            switch (page) {
            case "appearance":
                settingsStack.pushItem(
                    Qt.resolvedUrl("GXAppearancePage.qml"),
                    {
                        "canGoBack": true
                    }
                )
                break
            case "appearanceBehavior":
                settingsStack.pushItem(
                    Qt.resolvedUrl("GXAppearanceBehaviorPage.qml"),
                    {
                        "canGoBack": true
                    }
                )
                break
            }
        }
    }
}