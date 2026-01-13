import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects

Control {
    id: root

    property alias text: input.text
    property alias input: input
    property alias placeholderText: placeholder.text
    property alias maximumLength: input.maximumLength

    property Item nextItem: null
    property Item prevItem: null
    property bool isLast: false
    signal submitted()

    property int inputRadius: 18
    property int inputPadding: 14

    property real inputWidth: 260
    property real inputHeight: 56
    property real placeholderTextSize: 10
    property real inputTextSize: 12

    property bool isPassword: false
    property bool isDate: false
    property bool borderShadow: true

    property color backgroundColor: "#0c2a46"
    property color borderColor: "#041326"
    property color inputColor: "#FFFFFF"
    property color placeholderColor: "#3ee6ff"

    implicitWidth: inputWidth
    implicitHeight: inputHeight

    activeFocusOnTab: true
    focusPolicy: Qt.StrongFocus

    function focusInput() {
        input.forceActiveFocus()
    }

    background: Rectangle {
        id: styling
        radius: root.inputRadius
        color: root.backgroundColor
        border.color: root.borderColor
        border.width: 1
        opacity: 0.95

        layer.enabled: borderShadow
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

    contentItem: Item {
        anchors.fill: parent
        anchors.margins: inputPadding

        TextInput {
            id: input
            anchors.fill: parent
            font.pixelSize: root.inputTextSize
            clip: true
            focus: true
            color: root.inputColor
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            cursorVisible: activeFocus
            echoMode: isPassword ? TextInput.Password : TextInput.Normal

            activeFocusOnTab: true

            inputMethodHints: root.isLast ? Qt.ImhNoPredictiveText : Qt.ImhNoPredictiveText

            Keys.onTabPressed: (event) => {
                if (root.nextItem) {
                    if (root.nextItem.focusInput) root.nextItem.focusInput()
                    else root.nextItem.forceActiveFocus()
                    event.accepted = true
                }
            }
            Keys.onBacktabPressed: (event) => {
                if (root.prevItem) {
                    if (root.prevItem.focusInput) root.prevItem.focusInput()
                    else root.prevItem.forceActiveFocus()
                    event.accepted = true
                }
            }

            Keys.onReturnPressed: (event) => {
                if (!root.isLast && root.nextItem) {
                    if (root.nextItem.focusInput) root.nextItem.focusInput()
                    else root.nextItem.forceActiveFocus()
                } else {
                    root.submitted()
                    input.focus = false
                }
                event.accepted = true
            }
            Keys.onEnterPressed: Keys.onReturnPressed
        }

        Text {
            id: placeholder
            anchors.fill: parent
            font.pixelSize: root.placeholderTextSize
            color: root.placeholderColor
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            visible: input.text.length === 0 && !input.activeFocus
        }
    }

    onActiveFocusChanged: {
        if (activeFocus) input.forceActiveFocus()
    }

    // Binding {
    //     target: setText
    //     value: input.text
    // }
}
