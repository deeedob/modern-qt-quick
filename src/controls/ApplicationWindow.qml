import QtQuick
import QtQuick.Templates as T

// The palette reaches every control, including those taken from Basic, so they follow the theme too.
T.ApplicationWindow {
    color: Theme.window
    font.family: Theme.fontFamily
    font.pixelSize: 14

    palette {
        window: Theme.window
        windowText: Theme.text
        base: Theme.surface
        alternateBase: Theme.hover
        text: Theme.text
        button: Theme.surface
        buttonText: Theme.text
        mid: Theme.border
        dark: Theme.mutedText
        highlight: Theme.accent
        highlightedText: Theme.accentText
        placeholderText: Theme.mutedText
        toolTipBase: Theme.text
        toolTipText: Theme.window
    }
}
