// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import Qt5Compat.GraphicalEffects

Control {
    id: root

    /*
        columns: array of column descriptors:
        [
            {
              model: <int | array | ListModel | QAbstractItemModel>,
              currentIndex: 0,                 // optional initial index
              width: 96,                       // optional
              textRole: "name",                // optional (for model objects)
              valueRole: "id",                 // optional
              textFunction: function(item, index) { return ... },  // optional override
              onIndexChanged: function(index) { ... }              // optional callback
            },
            ...
        ]
    */
    property var columns: []
    property var indices: []
    property var values: []

    property int _lastColumnCount: -1
    property int columnSpacing: 12

    property bool resetOnColumnsChanged: true
    property bool useGradient: true
    property bool flipGradient: false

    property real cardRadius: 24
    property real borderWidth: 1
    property real borderOpacity: 0.25

    property color textColor: "#000000"
    property color backgroundColor: "#0c2a46"
    property color backgroundTopColor: "#1a4f76"
    property color backgroundBottomColor: "#041326"
    property color borderColor: "#041326"

    implicitHeight: contentItem.implicitHeight
    implicitWidth: contentItem.implicitWidth

    signal changed(int column, int index, var value)

    background: Rectangle {
        id: cardBackground
        radius: root.cardRadius
        color: root.backgroundColor
        border.color: Qt.rgba(
            Qt.rgba(borderColor.r, borderColor.g, borderColor.b).r,
            Qt.rgba(borderColor.r, borderColor.g, borderColor.b).g,
            Qt.rgba(borderColor.r, borderColor.g, borderColor.b).b,
            borderOpacity
        )
        border.width: root.borderWidth

        gradient: Gradient {
            GradientStop {
                position: flipGradient === true ? 0 : -0.65
                color: useGradient === true ? (flipGradient === true ? root.backgroundBottomColor : root.backgroundTopColor) : root.backgroundColor
            }
            GradientStop {
                position: flipGradient === true ? 1.65 : 0.95
                color: useGradient === true ? (flipGradient === true ? root.backgroundTopColor : root.backgroundBottomColor) : root.backgroundColor
            }
        }

        layer.enabled: true
        layer.effect: DropShadow {
            transparentBorder: true
            horizontalOffset: 0
            verticalOffset: 0
            radius: 18
            samples: 32
            color: root.borderColor
            spread: 0.1
        }
    }

    contentItem: RowLayout {
        id: row
        spacing: root.columnSpacing

        Repeater {
            model: (root.columns ? root.columns.length : 0)

            Tumbler {
                id: t
                required property int index

                Layout.preferredWidth: (root.columns[index] && root.columns[index].width) ? root.columns[index].width : 110

                model: root.columns[index] ? root.columns[index].model : 0

                currentIndex: (root.columns[index] && root.columns[index].currentIndex !== undefined) ? root.columns[index].currentIndex : 0

                delegate: Text {
                    color: textColor
                    required property int index
                    readonly property var col: root.columns[t.index] || ({})
                    readonly property var item: (typeof modelData !== "undefined") ? modelData : null

                    font.bold: (index === t.currentIndex)
                    opacity: (index === t.currentIndex) ? 1.0 : ((index === t.currentIndex - 2 || index === t.currentIndex + 2) ? 0.4 : 0.6)
                    Behavior on opacity {
                        PropertyAnimation {
                            duration: 500
                        }
                    }

                    text: {
                        if (col.textFunction)
                            return col.textFunction(item, index)

                        if (col.textRole && item && typeof item === "object" && item[col.textRole] !== undefined)
                            return String(item[col.textRole])

                        // For numeric model, modelData is the index; for JS arrays it’s the value
                        if (item !== null && item !== undefined)
                            return String(item)

                        return String(index)
                    }

                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }

                onCurrentIndexChanged: {
                    root._syncFromTumbler(t.index, currentIndex)

                    if (!t.moving) return

                    const col = root.columns[t.index] || ({})
                    if (col.onIndexChanged)
                        col.onIndexChanged(currentIndex)
                }
            }
        }
    }

    // onColumnsChanged: {
    //     if (!resetOnColumnsChanged) return;

    //     // Create indices/values arrays of correct size
    //     root.indices = new Array(root.columns.length).fill(0)
    //     root.values  = new Array(root.columns.length).fill(undefined)

    //     // Prime them from column currentIndex if available
    //     for (let i = 0; i < root.columns.length; ++i) {
    //         const ci = (root.columns[i] && root.columns[i].currentIndex !== undefined)
    //                    ? root.columns[i].currentIndex : 0
    //         root.indices[i] = Math.max(0, ci)
    //     }

    //     // values will sync once tumblers are completed; also try to compute now if possible
    //     for (let j = 0; j < root.columns.length; ++j)
    //         root.values[j] = root._valueFor(j, root.indices[j])
    // }
    // onColumnsChanged: {
    //     if (!resetOnColumnsChanged) return;

    //     const cols = Array.isArray(root.columns) ? root.columns : []
    //     const n = cols.length

    //     // ✅ Only rebuild when count changes (or first time)
    //     if (n === root._lastColumnCount && root._lastColumnCount !== -1)
    //         return;

    //     root._lastColumnCount = n

    //     root.indices = new Array(n).fill(0)
    //     root.values  = new Array(n).fill(undefined)

    //     for (let i = 0; i < n; ++i) {
    //         const ci = (cols[i] && cols[i].currentIndex !== undefined) ? cols[i].currentIndex : 0
    //         root.indices[i] = Math.max(0, ci)
    //     }

    //     for (let j = 0; j < n; ++j)
    //         root.values[j] = root._valueFor(j, root.indices[j])
    // }

    // function _valueFor(colIndex, idx) {
    //     const col = root.columns[colIndex] || ({})
    //     const m = col.model

    //     // Numeric model => value is index (+1 is often desired but app-specific)
    //     if (typeof m === "number")
    //         return idx

    //     // JS array model
    //     if (Array.isArray(m)) {
    //         const item = m[idx]
    //         if (col.valueRole && item && typeof item === "object" && item[col.valueRole] !== undefined)
    //             return item[col.valueRole]
    //         return item
    //     }

    //     // Unknown model type (ListModel/QAbstractItemModel)
    //     // We can't index into it directly in JS reliably; return index and let caller interpret.
    //     return idx
    // }

    function _valueFor(colIndex, idx) {
        const col = root.columns[colIndex] || ({})
        const m = col.model

        // If caller provided a valueFunction, use it first
        if (col.valueFunction) {
            // item is only meaningful for arrays; for numeric models pass null
            let item = null
            if (Array.isArray(m))
                item = m[idx]
            return col.valueFunction(item, idx)
        }

        // Numeric model => default value is index
        if (typeof m === "number")
            return idx

        // JS array model
        if (Array.isArray(m)) {
            const item = m[idx]
            if (col.valueRole && item && typeof item === "object" && item[col.valueRole] !== undefined)
                return item[col.valueRole]
            return item
        }

        // Unknown model type
        return idx
    }

    function _syncFromTumbler(colIndex, idx) {
        const newIdx = idx
        const newVal = root._valueFor(colIndex, newIdx)

        // ensure arrays are sized
        if (!root.indices || root.indices.length !== root.columns.length)
            root.indices = new Array(root.columns.length).fill(0)
        if (!root.values || root.values.length !== root.columns.length)
            root.values = new Array(root.columns.length).fill(undefined)

        root.indices[colIndex] = newIdx
        root.values[colIndex] = newVal
        root.changed(colIndex, newIdx, newVal)
    }

    // Convenience API
    function setIndex(column, index) {
        // update our arrays; Tumbler will also trigger sync when it updates
        if (!root.indices || root.indices.length !== root.columns.length)
            root.indices = new Array(root.columns.length).fill(0)
        root.indices[column] = index
    }
}
