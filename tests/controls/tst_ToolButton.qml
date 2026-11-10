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

    Component {
        id: focusComponent
        Item {
            TextField {
                objectName: "field"
                width: 100
            }
            ToolButton {
                objectName: "button"
                x: 120
                text: "Action"
                icon.source: Theme.icon("plus")
            }
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

    function test_clickTakesFocusFromEditor(): void {
        const fixture = createTemporaryObject(focusComponent, testCase);
        const field = findChild(fixture, "field");
        const button = findChild(fixture, "button");
        field.forceActiveFocus();
        verify(field.activeFocus);
        mouseClick(button);
        tryVerify(() => !field.activeFocus);
    }
}
