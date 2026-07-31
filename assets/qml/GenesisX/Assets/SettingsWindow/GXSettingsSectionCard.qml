import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

AbstractButton {
    id: root

    property string sectionTitle
    property string sectionDescription
    property string sectionIcon

    implicitHeight: 84

    hoverEnabled: true

    background: Rectangle {
        radius: 7

        color: {
            if (!root.enabled)
                return "#1c1d20"

            if (root.down)
                return "#303338"

            if (root.hovered)
                return "#292b2f"

            return "#232529"
        }

        border.width: 1
        border.color: root.hovered && root.enabled
                      ? "#4a4d52"
                      : "#35373b"

        Behavior on color {
            ColorAnimation {
                duration: 100
            }
        }
    }

    contentItem: RowLayout {
        spacing: 16

        Item {
            Layout.preferredWidth: 42
            Layout.preferredHeight: 42

            Rectangle {
                anchors.fill: parent

                radius: 6
                color: "#303338"

                Text {
                    anchors.centerIn: parent

                    // Temporary placeholder until GXIcon names are connected.
                    text: root.sectionTitle.length > 0
                          ? root.sectionTitle.charAt(0)
                          : ""

                    color: root.enabled ? "#ffffff" : "#707278"
                    font.pixelSize: 18
                    font.weight: Font.DemiBold
                }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true

            spacing: 5

            Label {
                Layout.fillWidth: true

                text: root.sectionTitle
                color: root.enabled ? "#ffffff" : "#777a80"

                font.pixelSize: 15
                font.weight: Font.Medium
            }

            Label {
                Layout.fillWidth: true

                text: root.sectionDescription
                color: root.enabled ? "#a8abb2" : "#62646a"

                font.pixelSize: 13
                wrapMode: Text.WordWrap
            }
        }

        Label {
            text: "›"

            visible: root.enabled

            color: root.hovered ? "#ffffff" : "#8b8e94"
            font.pixelSize: 26
        }
    }
}