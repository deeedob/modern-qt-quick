import QtQuick
import QtTest
import App.Controls

TestCase {
    name: "Theme"

    function init(): void {
        failOnWarning(/.?/);
    }

    function cleanup(): void {
        Theme.dark = false;
    }

    function test_hardLightPalette(): void {
        Theme.dark = false;
        verify(Theme.fontFamily !== "");
        verify(Qt.colorEqual(Theme.window, "#f2efdf"));
        verify(Qt.colorEqual(Theme.surface, "#fffbef"));
        verify(Qt.colorEqual(Theme.border, "#bec5b2"));
        verify(Qt.colorEqual(Theme.accent, "#3a94c5"));
        verify(Qt.colorEqual(Theme.accentText, "#1e2326"));
        verify(Qt.colorEqual(Theme.overlayText, "#fffbef"));
    }

    function test_hardDarkPalette(): void {
        Theme.dark = true;
        verify(Qt.colorEqual(Theme.window, "#1e2326"));
        verify(Qt.colorEqual(Theme.surface, "#272e33"));
        verify(Qt.colorEqual(Theme.border, "#495156"));
        verify(Qt.colorEqual(Theme.accent, "#7fbbb3"));
        verify(Qt.colorEqual(Theme.accentText, "#1e2326"));
        verify(Qt.colorEqual(Theme.overlayText, "#d3c6aa"));
    }
}
