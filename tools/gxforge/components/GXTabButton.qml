import QtQuick

Rectangle {
    id: root

    property string text: ""
    property bool active: false

    signal clicked()

    radius: 10
    height: 32
    color: active ? "#2a2f3a" : "#1f2430"
    border.width: 1
    border.color: active ? "#556070" : "#333a46"

    implicitWidth: Math.max(100, label.implicitWidth + 20)

    Text {
        id: label
        anchors.centerIn: parent
        text: root.text
        color: "#FFFFFF"
        font.pixelSize: 13
        opacity: root.active ? 1.0 : 0.85
        elide: Text.ElideRight
        width: parent.width - 20
        horizontalAlignment: Text.AlignHCenter
    }

    MouseArea {
        anchors.fill: parent
        onClicked: root.clicked()
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
    }
}
