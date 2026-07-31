pragma Singleton
import QtQuick

Item {
    id: style

    width: 0
    height: 0
    visible: false

    enum ThemeMode {
        System,
        Light,
        Dark
    }

    property int themeMode: Style.System

    readonly property bool darkMode: {
        switch (themeMode) {
        case Style.Light:
            return false

        case Style.Dark:
            return true

        case Style.System:
        default:
            return Qt.styleHints.colorScheme === Qt.Dark
        }
    }

    property alias fontAwesome: fontAwesomeLoader.name

    FontLoader {
        id: fontAwesomeLoader
        source: "qrc:/qt/qml/GenesisX/Assets/fontawesome.ttf"
    }

    // main colors
    readonly property color backgroundPrimary: darkMode ? "#191a1c" : "#ffffff"
    readonly property color backgroundSecondary: darkMode ? "#252629" : "#ffffff"
    readonly property color chromeColorPrimary: darkMode ? "#26282c" : "#f3f3f3"
    readonly property color shadowColorPrimary: "#90000000"

    // borders
    readonly property color borderColorPrimary: darkMode === true ? "#757575" : "#ababab"
    readonly property color borderColorHoverPrimary: darkMode ? "#4a4d52" : "#aeb3ba"
    readonly property color borderColorSecondary: darkMode ? "#4b4d51" : "#ababab"
    readonly property color borderColorHoverSecondary: darkMode ? "#6b6d72" : "#aeb3ba"
    readonly property color borderColorPressedSecondary: darkMode ? "#85878c" : "#aeb3ba"

    // common dimensions
    readonly property int radiusSmall: 4
    readonly property int radiusMedium: 6
    readonly property int radiusLarge: 8

    readonly property int spacingXSmall: 4
    readonly property int spacingSmall: 6
    readonly property int spacingMedium: 12
    readonly property int spacingLarge: 20

    // Text
    readonly property color textPrimary: darkMode ? "#ffffff" : "#202124"
    readonly property color textSecondary: darkMode ? "#c3c5cb" : "#6c707e"

    // frame buttons
    readonly property color buttonFrameHoverPrimary: darkMode ? "#3a3d42" : "#dbdbdb"
}
