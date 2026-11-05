// SPDX-FileCopyrightText: 2026 Dennis Oberst
// SPDX-License-Identifier: MIT-0

import QtQuick
import QtTest
import App.Frontend.Ui

TestCase {
    id: testCase

    name: "ProgressRing"
    width: 200
    height: 200
    // TestCase is invisible by default; rendering and mouse input need a visible parent.
    visible: true
    when: windowShown

    Component {
        id: ringComponent

        ProgressRing {
            width: 100
            height: 100
            thickness: 20
            color: "red"
            trackColor: "blue"
        }
    }

    function test_progressIsClamped(): void {
        const ring = createTemporaryObject(ringComponent, testCase);
        ring.progress = 1.5;
        compare(ring.progress, 1.0);
        ring.progress = -1;
        compare(ring.progress, 0.0);
    }

    function test_drawsClockwiseFromTwelveOClock(): void {
        // The software renderer (e.g. QT_QPA_PLATFORM=offscreen) does not draw custom geometry.
        if (GraphicsInfo.api === GraphicsInfo.Software)
            skip("custom scene graph geometry needs a GPU backend; run without QT_QPA_PLATFORM=offscreen");

        const ring = createTemporaryObject(ringComponent, testCase, {
            progress: 0.25
        });
        waitForRendering(ring);
        const image = grabImage(ring);
        const scale = image.width / ring.width;
        // Both points lie in the middle of the 20 px band (radius 40) at 45 degrees.
        verify(Qt.colorEqual(image.pixel(78 * scale, 22 * scale), "red"), "top-right quarter shows progress");
        verify(Qt.colorEqual(image.pixel(22 * scale, 22 * scale), "blue"), "top-left quarter shows the track");
    }
}
