pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import AppStyle

// A Flux view: it shows the store's state and turns input into actions, never editing the sketch.
Page {
    id: root

    required property CanvasStore store
    readonly property list<color> inks: ["#1c1c22", "#e5484d", "#30a46c", "#0090ff"]
    property color penColor: inks[0]

    component Swatch: AbstractButton {
        id: swatch

        required property color ink

        implicitWidth: 28
        implicitHeight: 28
        background: Rectangle {
            radius: width / 2
            color: swatch.ink
            border.width: swatch.checked ? 3 : 1
            border.color: swatch.checked ? Theme.accent : Theme.border
        }
    }

    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: Theme.spacing
            anchors.rightMargin: Theme.spacing / 2
            spacing: Theme.spacing / 2

            Repeater {
                model: root.inks

                delegate: Swatch {
                    required property color modelData

                    ink: modelData
                    text: qsTr("Ink %1").arg(modelData)
                    checked: Qt.colorEqual(root.penColor, ink)
                    onClicked: root.penColor = ink
                }
            }

            Switch {
                Layout.leftMargin: Theme.spacing
                text: qsTr("Snap shapes")
                checked: root.store.snapShapes
                onToggled: root.store.snapShapes = checked
            }

            Item {
                Layout.fillWidth: true
            }

            ToolButton {
                icon.source: Theme.icon("undo-2")
                text: qsTr("Undo")
                enabled: root.store.canUndo
                onClicked: root.store.undo()
            }

            ToolButton {
                icon.source: Theme.icon("redo-2")
                text: qsTr("Redo")
                enabled: root.store.canRedo
                onClicked: root.store.redo()
            }

            ToolButton {
                icon.source: Theme.icon("eraser")
                text: qsTr("Clear")
                enabled: !root.store.empty
                onClicked: root.store.clear()
            }
        }
    }

    Rectangle {
        anchors.fill: parent
        anchors.margins: Theme.spacing
        radius: Theme.radius
        color: "#fdfdfb" // paper stays light in dark mode, so saved ink colors keep working
        border.color: Theme.border
        clip: true

        StrokeItem {
            anchors.fill: parent
            store: root.store
            penColor: root.penColor
            onStrokeFinished: points => root.store.addStroke(points, root.penColor)
        }

        Label {
            anchors.centerIn: parent
            visible: root.store.empty
            text: qsTr("Draw here. Circles, rectangles and triangles snap into shape.")
            color: "#8a8a94"
        }
    }

    Shortcut {
        sequences: [StandardKey.Undo]
        onActivated: root.store.undo()
    }

    Shortcut {
        sequences: [StandardKey.Redo]
        onActivated: root.store.redo()
    }
}
