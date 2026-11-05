// SPDX-FileCopyrightText: 2026 Dennis Oberst
// SPDX-License-Identifier: MIT-0

import QtQuick
import QtTest
import App.Frontend.Ui

TestCase {
    id: testCase

    name: "IconButton"
    width: 100
    height: 100
    // TestCase is invisible by default; rendering and mouse input need a visible parent.
    visible: true
    when: windowShown

    Component {
        id: buttonComponent

        IconButton {
            iconName: "plus"
            toolTip: "Add"
        }
    }

    SignalSpy {
        id: clickedSpy

        signalName: "clicked"
    }

    function test_resolvesTheIconByName(): void {
        const button = createTemporaryObject(buttonComponent, testCase);
        verify(button.icon.source.toString().endsWith("icons/plus.svg"));
    }

    function test_toolTipDoublesAsAccessibleText(): void {
        const button = createTemporaryObject(buttonComponent, testCase);
        compare(button.text, "Add");
    }

    function test_emitsClicked(): void {
        const button = createTemporaryObject(buttonComponent, testCase);
        clickedSpy.target = button;
        mouseClick(button);
        compare(clickedSpy.count, 1);
    }
}
