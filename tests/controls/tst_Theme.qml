import QtQuick
import QtTest
import App.Controls

TestCase {
    id: testCase
    name: "Theme"

    Component {
        id: sliderComponent
        Slider {}
    }
    Component {
        id: switchComponent
        Switch {}
    }

    function init(): void {
        failOnWarning(/.?/);
    }

    function cleanup(): void {
        Theme.dark = false;
    }

    function test_sliderAndSwitchFollowTheme(): void {
        const slider = createTemporaryObject(sliderComponent, testCase);
        const toggle = createTemporaryObject(switchComponent, testCase);
        for (const dark of [false, true]) {
            Theme.dark = dark;
            for (const control of [slider, toggle]) {
                compare(control.palette.midlight, Theme.border);
                compare(control.palette.dark, Theme.accent);
                compare(control.palette.window, Theme.surface);
                compare(control.palette.light, Theme.pressed);
            }
        }
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
