// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import ".."

Item {
    id: toolbar

    required property Window window
    property var menus: []
    property int activeMenuIndex: -1
    readonly property bool childMenuOpen: activeMenuIndex >= 0

    implicitWidth: contentRow.implicitWidth + 10
    implicitHeight: 34

    RowLayout {
        id: contentRow

        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.verticalCenter: parent.verticalCenter

        spacing: Style.spacingXSmall

        Text {
            id: hamburger
            Layout.preferredWidth: implicitWidth
            Layout.preferredHeight: toolbar.height

            font.family: Style.fontAwesome
            font.pixelSize: 20

            text: "\uf0c9"
            color: Style.textSecondary

            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignLeft

            elide: Text.ElideRight
            clip: true

            MouseArea {
                id: hamburgerHover
                anchors.fill: parent
                hoverEnabled: visible
                acceptedButtons: Qt.LeftButton
                onClicked: {
                    hamburger.visible = false
                    const position = hamburger.mapToItem(
                        Overlay.overlay,
                        hamburger.width + Style.spacingXSmall,
                        (hamburger.height - toolbarPopup.implicitHeight) * .17
                    )

                    toolbarPopup.x = position.x
                    toolbarPopup.y = position.y

                    toolbarPopup.open()
                }
            }
        }

        Popup {
            id: toolbarPopup

            parent: Overlay.overlay

            padding: 0
            margins: 0

            modal: false
            focus: true

            closePolicy: toolbar.childMenuOpen
                         ? Popup.CloseOnEscape
                         : Popup.CloseOnPressOutside
                           | Popup.CloseOnEscape

            onClosed: {
                hamburger.visible = true
            }

            background: Rectangle {
                color: "transparent"
            }

            contentItem: Row {
                spacing: Style.spacingXSmall

                Repeater {
                    id: repeater
                    z: 2

                    model: visible ? toolbar.menus : null

                    delegate: Item {
                        id: menuDelegate

                        required property var modelData
                        required property int index

                        implicitWidth: menuButton.implicitWidth
                        implicitHeight: menuButton.implicitHeight

                        GXWindowHeaderToolbarButton {
                            id: menuButton

                            anchors.fill: parent
                            text: menuDelegate.modelData.title

                            onClicked: {
                                toolbar.openMenu(menuDelegate.index)
                            }

                            onHoveredChanged: {
                                if (hovered && toolbar.childMenuOpen && toolbar.activeMenuIndex !== menuDelegate.index)
                                    toolbar.openMenu(menuDelegate.index)
                            }
                        }

                        Loader {
                            id: menuLoader

                            active: menuDelegate.modelData.menu !== undefined
                            sourceComponent: menuDelegate.modelData.menu

                            onLoaded: {
                                item.parent = toolbar.window.contentItem

                                item.closed.connect(function() {
                                    toolbar.menuClosed(menuDelegate.index)
                                })
                                item.closeRequested.connect(function() {
                                    menuDelegate.closeMenu()
                                    activeMenuIndex = -1
                                    toolbarPopup.close()
                                })
                            }
                        }

                        function openMenu(index) {
                            if (!menuLoader.item)
                                    return

                                const position = menuButton.mapToItem(
                                    toolbar.window.contentItem,
                                    0,
                                    menuButton.height
                                )

                                menuLoader.item.x = position.x
                                menuLoader.item.y = position.y
                                menuLoader.item.open()
                        }

                        function closeMenu() {
                            if (menuLoader.item) {
                                menuLoader.item.close()
                                activeMenuIndex = -1
                                toolbarPopup.close()
                            }
                        }
                    }
                }
            }
        }
    }

    function openMenu(index) {
        const selectedDelegate = repeater.itemAt(index)

        if (!selectedDelegate)
            return

        // Clicking the currently open menu closes it.
        if (activeMenuIndex === index) {
            activeMenuIndex = -1
            selectedDelegate.closeMenu()
            return
        }

        const previousIndex = activeMenuIndex

        // Set the new index before closing the previous popup.
        activeMenuIndex = index

        if (previousIndex >= 0) {
            const previousDelegate = repeater.itemAt(previousIndex)

            if (previousDelegate)
                previousDelegate.closeMenu()
        }

        selectedDelegate.openMenu()
    }

    function menuClosed(index) {
        // Ignore the old popup closing while switching to another menu.
        if (activeMenuIndex === index)
            activeMenuIndex = -1
    }
}
