// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

import QtQuick
import GenesisX3D 1.0

Item {
    id: root

    property GXNode controlledObject: undefined

    property real speed: 0.5

    property real forwardSpeed: 0.1
    property real backSpeed: 0.1
    property real leftSpeed: 0.1
    property real rightSpeed: 0.1
    property real turnLeftSpeed: 0.5
    property real turnRightSpeed: 0.5
    property real turnUpSpeed: 0.5
    property real turnDownSpeed: 0.5

    property bool keysEnabled: true

    readonly property bool inputsNeedProcessing: status.moveForward | status.moveBack | status.moveLeft | status.moveRight | status.turnLeft | status.turnRight | status.turnUp | status.turnDown

    implicitHeight: parent.height
    implicitWidth: parent.width
    focus: keysEnabled

    Timer {
        id: setFocus
        interval: 1000
        running: true
        repeat: true
        onTriggered: {
            if (keysEnabled === true) {
                root.focus = true
                root.forceActiveFocus()
            }
        }
    }

    Keys.onPressed: (event) => { if (keysEnabled && !event.isAutoRepeat) handleKeyPress(event) }
    Keys.onReleased: (event) => { if (keysEnabled && !event.isAutoRepeat) handleKeyRelease(event) }

    function handleKeyPress(event) {
        switch (event.key) {
        case Qt.Key_W:
            forwardPressed();
            break;
        case Qt.Key_S:
            backPressed();
            break;
        case Qt.Key_A:
            leftPressed();
            break;
        case Qt.Key_D:
            rightPressed();
            break;
        case Qt.Key_Left:
            turnLeftPressed();
            break;
        case Qt.Key_Right:
            turnRightPressed();
            break;
        case Qt.Key_Up:
            upPressed();
            break;
        case Qt.Key_Down:
            downPressed();
        }
    }

    function handleKeyRelease(event) {
        switch (event.key) {
        case Qt.Key_W:
            forwardReleased();
            break;
        case Qt.Key_S:
            backReleased();
            break;
        case Qt.Key_A:
            leftReleased();
            break;
        case Qt.Key_D:
            rightReleased();
            break;
        case Qt.Key_Left:
            turnLeftReleased();
            break;
        case Qt.Key_Right:
            turnRightReleased();
            break;
        case Qt.Key_Up:
            upReleased();
            break;
        case Qt.Key_Down:
            downReleased();
            break;
        }
    }

    function forwardPressed() {
        status.moveForward = true
        status.moveBack = false
    }

    function forwardReleased() {
        status.moveForward = false
    }

    function backPressed() {
        status.moveBack = true
        status.moveForward = false
    }

    function backReleased() {
        status.moveBack = false
    }

    function leftPressed() {
        status.moveLeft = true
        status.moveRight = false
    }

    function leftReleased() {
        status.moveLeft = false
    }

    function rightPressed() {
        status.moveRight = true
        status.moveLeft = false
    }

    function rightReleased() {
        status.moveRight = false
    }

    function turnLeftPressed() {
        status.turnLeft = true
        status.turnRight = false
    }

    function turnLeftReleased() {
        status.turnLeft = false
    }

    function turnRightPressed() {
        status.turnRight = true
        status.turnLeft = false
    }

    function turnRightReleased() {
        status.turnRight = false
    }

    function upPressed() {
        status.turnUp = true
        status.turnDown = false
    }

    function upReleased() {
        status.turnUp = false
    }

    function downPressed() {
        status.turnDown = true
        status.turnUp = false
    }

    function downReleased() {
        status.turnDown = false
    }

    FrameAnimation {
        id: updateTimer
        running: root.inputsNeedProcessing
        onTriggered: status.processInput(frameTime * 100)
    }

    QtObject {
        id: status

        property bool moveForward: false
        property bool moveBack: false
        property bool moveLeft: false
        property bool moveRight: false
        property bool turnLeft: false
        property bool turnRight: false
        property bool turnUp: false
        property bool turnDown: false

        function updatePosition(vector, speed, position) {
            speed *= root.speed

            var direction = vector;
            var velocity = Qt.vector3d(direction.x * speed, direction.y * speed, direction.z * speed)
            controlledObject.position = Qt.vector3d(position.x + velocity.x, position.y + velocity.y, position.z + velocity.z)
        }

        function processInput(frameDelta) {
            if (root.controlledObject == undefined) return;

            if (moveForward) updatePosition(root.controlledObject.forward, root.forwardSpeed * frameDelta, root.controlledObject.position)
            else if (moveBack) updatePosition(root.controlledObject.back, root.backSpeed * frameDelta, root.controlledObject.position)

            if (moveRight) updatePosition(root.controlledObject.right, root.rightSpeed * frameDelta, root.controlledObject.position)
            else if (moveLeft) updatePosition(root.controlledObject.left, root.leftSpeed * frameDelta, root.controlledObject.position)

            if (turnRight) root.controlledObject.addYaw(-root.turnRightSpeed * frameDelta)
            else if (turnLeft) root.controlledObject.addYaw(root.turnLeftSpeed * frameDelta)

            if (turnUp) root.controlledObject.addPitch(root.turnUpSpeed * frameDelta)
            else if (turnDown) root.controlledObject.addPitch(-root.turnDownSpeed * frameDelta)
        }
    }
}
