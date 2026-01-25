// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

import QtQuick
import QtQuick.Controls
import Qt.labs.platform as Platform
import QtMultimedia

import GenesisX.Assets 1.0
import GenesisX.Images 1.0
import GenesisX.SystemInfo 1.0

Item {
    id: root

    property alias card: card
    property alias albumButton: albumButton
    property alias cameraButton: cameraButton

    property alias previewSource: preview.source

    property bool cameraMode: false

    signal imageReady(url fileUrl)

    function open() {
        chooser.open()
    }

    function openCamera() {
        root.cameraMode = true
        cam.start()
    }

    function takePhoto() {
        // Save into temp (works desktop + mobile)
        const path = StandardPaths.writableLocation(StandardPaths.TempLocation) + "/profile_photo.jpg"
        snap.captureToFile(path)
    }

    Connections {
        target: PhotoPicker

        function onImageReady(path) {
            imageReady(path)
        }

        function onError(message) {
            console.log(message)
        }
    }

    Popup {
        id: chooser
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        anchors.centerIn: parent

        background: Rectangle {
            color: "transparent"
        }

        GXCard {
            id: card
            anchors.fill: parent

            GXButton {
                id: albumButton
                anchors.left: parent.left
                anchors.right: parent.right
            }
            MouseArea {
                anchors.fill: albumButton
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    chooser.close()
                    if (SystemInfo.platform() === "android" || SystemInfo.platform() === "ios") {console.log("photo picker galery")
                        PhotoPicker.pickFromGallery()
                    } else {
                        fileDialog.open()
                    }
                }
            }
            GXButton {
                id: cameraButton
                anchors.top: albumButton.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.topMargin: 20
            }
            MouseArea {
                anchors.fill: cameraButton
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    chooser.close()
                    if (SystemInfo.platform() === "android" || SystemInfo.platform() === "ios") {console.log("photo picker take photo")
                        PhotoPicker.takePhoto()
                    } else {
                        openCamera()
                    }
                }
            }
        }
    }

    Platform.FileDialog {
        id: fileDialog
        title: "Select profile photo"
        nameFilters: ["Images (*.png *.jpg *.jpeg *.webp)"]
        onAccepted: root.imageReady(file)
    }

    MediaDevices { id: mediaDevices }

    CaptureSession {
        id: cap
        camera: Camera {
            id: cam
            cameraDevice: {
                for (let i = 0; i < mediaDevices.videoInputs.length; ++i) {
                    const dev = mediaDevices.videoInputs[i]
                    if (dev.position === CameraDevice.FrontFace)
                        return dev
                }
                return mediaDevices.defaultVideoInput
            }
        }
        videoOutput: videoOutput

        imageCapture: ImageCapture {
            id: snap
            onImageSaved: (fileName) => {
                root.cameraMode = false
                root.imageReady(Qt.resolvedUrl("file://" + fileName)) // fileName is local path
            }
        }
    }

    VideoOutput {
        id: videoOutput
        anchors.fill: parent
        visible: root.cameraMode
    }

    Image {
        id: preview
        anchors.fill: parent
        fillMode: Image.PreserveAspectCrop
        visible: false
    }
}
