pragma Singleton

import QtQuick

// Design tokens: the one place for colors, sizes and type. Basic's controls get them via the palette.
QtObject {
    readonly property bool dark: Application.styleHints.colorScheme === Qt.ColorScheme.Dark

    readonly property color window: dark ? "#17171c" : "#f6f6f8"
    readonly property color surface: dark ? "#22222a" : "#ffffff"
    readonly property color hover: dark ? "#2d2d36" : "#ececf1"
    readonly property color pressed: dark ? "#393944" : "#dedee6"
    readonly property color border: dark ? "#3a3a45" : "#d6d6de"
    readonly property color text: dark ? "#ececf1" : "#1c1c22"
    readonly property color mutedText: dark ? "#9b9ba8" : "#6c6c78"
    readonly property color accent: "#5b5bd6"
    readonly property color accentPressed: "#4a4ac0"
    readonly property color accentText: "#ffffff"

    readonly property int spacing: 12
    readonly property int radius: 8
    readonly property int controlHeight: 36
    readonly property int iconSize: 20
    readonly property string fontFamily: interFont.name

    function icon(name: string): url {
        return Qt.resolvedUrl(`icons/${name}.svg`);
    }

    readonly property FontLoader interFont: FontLoader {
        source: Qt.resolvedUrl("fonts/InterVariable.ttf")
    }
}
