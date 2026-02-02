import QtQuick

QtObject {
    property int currentIndex: 0

    // your “tabs model”: later you can replace this with a C++ model
    property var tabs: [
        { id: "input",   title: "Input",   qml: "panels/InputPanel.qml" },
        { id: "scene",   title: "Scene",   qml: "panels/ScenePanel.qml" },
        { id: "props",   title: "Props",   qml: "panels/PropertiesPanel.qml" },
        { id: "anim",    title: "Anim",    qml: "panels/AnimationPanel.qml" }
    ]

    function setCurrent(i) {
        if (i < 0 || i >= tabs.length) return
        currentIndex = i
    }
}
