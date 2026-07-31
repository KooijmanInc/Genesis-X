import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    id: root

    property string title
    default property alias content: groupContent.data

    spacing: 10

    Label {
        text: root.title

        color: "#ffffff"
        font.pixelSize: 15
        font.weight: Font.DemiBold
    }

    Rectangle {
        Layout.fillWidth: true
        implicitHeight: groupContent.implicitHeight + 28

        radius: 7

        color: "#232529"

        border.width: 1
        border.color: "#35373b"

        ColumnLayout {
            id: groupContent

            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
                margins: 14
            }

            spacing: 10
        }
    }
}