import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    objectName: "appearance"

    property bool canGoBack: false

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

            spacing: 22

            Item {
                Layout.preferredHeight: 12
            }

            RowLayout {
                Layout.fillWidth: true

                spacing: 12

                Button {
                    visible: root.canGoBack

                    text: "‹"

                    flat: true

                    font.pixelSize: 24

                    onClicked: {
                        if (StackView.view !== null)  StackView.view.pop()
                    }
                }

                Label {
                    text: qsTr("Appearance")

                    color: "#ffffff"
                    font.pixelSize: 24
                    font.weight: Font.DemiBold
                }
            }

            Label {
                Layout.fillWidth: true

                text: qsTr("Customize the appearance of LabWorks.")

                color: "#a8abb2"
                font.pixelSize: 14
                wrapMode: Text.WordWrap
            }

            GXSettingsGroup {
                Layout.fillWidth: true

                title: qsTr("Theme")

                RadioButton {
                    text: qsTr("System")
                }

                RadioButton {
                    text: qsTr("Light")
                }

                RadioButton {
                    text: qsTr("Dark")
                    checked: true
                }
            }

            GXSettingsGroup {
                Layout.fillWidth: true

                title: qsTr("Accent Color")

                ComboBox {
                    Layout.preferredWidth: 240

                    model: [
                        qsTr("Teal"),
                        qsTr("Blue"),
                        qsTr("Purple"),
                        qsTr("Orange")
                    ]
                }
            }

            GXSettingsGroup {
                Layout.fillWidth: true

                title: qsTr("Application Font")

                RowLayout {
                    Layout.fillWidth: true

                    spacing: 12

                    Label {
                        text: qsTr("Font family")
                        color: "#d6d7da"
                    }

                    ComboBox {
                        Layout.fillWidth: true

                        model: Qt.fontFamilies()
                    }

                    Label {
                        text: qsTr("Size")
                        color: "#d6d7da"
                    }

                    SpinBox {
                        from: 8
                        to: 24
                        value: 10
                    }
                }
            }

            GXSettingsGroup {
                Layout.fillWidth: true

                title: qsTr("Window")

                CheckBox {
                    text: qsTr("Enable animations")
                    checked: true
                }

                CheckBox {
                    text: qsTr("Show window shadows")
                    checked: true
                }

                CheckBox {
                    text: qsTr("Use rounded window corners")
                    checked: true
                }
            }

            Item {
                Layout.preferredHeight: 24
            }
        }
    }
}