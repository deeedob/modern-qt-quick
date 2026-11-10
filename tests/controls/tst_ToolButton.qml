import QtQuick
import QtTest
import App.Controls

TestCase {
    id: testCase

    name: "ToolButton"
    width: 200
    height: 200
    // TestCase is invisible by default; rendering and mouse input need a visible parent.
    visible: true
    when: windowShown

    function init(): void {
        failOnWarning(/.?/);
    }

    Component {
        id: buttonComponent

        ToolButton {
            icon.source: Theme.icon("plus")
            text: "Add"
            // The offscreen test platform turns hover effects off by default.
            hoverEnabled: true
        }
    }

    function test_hoverShowsTheTextAsToolTip(): void {
        // Qt's offscreen macOS font fallback requests an unavailable generic family.
        ignoreWarning(/Populating font family aliases took .*Replace uses of missing font family "Sans Serif".*/);
        const button = createTemporaryObject(buttonComponent, testCase);
        const toolTip = findChild(button, "toolTip");
        verify(!toolTip.visible);

        mouseMove(button);

        tryVerify(() => toolTip.visible);
        compare(toolTip.text, "Add");
    }
}
