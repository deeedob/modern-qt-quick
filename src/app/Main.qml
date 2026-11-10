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

    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: Theme.spacing / 2
            anchors.rightMargin: Theme.spacing / 2

            ToolButton {
                icon.source: Theme.icon("arrow-left")
                text: qsTr("Back")
                visible: stack.depth > 1
                onClicked: stack.pop()
            }

            Label {
                Layout.fillWidth: true
                Layout.leftMargin: Theme.spacing / 2
                text: window.title
                font.pixelSize: 16
                font.weight: Font.DemiBold
                elide: Text.ElideRight
            }

            ToolButton {
                icon.source: Theme.icon("languages")
                text: qsTr("Switch language")
                onClicked: AppController.language = AppController.language === "de" ? "en" : "de"
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
            onOpened: stack.push(canvasPage)
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
}
