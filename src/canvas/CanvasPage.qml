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

    // Zoom belongs to this view, not to the sketch, so it lives here instead of in the store.
    property real zoom: 1.0

    // Clamps directly: Flickable.returnToBounds() would start an animation on every pinch step.
    function scrollTo(x: real, y: real): void {
        viewport.contentX = Math.max(0, Math.min(x, viewport.contentWidth - viewport.width));
        viewport.contentY = Math.max(0, Math.min(y, viewport.contentHeight - viewport.height));
    }

    // Keeps the paper point under `position`, in viewport coordinates, where it is.
    function zoomBy(factor: real, position: point): void {
        const next = Math.min(Math.max(root.zoom * factor, 0.25), 4);
        const ratio = next / root.zoom;
        root.zoom = next;
        root.scrollTo((viewport.contentX + position.x) * ratio - position.x, (viewport.contentY + position.y) * ratio - position.y);
    }

    // One finger or the mouse draws. The wheel, the trackpad, the scroll bars and two fingers
    // move the paper; Ctrl+wheel and pinching zoom it.
    Flickable {
        id: viewport

        anchors.fill: parent
        anchors.margins: Theme.spacing
        clip: true
        contentWidth: paper.width * root.zoom
        contentHeight: paper.height * root.zoom
        boundsBehavior: Flickable.StopAtBounds
        ScrollBar.horizontal: ScrollBar {}
        ScrollBar.vertical: ScrollBar {}

        Rectangle {
            id: paper

            width: 1600
            height: 1200
            scale: root.zoom
            transformOrigin: Item.TopLeft
            color: "#fdfdfb" // paper stays light in dark mode, so saved ink colors keep working
            border.color: Theme.border
            // StrokeItem keeps the pointer while drawing, even past the edge: hide what is off the paper.
            clip: true

            StrokeItem {
                anchors.fill: parent
                store: root.store
                penColor: root.penColor
                onStrokeFinished: points => root.store.addStroke(points, root.penColor)
            }
        }
    }

    // Outside the Flickable, so the handlers see positions that do not move while zooming.
    Item {
        anchors.fill: viewport

        PinchHandler {
            target: null
            onScaleChanged: delta => root.zoomBy(delta, centroid.position)
            // A trackpad pinch only zooms; its first delta is the whole pointer position (Qt 6.13).
            onTranslationChanged: delta => {
                if (centroid.device.type === PointerDevice.TouchScreen)
                    root.scrollTo(viewport.contentX - delta.x, viewport.contentY - delta.y);
            }
        }

        WheelHandler {
            target: null
            acceptedModifiers: Qt.ControlModifier
            onWheel: event => root.zoomBy(Math.pow(2, event.angleDelta.y / 720), point.position)
        }
    }

    Label {
        anchors.centerIn: viewport
        visible: root.store.empty
        text: qsTr("Draw here. Circles, rectangles and triangles snap into shape.")
        color: "#8a8a94"
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
