pragma Singleton

import QtQuick

QtObject {
    property bool dark: Application.styleHints.colorScheme === Qt.ColorScheme.Dark

    readonly property color window: dark ? "#1e2326" : "#f2efdf"
    readonly property color surface: dark ? "#272e33" : "#fffbef"
    readonly property color hover: dark ? "#2e383c" : "#f8f5e4"
    readonly property color pressed: dark ? "#374145" : "#edeada"
    readonly property color border: dark ? "#495156" : "#bec5b2"
    readonly property color text: dark ? "#d3c6aa" : "#5c6a72"
    readonly property color mutedText: dark ? "#859289" : "#708089"
    readonly property color accent: dark ? "#7fbbb3" : "#3a94c5"
    readonly property color accentPressed: Qt.darker(accent, 1.15)
    readonly property color accentText: "#1e2326"
    readonly property color overlay: dark ? "#1e2326" : "#5c6a72"
    readonly property color overlayText: dark ? "#d3c6aa" : "#fffbef"

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
