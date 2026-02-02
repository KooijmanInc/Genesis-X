import QtQuick
import QtQuick.Layouts

import components

Item {
    id: root

    anchors.fill: parent

    EditorState { id: editorState }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        GXTabBar {
            Layout.fillWidth: true
            tabs: editorState.tabs
            currentIndex: editorState.currentIndex
            onTabActivated: editorState.setCurrent(index)
        }

        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: "#2b3240"
        }

        GXPanelHost {
            Layout.fillWidth: true
            Layout.fillHeight: true
            tabs: editorState.tabs
            currentIndex: editorState.currentIndex
        }
    }

    Keys.onPressed: (e) => {
        if (e.modifiers & Qt.ControlModifier && e.key === Qt.Key_Tab) {
            editorState.setCurrent((editorState.currentIndex + 1) % editorState.tabs.length)
            e.accepted = true
        }
    }
    focus: true
}
