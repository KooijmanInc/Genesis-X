// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

import QtQuick

import ".."

Window {
    id: settingsWindow
    width: 1024
    height: 768

    flags: Qt.Window | Qt.FramelessWindowHint

    color: "transparent"

    title: qsTr("genesisx.settings.windowTitle")

    Component.onCompleted: {
        settingsWindow.close()
    }

    function open() {
        show()
        raise()
        requestActivate()
    }

    property string icon

    property bool darkMode: true

    property color backgroundColor: Style.backgroundPrimary

    property int headerBorderWidth: 1

    property int sideBarWidth: 34

    property int leftBarWidth: 0
    property int rightBarWidth: 0

    property int splitterBorderWidth: 4

    GXWindowFrame {
        window: settingsWindow

        title: settingsWindow.title
        icon: settingsWindow.icon

        backgroundColor: settingsWindow.backgroundColor
        darkMode: settingsWindow.darkMode
        headerBorderWidth: settingsWindow.headerBorderWidth
        leftBarWidth: settingsWindow.leftBarWidth !== 0 ? settingsWindow.leftBarWidth : settingsWindow.sideBarWidth
        rightBarWidth: settingsWindow.rightBarWidth !== 0 ? settingsWindow.rightBarWidth : settingsWindow.sideBarWidth
        statusBarHeight: 0

        useToolbar: false

        content: Component {
            Rectangle {
                id: box
                anchors.fill: parent
                color: "transparent"
                signal settingsPageRequested(string page)
                GXSplitter {
                    anchors {
                        top: parent.top
                        right: parent.right
                        bottom: buttons.bottom
                        left: parent.left
                    }

                    orientation: Qt.Horizontal

                    leadingSize: 220
                    minimumLeadingSize: 160
                    minimumTrailingSize: 300

                    handleThickness: settingsWindow.splitterBorderWidth

                    leading: Component {
                        GXSettingsNavigation {
                            id: settingsNavigation
                            onPageSelected: page => {
                                box.settingsPageRequested(page)
                            }
                        }
                    }

                    trailing: Component {
                        GXSettingsContent {
                            Connections {
                                target: box

                                function onSettingsPageRequested(page) {
                                    openPage(page)
                                }
                            }
                        }
                    }
                }
                Rectangle {
                    id: buttons
                    anchors {
                        left: parent.left
                        right: parent.right
                        bottom: parent.bottom
                    }
                    height: 50
                    border.color: "#4b4d51"
                    border.width: 1

                    color: "transparent"
                    Rectangle {
                        anchors {
                            bottom: parent.bottom
                            right: parent.right
                            left: parent.left
                        }
                        height: 49

                        color: settingsWindow.backgroundColor
                        bottomLeftRadius: 8
                        bottomRightRadius: 8
                    }
                }
            }
        }
    }
}
