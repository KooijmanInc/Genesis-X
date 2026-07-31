import QtQuick
import QtQuick.Controls

import GenesisX.Assets 1.0

Item {
    id: settingsNavigation

    property string currentPage: "appearance"

    signal pageSelected(string page)

    GXSettingsNavigationModel {
        id: navigationModel
    }

    ListView {
        id: navigationList

        anchors.fill: parent
        clip: true

        model: navigationModel

        delegate: Rectangle {
            id: navigationItem

            required property string key
            required property string label
            required property int depth
            required property bool expandable
            required property bool expanded
            required property string page
            required property int index
            required property bool visibleItem
            visible: navigationItem.visibleItem ?? false
            height: navigationItem.visible ? 30 : 0

            width: navigationList.width

            color: {
                if (mouseArea.containsMouse)
                    return "#313338"

                if (page !== "" && navigationItem.currentPage === page)
                    return "#3a3d42"

                return "transparent"
            }

            Row {
                anchors {
                    fill: parent
                    leftMargin: 8 + navigationItem.depth * 18
                    rightMargin: 8
                }

                spacing: 6

                Text {
                    width: navigationItem.expandable ? 12 : 12
                    height: parent.height

                    font.family: Style.fontAwesome
                    text: {
                        if (!navigationItem.expandable)
                            return ""

                        // return navigationItem.expanded ? "▾" : "▸"
                        return navigationItem.expanded ? "\uf078" : "\uf054"
                    }

                    color: "#cfcfcf"
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                }

                Text {
                    height: parent.height

                    text: navigationItem.label
                    color: "#f0f0f0"

                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }
            }

            MouseArea {
                id: mouseArea

                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor

                onClicked: {
                    if (navigationItem.expandable) {
                        const newExpanded = !navigationItem.expanded

                        navigationModel.setProperty(
                            navigationItem.index,
                            "expanded",
                            newExpanded
                        )

                        for (let i = navigationItem.index + 1;
                            i < navigationModel.count;
                            ++i) {

                            if (navigationModel.get(i).depth <= navigationItem.depth)
                                break

                            navigationModel.setProperty(
                                i,
                                "visibleItem",
                                newExpanded
                            )
                        }
                        settingsNavigation.currentPage = navigationItem.page
                        settingsNavigation.pageSelected(navigationItem.page)
                    } else if (navigationItem.page !== "") {
                        settingsNavigation.currentPage = navigationItem.page
                        settingsNavigation.pageSelected(navigationItem.page)
                    }
                }
            }
        }
    }
}
