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

    width: 960
    height: 680
    visible: true
    title: Application.displayName

    // demo:begin
    onClosing: function (close) {
        if (!sketchError.discardUnsavedChanges && window.sketchbook.hasUnsavedChanges) {
            close.accepted = false;
            sketchError.closeAfterSave = true;
            sketchError.message = qsTr("Some sketches have not been saved.");
            sketchError.retryable = true;
            sketchError.open();
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
            sketchError.operation = operation;
            sketchError.source = source;
            sketchError.message = message;
            sketchError.retryable = retryable || window.sketchbook.hasUnsavedChanges;
            sketchError.open();
        }

        function onUnsavedChangesChanged(): void {
            if (!window.sketchbook.hasUnsavedChanges) {
                sketchError.retryable = false;
                if (sketchError.closeAfterSave)
                    window.close();
            }
        }
    }

    Dialog {
        id: sketchError

        property string message: ""
        property int operation: 0
        property string source: ""
        property bool retryable: false
        property bool closeAfterSave: false
        property bool discardUnsavedChanges: false

        title: qsTr("Sketchbook error")
        standardButtons: sketchError.closeAfterSave ? Dialog.Retry | Dialog.Discard | Dialog.Cancel :
                                                      sketchError.retryable ? Dialog.Retry
                                                                              | Dialog.Cancel :
                                                                              Dialog.Ok
        onAccepted: {
            if (sketchError.retryable) {
                sketchError.retryable = false;
                window.sketchbook.retryFailedSaves();
            }
        }
        onRejected: sketchError.closeAfterSave = false
        onDiscarded: {
            sketchError.closeAfterSave = false;
            sketchError.discardUnsavedChanges = true;
            window.close();
        }
        contentItem: Label {
            text: sketchError.message
            wrapMode: Text.Wrap
        }
    }
    // demo:end
}
