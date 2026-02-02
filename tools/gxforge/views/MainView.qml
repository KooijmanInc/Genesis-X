import QtQuick
import QtQuick.Controls

Window {
    id: mainView
    visible: true
    width: 1024
    height: 768

    title: qsTr("Genesis-X 3D model converter")

    StackView {
        id: contentView
        anchors.fill: parent
        initialItem: "qrc:/views/TabView.qml"
    }
}
