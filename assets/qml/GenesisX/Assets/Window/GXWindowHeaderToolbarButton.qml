import QtQuick
import QtQuick.Layouts

import ".."

Item {
    id: toolbarButton

    property string text: ""

    property int horizontalPadding: 3
    property int verticalPadding: 4

    signal clicked()

    readonly property bool hovered: mouseArea.containsMouse
    readonly property bool pressed: mouseArea.pressed

    implicitWidth: label.implicitWidth + horizontalPadding * 2
    implicitHeight: label.implicitHeight + verticalPadding * 2

    Text {
        id: label

        anchors.centerIn: parent
        color: Style.textSecondary

        text: toolbarButton.text

        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
    }

    MouseArea {
        id: mouseArea

        anchors.fill: parent
        hoverEnabled: true

        cursorShape: Qt.PointingHandCursor

        onClicked: toolbarButton.clicked()
    }
}
