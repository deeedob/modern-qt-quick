import QtQuick
import QtTest
import AppStyle

TestCase {
    id: testCase

    name: "ToolButton"
    width: 200
    height: 200
    // TestCase is invisible by default; rendering and mouse input need a visible parent.
    visible: true
    when: windowShown

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
        const button = createTemporaryObject(buttonComponent, testCase);
        const toolTip = findChild(button, "toolTip");
        verify(!toolTip.visible);

        mouseMove(button);

        tryVerify(() => toolTip.visible);
        compare(toolTip.text, "Add");
    }
}
