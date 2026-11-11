import QtQuick
import QtTest
import App.Controls
import "../../src/canvas" as Canvas

TestCase {
    id: testCase
    name: "SketchTitle"
    width: 350
    height: 120
    visible: true
    when: windowShown
    property string renamedTo: ""

    function init(): void {
        failOnWarning(/.?/);
        testCase.renamedTo = "";
    }

    Component {
        id: fixtureComponent
        Control {
            width: 350
            height: 120
            font.family: Theme.fontFamily
            Canvas.SketchTitle {
                objectName: "title"
                width: 200
                name: "Original"
                onRenameRequested: name => testCase.renamedTo = name
            }
            ToolButton {
                objectName: "other"
                x: 240
                text: "Other"
                icon.source: Theme.icon("info")
            }
        }
    }

    function test_doubleClickAndCommitOnFocusLoss(): void {
        const fixture = createTemporaryObject(fixtureComponent, testCase);
        const title = findChild(fixture, "title");
        const label = findChild(title, "sketchTitleLabel");
        const field = findChild(title, "sketchNameField");
        compare(label.width, 200);
        compare(label.height, 36);
        verify(label.visible);
        mouseDoubleClickSequence(label);
        verify(title.editing);
        tryVerify(() => field.activeFocus);
        field.text = "Renamed";
        mouseClick(findChild(fixture, "other"));
        compare(testCase.renamedTo, "Renamed");
        verify(!field.visible);
    }

    function test_escapeCancelsRename(): void {
        const fixture = createTemporaryObject(fixtureComponent, testCase);
        const title = findChild(fixture, "title");
        const field = findChild(title, "sketchNameField");
        mouseDoubleClickSequence(findChild(title, "sketchTitleLabel"));
        verify(title.editing);
        tryVerify(() => field.activeFocus);
        field.text = "Discard";
        keyClick(Qt.Key_Escape);
        compare(testCase.renamedTo, "");
        verify(!field.visible);
    }
}
