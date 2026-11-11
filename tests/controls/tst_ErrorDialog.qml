pragma ComponentBehavior: Bound

import QtQuick
import QtTest
import App.Controls

TestCase {
    name: "ErrorDialog"
    width: 600
    height: 400
    visible: true
    when: windowShown

    ErrorDialog {
        id: errorDialog

        title: "Save"
        source: "Field <notes>"
        message: "Could not save sketch: disk is full"
        standardButtons: Dialog.Retry | Dialog.Cancel
    }

    SignalSpy {
        id: acceptedSpy
        target: errorDialog
        signalName: "accepted"
    }

    SignalSpy {
        id: rejectedSpy
        target: errorDialog
        signalName: "rejected"
    }

    SignalSpy {
        id: discardedSpy
        target: errorDialog
        signalName: "discarded"
    }

    function init(): void {
        failOnWarning(/.*/);
        errorDialog.close();
        errorDialog.standardButtons = Dialog.Retry | Dialog.Cancel;
        acceptedSpy.clear();
        rejectedSpy.clear();
        discardedSpy.clear();
    }

    function cleanup(): void {
        errorDialog.close();
    }

    function test_presentsCallerTitleSourceAndMessage(): void {
        errorDialog.open();

        verify(errorDialog.visible);
        compare(errorDialog.title, "Save");
        compare(findChild(errorDialog, "sourceLabel").text, "Field <notes>");
        compare(findChild(errorDialog, "sourceLabel").textFormat, Text.PlainText);
        compare(findChild(errorDialog, "messageLabel").text, "Could not save sketch: disk is full");
        compare(findChild(errorDialog, "messageLabel").textFormat, Text.PlainText);
        verify(errorDialog.standardButton(Dialog.Retry).visible);
        verify(errorDialog.standardButton(Dialog.Cancel).visible);
        verify(!errorDialog.standardButton(Dialog.Discard));

        mouseClick(errorDialog.standardButton(Dialog.Retry));
        compare(acceptedSpy.count, 1);
    }

    function test_inheritedDiscardSignalAndButton(): void {
        errorDialog.standardButtons = Dialog.Retry | Dialog.Discard | Dialog.Cancel;
        errorDialog.open();

        verify(errorDialog.standardButton(Dialog.Retry).visible);
        const discardButton = errorDialog.standardButton(Dialog.Discard);
        verify(discardButton.visible);
        verify(errorDialog.standardButton(Dialog.Cancel).visible);
        tryVerify(() => discardButton.width > 0 && discardButton.height > 0);
        verify(waitForPolish(discardButton.Window.window));
        mouseClick(discardButton, discardButton.width / 2, discardButton.height / 2);
        compare(discardedSpy.count, 1);
    }

    function test_inheritedCancelSignal(): void {
        errorDialog.open();
        errorDialog.reject();
        compare(rejectedSpy.count, 1);
    }
}
