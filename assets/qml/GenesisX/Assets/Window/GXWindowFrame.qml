// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

import QtQuick
import QtQuick.Window
import QtQuick.Effects

import ".."

Item {
    id: windowFrame

    required property Window window

    property string title: "title"
    property string icon

    property int headerHeight: 34
    property int resizeMargin: 6
    property int frameRadius: 8
    property int frameMargin: windowFrame.maximized ? 0 : 1

    property int shadowMargin: windowFrame.maximized ? 0 : 12

    property int sideBarWidth: 34

    property int leftBarWidth: windowFrame.sideBarWidth
    property int rightBarWidth: windowFrame.sideBarWidth

    property int statusBarHeight: 24

    property color backgroundColor: Style.backgroundPrimary
    property color chromeColor: Style.chromeColorPrimary

    property color headerBackgroundColor: chromeColor
    property color leftBarBackgroundColor: chromeColor
    property color rightBarBackgroundColor: chromeColor
    property color statusBarBackgroundColor: chromeColor

    property color borderColor: Style.borderColorPrimary
    property int borderWidth: 1
    property int headerBorderWidth: 1

    property bool useToolbar: true

    readonly property bool maximized: windowFrame.window.visibility === Window.Maximized

    property bool darkMode: true

    anchors.fill: parent

    property Component header: Component {
        GXWindowHeader {
            id: windowHeader
            window: windowFrame.window
            title: windowFrame.title
            icon: windowFrame.icon
            borderRadius: windowFrame.maximized ? 0 : windowFrame.frameRadius
            backgroundColor: windowFrame.headerBackgroundColor
            borderColor: windowFrame.borderColor
            borderWidth: windowFrame.headerBorderWidth
            textColor: Style.textPrimary
            useToolbar: windowFrame.useToolbar
            headerToolbar: windowFrame.headerToolbar
        }
    }
    property Component headerToolbar

    property Component content
    property Component leftBar: Component {
        GXWindowLeftBar {
            id: leftBar
            window: windowFrame.window
            backgroundColor: windowFrame.leftBarBackgroundColor
        }
    }

    property Component rightBar: Component {
        GXWindowRightBar {
            id: rightBar
            window: windowFrame.window
            backgroundColor: windowFrame.rightBarBackgroundColor
        }
    }
    property Component statusBar: Component {
        GXWindowStatusBar {
            id: statusBar
            window: windowFrame.window
            backgroundColor: windowFrame.statusBarBackgroundColor
        }
    }

    MultiEffect {
        anchors.fill: frame

        source: frame

        shadowEnabled: true

        shadowBlur: 0.8
        shadowColor: Style.shadowColorPrimary

        shadowVerticalOffset: 3

        visible: !windowFrame.maximized
    }

    Rectangle {
        id: frame

        anchors.fill: parent
        anchors.margins: shadowMargin

        color: windowFrame.backgroundColor
        radius: windowFrame.maximized ? 0 : windowFrame.frameRadius

        border.width: windowFrame.maximized ? 0 : windowFrame.borderWidth
        border.color: windowFrame.borderColor

        clip: true

        Loader {
            id: headerLoader

            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
                margins: windowFrame.frameMargin
            }

            height: windowFrame.headerHeight

            sourceComponent: windowFrame.header
        }

        Row {
            id: body

            anchors {
                top: headerLoader.bottom
                left: parent.left
                right: parent.right
                bottom: statusBarLoader.top
                margins: windowFrame.frameMargin
                bottomMargin: 0
            }

            Loader {
                id: leftBarLoader

                width: windowFrame.leftBarWidth
                height: body.height

                sourceComponent: windowFrame.leftBar
            }

            Loader {
                id: contentLoader

                width: parent.width - leftBarLoader.width - rightBarLoader.width
                height: body.height

                sourceComponent: windowFrame.content
            }

            Loader {
                id: rightBarLoader

                width: windowFrame.rightBarWidth
                height: body.height

                sourceComponent: windowFrame.rightBar
            }
        }

        Loader {
            id: statusBarLoader

            anchors {
                left: parent.left
                right: parent.right
                bottom: parent.bottom
                margins: windowFrame.frameMargin
                bottomMargin: 2
            }

            height: windowFrame.statusBarHeight

            sourceComponent: windowFrame.statusBar
        }

    }

}
