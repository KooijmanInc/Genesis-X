import QtQuick
import QtQuick.Layouts

import ".."

Item {
    id: windowIdentity

    property alias iconSource: icon.source
    property string title: ""
    property color textColor: Style.textPrimary

    property int iconSize: 16
    property int iconBlockWidth: 24
    property int spacing: Style.spacingXSmall

    implicitWidth: contentRow.implicitWidth
    implicitHeight: 34

    RowLayout {
        id: contentRow

        anchors.fill: parent
        spacing: windowIdentity.spacing

        Item {
            id: iconBlock

            Layout.preferredWidth: windowIdentity.iconSize
            Layout.preferredHeight: windowIdentity.iconSize

            Image {
                id: icon

                anchors.centerIn: parent

                source: "qrc:/qt/qml/GenesisX/Assets/logo.ico"

                width: windowIdentity.iconSize
                height: windowIdentity.iconSize

                fillMode: Image.PreserveAspectFit
                smooth: true
            }
        }

        Text {
            Layout.preferredWidth: implicitWidth
            Layout.fillHeight: true

            color: windowIdentity.textColor
            text: windowIdentity.title

            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignLeft

            elide: Text.ElideRight
            clip: true
        }
    }
}
