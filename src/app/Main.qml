pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import AppStyle
import App.Canvas
import App.Sketchbook

// main() injects the dependencies as required properties; only AppController is a singleton.
ApplicationWindow {
    id: window

    required property SketchbookViewModel sketchbook
    required property CanvasStore canvas

    width: 960
    height: 680
    visible: true
    title: AppController.displayName

    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: Theme.spacing / 2
            anchors.rightMargin: Theme.spacing / 2

            ToolButton {
                icon.source: Theme.icon("arrow-left")
                text: qsTr("Back to the sketchbook")
                visible: stack.depth > 1
                onClicked: stack.pop()
            }

            Label {
                Layout.fillWidth: true
                Layout.leftMargin: Theme.spacing / 2
                text: stack.depth > 1 ? window.sketchbook.openName : AppController.displayName
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
        initialItem: SketchbookPage {
            viewModel: window.sketchbook
            onOpened: stack.push(canvasPage)
        }
    }

    Component {
        id: canvasPage

        CanvasPage {
            store: window.canvas
        }
    }

    AboutDialog {
        id: about
    }
}
