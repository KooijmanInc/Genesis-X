import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    objectName: "appearanceBehavior"

    property bool canGoBack: false

    signal openPage(string page)

    ListModel {
        id: sectionModel

        ListElement {
            title: qsTr("Appearance")
            description: qsTr("Change the application theme, colors, icons and fonts.")
            iconName: "appearance"
            pageName: "appearance"
        }

        ListElement {
            title: qsTr("Language")
            description: qsTr("Change the interface language.")
            iconName: "language"
            pageName: "language"
        }

        ListElement {
            title: qsTr("Shortcuts")
            description: qsTr("Configure keyboard shortcuts.")
            iconName: "keyboard"
            pageName: "shortcuts"
        }

        ListElement {
            title: qsTr("Behavior")
            description: qsTr("Configure startup, navigation and interaction.")
            iconName: "behavior"
            pageName: "behavior"
        }

        ListElement {
            title: qsTr("Notifications")
            description: qsTr("Configure desktop notifications.")
            iconName: "notifications"
            pageName: "notifications"
        }
    }

    Flickable {
        id: flickable
        anchors.fill: parent

        contentWidth: width
        contentHeight: pageLayout.implicitHeight + 48

        clip: true

        ScrollBar.vertical: ScrollBar {}

        ColumnLayout {
            id: pageLayout

            width: Math.min(flickable.width - 48, 900)
            anchors.horizontalCenter: parent.horizontalCenter

            spacing: 20

            Item {
                Layout.preferredHeight: 12
            }

            Label {
                text: qsTr("Appearance & Behavior")

                color: "#ffffff"
                font.pixelSize: 24
                font.weight: Font.DemiBold
            }

            Label {
                Layout.fillWidth: true

                text: qsTr(
                    "Customize the look and feel of LabWorks and how the user interface behaves."
                )

                color: "#a8abb2"
                font.pixelSize: 14
                wrapMode: Text.WordWrap
            }

            Item {
                Layout.preferredHeight: 4
            }

            Repeater {
                model: sectionModel

                delegate: GXSettingsSectionCard {
                    required property string title
                    required property string description
                    required property string iconName
                    required property string pageName

                    Layout.fillWidth: true

                    sectionTitle: title
                    sectionDescription: description
                    sectionIcon: iconName

                    enabled: pageName === "appearance"

                    onClicked: root.openPage(pageName)
                }
            }

            Item {
                Layout.preferredHeight: 24
            }
        }
    }
}