import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Fusion
import Qt.labs.platform as Platform

import components

Item {
    id: root



    Rectangle {
        anchors.fill: parent
        color: "#0f1220"
    }

    Platform.FileDialog {
        id: fileDialog
        title: "Select GLTF file"
        nameFilters: ["3D Scene files (*.glb *.gltf)"]
        Component.onCompleted: {
            fileDialog.folder = converter.currentDirectory
        }
        onAccepted: {
            converter.fileUrl = fileDialog.file
        }
    }

    Platform.FolderDialog {
        id: folderDialog
        title: "Select output directory"

        Component.onCompleted: {
            folderDialog.folder = Qt.resolvedUrl(converter.outputFileDirectory)
        }

        onAccepted: {
            const path = folderDialog.currentFolder

            converter.outputFileDirectory = path
            // selectedDirectory.text = converter.selectedDirectory
        }
    }

    function setFolder(folder) {
        console.log(folder)
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 10

        Text {
            id: inputFiles
            Layout.fillWidth: true
            Layout.topMargin: 10
            Layout.leftMargin: 10
            text: "Input files:"
            color: "#FFFFFF"
            font.pixelSize: 13
        }

        RowLayout {
            Layout.fillWidth: true
            // Layout.preferredHeight: (root.height / 2)
            Layout.preferredHeight: 220
            Layout.maximumHeight: root.height * 0.4
            spacing: 10

            Rectangle {
                id: inputList
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.margins: 10
                clip: true
                radius: 10
                border.width: 1
                border.color: "#556070"
                color: "#0f1220"
                Text {
                    width: parent.width
                    height: 20
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.topMargin: 10
                    anchors.leftMargin: 10
                    text: converter.file
                    font.pixelSize: 13
                    color: "#FFFFFF"
                }
            }

            ColumnLayout {
                spacing: 8
                Layout.preferredWidth: browseButton.implicitWidth
                Layout.topMargin: 10
                Layout.rightMargin: 20
                Layout.alignment: Qt.AlignTop

                GXButton {
                    id: browseButton
                    // Layout.fillWidth: true
                    text: "Browse..."

                    onClicked: {
                        fileDialog.open()
                    }
                }

                // Button { text: "Browse..." }
                // Button { text: "Remove selected" }
                // Button { text: "Select all" }
            }
        }

        Text {
            id: outputFolder
            Layout.fillWidth: true
            Layout.leftMargin: 10
            text: "Output folder:"
            color: "#FFFFFF"
            font.pixelSize: 13
        }

        RowLayout {
            id: outputFolderInputBox
            Layout.fillWidth: true
            Layout.preferredHeight: folderBrowseBox.height
            spacing: 10

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: folderBrowseBox.height
                Layout.margins: 10
                clip: true
                radius: 10
                border.width: 1
                border.color: "#556070"
                color: "#0f1220"
                TextInput {
                    id: selectedDirectory
                    anchors.fill: parent
                    anchors.leftMargin: 10
                    anchors.topMargin: 5
                    font.pixelSize: 13
                    color: "#FFFFFF"
                    clip: true
                    text: converter.selectedDirectory
                }
            }

            ColumnLayout {
                id: folderBrowseBox
                spacing: 8
                Layout.preferredWidth: folderBrowseButton.implicitWidth
                Layout.topMargin: 10
                Layout.rightMargin: 20
                Layout.alignment: Qt.AlignTop

                GXButton {
                    id: folderBrowseButton
                    text: "Browse..."

                    onClicked: {
                        folderDialog.open()
                    }
                }
            }
        }

        Text {
            id: statusText
            Layout.fillWidth: true
            Layout.leftMargin: 10
            text: "Status:"
            color: "#FFFFFF"
            font.pixelSize: 13
        }

        Rectangle {
            id: status
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: 10
            clip: true
            radius: 10
            border.width: 1
            border.color: "#556070"
            color: "#0f1220"
            ScrollView {
                anchors.fill: parent

                TextArea {
                    readOnly: true
                    wrapMode: Text.Wrap
                    background: Rectangle {
                        color: "transparent"
                    }
                    text: converter.status
                }
            }
        }

        GXButton {
            Layout.fillWidth: true
            Layout.preferredHeight: folderBrowseBox.height
            Layout.leftMargin: 10
            Layout.rightMargin: 10
            Layout.bottomMargin: 10
            text: "Convert"
            onClicked: {
                converter.convertToMesh()
            }
        }
    }
}
