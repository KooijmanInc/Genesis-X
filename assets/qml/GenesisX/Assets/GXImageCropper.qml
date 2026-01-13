import QtQuick
import QtQuick.Controls

import GenesisX.Images 1.0

Item {
    id: root

    property url source
    // property alias img: img

    property real cropAspectPortrait: 1/1
    property real cropAspectLandscape: 1/1

    property int outputWidth: 1024
    property int outputHeight: 1024

    property bool isLandscape: img.status === Image.Ready && img.sourceSize.width > img.sourceSize.height

    property rect cropRect: Qt.rect(0, 0, 0, 0)

    signal accepted(url outFile)
    signal cancelled()

    Rectangle {
        id: background
        anchors.fill: parent
        color: "#000000"
    }

    Image {
        id: img

        anchors.fill: parent
        fillMode: isLandscape ? Image.PreserveAspectFit : Image.PreserveAspectCrop
        smooth: true
        mipmap: true
        z: 1
        source: root.source
    }

    function resetCrop() {
        // const margin = Math.min(root.width, root.height) * 0.08
        // const aspect = root.isLandscape ? root.cropAspectLandscape : root.cropAspectPortrait

        // let w = root.width - margin * 2
        // let h = w / aspect
        // if (h > root.height - margin * 2) {
        //     h = root.height - margin * 2
        //     w = h * aspect
        // }

        const pr = paintedRect()
        if (pr.width <= 0 || pr.height <= 0) return

        const margin = Math.min(pr.width, pr.height) * 0.08
        const aspect = root.isLandscape ? root.cropAspectLandscape : root.cropAspectPortrait

        let w = pr.width - margin * 2
        let h = w / aspect
        if (h > pr.height - margin * 2) {
            h = pr.height - margin * 2
            w = h * aspect
        }

        root.cropRect = Qt.rect(
            pr.x + (root.width - w) / 2,
            pr.y + (root.height - h) / 2,
            w,
            h
        )
    }

    onWidthChanged: {
        Qt.callLater(resetCrop)
    }
    onHeightChanged: {
        Qt.callLater(resetCrop)
    }

    Connections {
        target: img

        function onStatusChanged() {
            if (img.status === Image.Ready) {
                Qt.callLater(() => {
                    root.resetCrop()
                    mask.requestPaint()
                })
            }
        }
    }

    Canvas {
        id: mask

        anchors.fill: parent
        z: 2
        onPaint: {
            const ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)

            ctx.fillStyle = "rgba(0, 0, 0, 0.85)"
            ctx.fillRect(0, 0, width, height)

            const r = Math.min(root.cropRect.width, root.cropRect.height) * 0.08

            ctx.save()
            ctx.globalCompositeOperation = "destination-out"
            ctx.fillStyle = "rgba(0,0,0,1)"
            roundRectPath(ctx,
                          root.cropRect.x, root.cropRect.y,
                          root.cropRect.width, root.cropRect.height,
                          r)
            ctx.fill()
            ctx.restore()

            // Rounded border
            ctx.strokeStyle = "white"
            ctx.lineWidth = 2
            roundRectPath(ctx,
                          root.cropRect.x, root.cropRect.y,
                          root.cropRect.width, root.cropRect.height,
                          r)
            ctx.stroke()
        }

        Connections {
            target: root
            function onCropRectChanged() {
                mask.requestPaint()
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.OpenHandCursor

        property real pressX
        property real pressY
        property rect startRect

        onPressed: (m) => {
            if (root.cropRect.width <= 0 || root.cropRect.height <= 0) return

            const inside =
                m.x >= root.cropRect.x && m.x <= root.cropRect.x + root.cropRect.width &&
                m.y >= root.cropRect.y && m.y <= root.cropRect.y + root.cropRect.height

            if (!inside) return

            pressX = m.x
            pressY = m.y
            startRect = root.cropRect
            cursorShape = Qt.ClosedHandCursor
        }

        onReleased: cursorShape = Qt.OpenHandCursor

        onPositionChanged: (m) => {
            if (startRect.width <= 0) return

            const pr = paintedRect()

            const dx = m.x - pressX
            const dy = m.y - pressY

            let nx = startRect.x + dx
            let ny = startRect.y + dy

            root.cropRect = clampRectInside(Qt.rect(nx, ny, startRect.width, startRect.height), pr)
        }
    }

    Row {
        spacing: 12
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 24
        z: 10

        Button {
            text: "Cancel"
            onClicked: root.cancelled()
        }
        Button {
            text: "Use"
            onClicked: {
                const crop = root.cropRectInImageNormalized()
                const out = ImageCrop.cropAndSave(root.source, crop, root.outputWidth, root.outputHeight, "JPG", 90)

                if (out && out.toString().length > 0) root.accepted(out)
            }
        }
    }

    function cropRectInImageNormalized() {
        const iw = img.sourceSize.width
        const ih = img.sourceSize.height
        if (iw <= 0 || ih <= 0) return Qt.rect(0, 0, 0, 0)

        const pr = paintedRect()
        if (pr.width <= 0 || pr.height <= 0) return Qt.rect(0, 0, 0, 0)

        const scale = pr.width / iw

        const xPx = (root.cropRect.x - pr.x) / scale
        const yPx = (root.cropRect.y - pr.y) / scale
        const wPx = root.cropRect.width / scale
        const hPx = root.cropRect.height / scale

        // normalize
        return Qt.rect(xPx / iw, yPx / ih, wPx / iw, hPx / ih)
    }

    function roundRectPath(ctx, x, y, w, h, r) {
        r = Math.max(0, Math.min(r, Math.min(w, h) / 2))

        ctx.beginPath()
        ctx.moveTo(x + r, y)
        ctx.arcTo(x + w, y,     x + w, y + h, r)
        ctx.arcTo(x + w, y + h, x,     y + h, r)
        ctx.arcTo(x,     y + h, x,     y,     r)
        ctx.arcTo(x,     y,     x + w, y,     r)
        ctx.closePath()
    }

    function cropRectInImagePixels() {
        const iw = img.sourceSize.width
        const ih = img.sourceSize.height
        if (iw <= 0 || ih <= 0) return Qt.rect(0, 0, 0, 0)

        const pr = paintedRect()
        if (pr.width <= 0 || pr.height <= 0) return Qt.rect(0, 0, 0, 0)

        const scale = pr.width / iw  // same as pr.height / ih

        const x = (root.cropRect.x - pr.x) / scale
        const y = (root.cropRect.y - pr.y) / scale
        const w = root.cropRect.width / scale
        const h = root.cropRect.height / scale

        const cx = Math.max(0, Math.min(x, iw))
        const cy = Math.max(0, Math.min(y, ih))
        const cw = Math.max(0, Math.min(w, iw - cx))
        const ch = Math.max(0, Math.min(h, ih - cy))

        return Qt.rect(cx, cy, cw, ch)
    }

    function paintedRect() {
        const iw = img.sourceSize.width
        const ih = img.sourceSize.height
        const cw = root.width
        const ch = root.height
        if (iw <= 0 || ih <= 0 || cw <= 0 || ch <= 0)
            return Qt.rect(0, 0, 0, 0)

        const sx = cw / iw
        const sy = ch / ih

        const scale = (img.fillMode === Image.PreserveAspectFit)
                ? Math.min(sx, sy)
                : Math.max(sx, sy) // PreserveAspectCrop

        const dw = iw * scale
        const dh = ih * scale
        const x = (cw - dw) / 2
        const y = (ch - dh) / 2
        return Qt.rect(x, y, dw, dh)
    }

    function clampRectInside(r, bounds) {
        let nx = r.x
        let ny = r.y
        let nw = r.width
        let nh = r.height

        if (nw > bounds.width) nw = bounds.width
        if (nh > bounds.height) nh = bounds.height

        nx = Math.max(bounds.x, Math.min(nx, bounds.x + bounds.width - nw))
        ny = Math.max(bounds.y, Math.min(ny, bounds.y + bounds.height - nh))
        return Qt.rect(nx, ny, nw, nh)
    }
}
