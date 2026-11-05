import QtQuick
import QtTest
import App.Canvas

TestCase {
    id: testCase

    name: "StrokeItem"
    width: 200
    height: 200
    // TestCase is invisible by default; rendering and mouse input need a visible parent.
    visible: true
    when: windowShown

    Component {
        id: itemComponent

        StrokeItem {
            anchors.fill: parent
        }
    }

    SignalSpy {
        id: finishedSpy

        signalName: "strokeFinished"
    }

    function init(): void {
        finishedSpy.clear();
    }

    function test_dragReportsTheDrawnPoints(): void {
        const item = createTemporaryObject(itemComponent, testCase);
        finishedSpy.target = item;

        mousePress(item, 10, 10);
        mouseMove(item, 50, 40, -1, Qt.LeftButton);
        mouseMove(item, 90, 80, -1, Qt.LeftButton);
        mouseRelease(item, 90, 80);

        compare(finishedSpy.count, 1);
        const points = finishedSpy.signalArguments[0][0];
        compare(points.length, 3);
        compare(points[0], Qt.point(10, 10));
        compare(points[2], Qt.point(90, 80));
    }

    function test_clickReportsOnePoint(): void {
        const item = createTemporaryObject(itemComponent, testCase);
        finishedSpy.target = item;

        mouseClick(item, 30, 30);

        compare(finishedSpy.count, 1);
        compare(finishedSpy.signalArguments[0][0].length, 1);
    }
}
