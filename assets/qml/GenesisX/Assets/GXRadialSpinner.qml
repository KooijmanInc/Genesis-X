import QtQuick

Item {
    id: root
    width: 64
    height: 64

    property color startColor: "#FFFFFF"
    property color endColor: "#000000"
    property real lineWidth: 4
    property int duration: 1400

    property alias start: startSpinning.running

    Canvas {
        id: canvas
        anchors.fill: parent

        property real startAngle: -90
        property real sweepAngle: 0

        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)

            var r = Math.min(width, height) / 2 - root.lineWidth
            var cx = width / 2
            var cy = height / 2

            var start = canvas.startAngle * Math.PI / 180
            var end   = (canvas.startAngle + canvas.sweepAngle) * Math.PI / 180

            var segments = 80   // more = smoother gradient
            var step = (end - start) / segments

            ctx.lineWidth = root.lineWidth
            ctx.lineCap = "round"

            for (var i = 0; i < segments; i++) {
                var t = i / segments

                // Simple 2-color gradient (customize as needed)
                var c1 = Qt.color(root.startColor)
                var c2 = Qt.color(root.endColor)

                var rCol = c1.r + (c2.r - c1.r) * t
                var gCol = c1.g + (c2.g - c1.g) * t
                var bCol = c1.b + (c2.b - c1.b) * t
                var aCol = c1.a + (c2.a - c1.a) * t

                ctx.strokeStyle = Qt.rgba(rCol, gCol, bCol, aCol)

                ctx.beginPath()
                ctx.arc(
                    cx, cy, r,
                    start + step * i,
                    start + step * (i + 1),
                    false
                )
                ctx.stroke()
            }
        }

        // onPaint: {
        //     var ctx = getContext("2d")
        //     ctx.clearRect(0, 0, width, height)

        //     ctx.beginPath()
        //     ctx.strokeStyle = root.color
        //     ctx.lineWidth = root.lineWidth
        //     ctx.lineCap = "round"

        //     var r = Math.min(width, height) / 2 - root.lineWidth
        //     var cx = width / 2
        //     var cy = height / 2

        //     ctx.arc(
        //         cx, cy, r,
        //         canvas.startAngle * Math.PI / 180,
        //         (canvas.startAngle + canvas.sweepAngle) * Math.PI / 180,
        //         false
        //     )

        //     ctx.stroke()
        // }

        onStartAngleChanged: requestPaint()
        onSweepAngleChanged: requestPaint()
    }

    SequentialAnimation {
        id: startSpinning
        running: false
        loops: Animation.Infinite

        NumberAnimation {
            target: canvas
            property: "startAngle"
            to: -90
            duration: 0
        }

        NumberAnimation {
            target: canvas
            property: "sweepAngle"
            from: 0
            to: 360
            duration: root.duration
            easing.type: Easing.InOutCubic
            onRunningChanged: canvas.requestPaint()
            onStopped: canvas.requestPaint()
        }

        ParallelAnimation {
            NumberAnimation {
                target: canvas
                property: "startAngle"
                from: -90
                to: 270      // -90 + 360
                duration: root.duration
                easing.type: Easing.InOutCubic
            }

            NumberAnimation {
                target: canvas
                property: "sweepAngle"
                from: 360
                to: 0
                duration: root.duration
                easing.type: Easing.InOutCubic
                onRunningChanged: canvas.requestPaint()
                onStopped: canvas.requestPaint()
            }
        }

        ScriptAction {
            script: { canvas.startAngle = -90; canvas.sweepAngle = 0; }
        }
    }
}
