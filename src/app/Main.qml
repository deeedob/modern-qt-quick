pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import App.Controls
import App.Canvas // demo
import App.Sketchbook // demo

// main() injects the dependencies as required properties; only AppController is a singleton.
ApplicationWindow {
    id: window

    required property SketchbookViewModel sketchbook // demo
    required property CanvasStore canvas // demo

    // demo:begin
    property bool hasFailedChanges: false
    property bool closeAfterSave: false
    property bool discardUnsavedChanges: false
    property int errorOperation: 0
    property string errorSource: ""
    property string errorMessage: ""
    property string notification: ""

    function operationTitle(operation: int): string {
        switch (operation) {
        case SketchbookViewModel.Operation.Open:
            return qsTr("Open sketch");
        case SketchbookViewModel.Operation.Rename:
            return qsTr("Rename sketch");
        case SketchbookViewModel.Operation.Duplicate:
            return qsTr("Duplicate sketch");
        case SketchbookViewModel.Operation.Save:
            return qsTr("Save sketch");
        case SketchbookViewModel.Operation.Remove:
            return qsTr("Delete sketch");
        default:
            return qsTr("Error");
        }
    }
    // demo:end

    width: 960
    height: 680
    visible: true
    title: Application.displayName

    // demo:begin
    onClosing: function (close) {
        if (!window.discardUnsavedChanges && window.sketchbook.hasUnsavedChanges) {
            close.accepted = false;
            window.closeAfterSave = true;
            if (window.hasFailedChanges)
                errorDialog.open();
        }
    }
    // demo:end

    Binding {
        target: Theme
        property: "dark"
        value: AppController.darkTheme
    }

    LoggingCategory {
        id: navigationLog
        name: "app.navigation"
    }

    // demo:begin
    LoggingCategory {
        id: storageLog
        name: "app.sketchbook"
    }
    // demo:end

    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: Theme.spacing / 2
            anchors.rightMargin: Theme.spacing / 2

            ToolButton {
                icon.source: Theme.icon("arrow-left")
                text: qsTr("Back")
                visible: stack.depth > 1
                onClicked: {
                    console.info(navigationLog, "Returning to sketchbook");
                    stack.pop();
                }
            }

            Label {
                Layout.fillWidth: true
                Layout.leftMargin: Theme.spacing / 2
                text: window.title
                font.pixelSize: 16
                font.weight: Font.DemiBold
                elide: Text.ElideRight
                visible: stack.depth === 1 // demo
            }

            // demo:begin
            SketchTitle {
                Layout.fillWidth: true
                visible: stack.depth > 1
                name: window.sketchbook.openName
                onRenameRequested: name => window.sketchbook.renameOpen(name)
            }
            // demo:end

            // demo:begin
            Label {
                visible: stack.depth > 1
                text: qsTr("%n stroke(s)", "", window.canvas.strokeCount) + " · " + qsTr("%n point(s)",
                                                                                         "", window.canvas.pointCount)
                color: Theme.mutedText
            }
            // demo:end

            ToolButton {
                icon.source: Theme.icon(AppController.darkTheme ? "sun" : "moon")
                text: AppController.darkTheme ? qsTr("Switch to light theme") : qsTr(
                                                    "Switch to dark theme")
                onClicked: AppController.darkTheme = !AppController.darkTheme
            }

            LanguageSelector {
                language: AppController.language
                onLanguageSelected: language => AppController.language = language
            }

            ToolButton {
                icon.source: Theme.icon("info")
                text: qsTr("About")
                onClicked: about.open()
            }
        }
    }

    StackView {
        id: stack

        anchors.fill: parent
        // demo:begin
        initialItem: SketchbookPage {
            viewModel: window.sketchbook
            onOpened: {
                console.info(navigationLog, "Opening canvas");
                stack.push(canvasPage);
            }
        }
        // demo:end
    }

    // demo:begin
    Component {
        id: canvasPage

        CanvasPage {
            store: window.canvas
        }
    }

    Binding {
        window.title: window.sketchbook.openName
        when: stack.depth > 1
    }
    // demo:end

    AboutDialog {
        id: about
    }

    // demo:begin
    Connections {
        target: window.sketchbook

        function onErrorOccurred(operation: int, source: string, message: string, retryable: bool):
        void {
            console.error(storageLog, "Sketchbook operation " + operation + " failed for " + source
                          + ": " + message);
            if (retryable) {
                window.hasFailedChanges = true;
                window.errorOperation = operation;
                window.errorSource = source;
                window.errorMessage = message;
                errorDialog.open();
            } else {
                window.notification = message;
                errorNotification.open();
                notificationTimer.restart();
            }
        }

        function onUnsavedChangesChanged(): void {
            if (!window.sketchbook.hasUnsavedChanges) {
                window.hasFailedChanges = false;
                if (window.closeAfterSave)
                    window.close();
            }
        }
    }

    ErrorDialog {
        id: errorDialog
        objectName: "storageErrorDialog"

        title: window.operationTitle(window.errorOperation)
        source: window.errorSource
        message: window.errorMessage
        standardButtons: window.closeAfterSave ? Dialog.Retry | Dialog.Discard | Dialog.Cancel :
                                                 Dialog.Retry | Dialog.Cancel

        onAccepted: window.sketchbook.retryFailedSaves()
        onRejected: window.closeAfterSave = false
        onDiscarded: {
            window.closeAfterSave = false;
            window.discardUnsavedChanges = true;
            window.close();
        }

        onClosed: {
            if (!window.closeAfterSave)
                window.discardUnsavedChanges = false;
        }
    }

    Popup {
        id: errorNotification

        x: (window.width - width) / 2
        y: window.height - height - Theme.spacing * 2
        width: Math.min(440, window.width - Theme.spacing * 2)
        padding: Theme.spacing
        closePolicy: Popup.NoAutoClose

        contentItem: Label {
            text: window.notification
            wrapMode: Text.Wrap
        }

        background: Rectangle {
            radius: Theme.radius
            color: Theme.surface
            border.color: Theme.border
        }
    }

    Timer {
        id: notificationTimer

        interval: 5000
        onTriggered: errorNotification.close()
    }
    // demo:end
}
