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

    Component {
        id: scrollableComponent

        Flickable {
            property alias item: strokeItem

            anchors.fill: parent
            contentWidth: 400
            contentHeight: 400

            StrokeItem {
                id: strokeItem

                width: 400
                height: 400
            }
        }
    }

    function test_dragDrawsInsteadOfScrolling(): void {
        const flickable = createTemporaryObject(scrollableComponent, testCase);
        finishedSpy.target = flickable.item;

        mousePress(flickable, 100, 150);
        for (let y = 140; y >= 20; y -= 20)
            mouseMove(flickable, 100, y, -1, Qt.LeftButton);
        mouseRelease(flickable, 100, 20);

        compare(flickable.contentY, 0);
        compare(finishedSpy.count, 1);
        verify(finishedSpy.signalArguments[0][0].length > 2);
    }

    function test_clickReportsOnePoint(): void {
        const item = createTemporaryObject(itemComponent, testCase);
        finishedSpy.target = item;

        mouseClick(item, 30, 30);

        compare(finishedSpy.count, 1);
        compare(finishedSpy.signalArguments[0][0].length, 1);
    }
}
