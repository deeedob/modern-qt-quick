pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import App.Controls

// A Flux view: it shows the store's state and turns input into actions, never editing the sketch.
Page {
    id: root

    required property CanvasStore store
    readonly property list<color> inks: ["#1c1c22", "#e5484d", "#30a46c", "#0090ff"]
    property color penColor: inks[0]
    property real penWidth: 4.0
    property real manualZoom: 0.0
    readonly property real zoom: manualZoom > 0 ? manualZoom : fitZoom
    property bool zoomActive: false
    readonly property real fitZoom: Math.min(viewport.width / paper.width, viewport.height
                                             / paper.height)
    readonly property real minimumZoom: fitZoom * 0.85

    // Clamps directly: Flickable.returnToBounds() would start an animation on every pinch step.
    function scrollTo(x: real, y: real): void {
        viewport.contentX = Math.max(0, Math.min(x, viewport.contentWidth - viewport.width));
        viewport.contentY = Math.max(0, Math.min(y, viewport.contentHeight - viewport.height));
    }

    // Keeps the paper point under `position`, in viewport coordinates, where it is.
    function zoomBy(factor: real, position: point): void {
        const next = Math.min(Math.max(root.zoom * factor, root.minimumZoom), 4);
        const ratio = next / root.zoom;
        root.manualZoom = next;
        root.scrollTo((viewport.contentX + position.x) * ratio - position.x, (viewport.contentY
                                                                              + position.y) * ratio
                      - position.y);
    }

    function fitPage(): void {
        root.manualZoom = 0;
        root.scrollTo((viewport.contentWidth - viewport.width) / 2, (viewport.contentHeight
                                                                     - viewport.height) / 2);
    }

    component Swatch: AbstractButton {
        id: swatch

        required property color ink
        signal picked(color ink)

        implicitWidth: 28
        implicitHeight: 28
        onClicked: swatch.picked(swatch.ink)
        background: Rectangle {
            radius: width / 2
            color: swatch.ink
            border.width: swatch.checked ? 3 : 1
            border.color: swatch.checked ? Theme.accent : Theme.border
        }
    }

    onZoomChanged: {
        root.zoomActive = true;
        zoomIdle.restart();
    }

    header: ToolBar {
        implicitHeight: controls.implicitHeight + (Theme.spacing * 2)

        ColumnLayout {
            id: controls

            anchors.fill: parent
            anchors.leftMargin: Theme.spacing
            anchors.rightMargin: Theme.spacing / 2
            spacing: Theme.spacing / 2

            RowLayout {
                Layout.fillWidth: true
                spacing: root.width < 600 ? Theme.spacing / 6 : Theme.spacing / 2

                Repeater {
                    model: root.inks

                    delegate: Swatch {
                        required property color modelData

                        ink: modelData
                        text: qsTr("Ink %1").arg(modelData)
                        checked: Qt.colorEqual(root.penColor, ink)
                        onPicked: ink => root.penColor = ink
                    }
                }

                Slider {
                    Layout.leftMargin: Theme.spacing
                    Layout.fillWidth: true
                    Layout.minimumWidth: 48
                    Layout.preferredWidth: 120
                    Layout.maximumWidth: 120
                    from: 1
                    to: 36
                    stepSize: 1
                    value: root.penWidth
                    onMoved: root.penWidth = value
                }

                Label {
                    text: qsTr("%1 px").arg(Math.round(root.penWidth))
                    color: Theme.mutedText
                }

                Switch {
                    Layout.leftMargin: Theme.spacing
                    text: root.width < 600 ? "" : qsTr("Snap shapes")
                    Accessible.name: qsTr("Snap shapes")
                    ToolTip.text: qsTr("Snap shapes")
                    ToolTip.visible: hovered && text === ""
                    ToolTip.delay: 600
                    checked: root.store.snapShapes
                    onToggled: root.store.snapShapes = checked
                }

                Item {
                    Layout.fillWidth: true
                }

                CanvasSettings {
                    store: root.store
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
    }

    Timer {
        id: zoomIdle

        interval: 1800
        onTriggered: root.zoomActive = false
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

            width: root.store.pageWidth
            height: root.store.pageHeight
            scale: root.zoom
            transformOrigin: Item.TopLeft
            color: "#fdfdfb" // paper stays light in dark mode, so saved ink colors keep working
            border.color: Theme.border
            // StrokeItem keeps the pointer while drawing, even past the edge: hide what is off the paper.
            clip: true

            Item {
                id: dots

                anchors.fill: parent
                visible: root.store.paperBackground === 1

                readonly property int columns: Math.ceil(paper.width / root.store.paperSpacing)
                readonly property int rows: Math.ceil(paper.height / root.store.paperSpacing)

                Repeater {
                    model: dots.visible ? dots.columns * dots.rows : 0

                    delegate: Rectangle {
                        required property int index

                        x: (index % dots.columns) * root.store.paperSpacing
                        y: Math.floor(index / dots.columns) * root.store.paperSpacing
                        width: root.store.paperThickness
                        height: root.store.paperThickness
                        radius: width / 2
                        color: "#d5d5dc"
                    }
                }
            }

            Item {
                anchors.fill: parent
                visible: root.store.paperBackground === 2

                Repeater {
                    model: root.store.paperBackground === 2 ? Math.ceil(paper.height
                                                                        / root.store.paperSpacing) :
                                                              0

                    delegate: Rectangle {
                        required property int index

                        x: 0
                        y: index * root.store.paperSpacing
                        width: paper.width
                        height: root.store.paperThickness
                        color: "#d5d5dc"
                    }
                }
            }

            StrokeItem {
                anchors.fill: parent
                store: root.store
                penColor: root.penColor
                penWidth: root.penWidth
                onStrokeFinished: points => root.store.addStroke(points, root.penColor,
                                                                 root.penWidth)
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
        color: Theme.mutedText
    }

    Button {
        id: zoomButton
        objectName: "zoomButton"
        anchors.right: viewport.right
        anchors.bottom: viewport.bottom
        anchors.margins: Theme.spacing
        text: qsTr("%1%").arg(Math.round(root.zoom * 100))
        opacity: hovered || activeFocus ? 1 : root.zoomActive ? 0.95 : 0.8
        onClicked: root.fitPage()

        Behavior on opacity {
            NumberAnimation {
                duration: 350
            }
        }

        contentItem: Text {
            text: zoomButton.text
            font: zoomButton.font
            color: Theme.overlayText
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        background: Rectangle {
            implicitWidth: 72
            implicitHeight: Theme.controlHeight
            radius: Theme.radius
            color: Theme.overlay
            border.width: zoomButton.visualFocus ? 2 : 0
            border.color: Theme.accent
        }

        ToolTip {
            y: -height - Theme.spacing / 2
            text: qsTr("Fit canvas to window")
            visible: zoomButton.hovered
            delay: 600
        }
    }

    Label {
        id: snapHint

        objectName: "snapHint"
        anchors.horizontalCenter: viewport.horizontalCenter
        anchors.bottom: viewport.bottom
        anchors.bottomMargin: Theme.spacing
        padding: Theme.spacing / 2
        opacity: 0
        background: Rectangle {
            radius: Theme.radius
            color: Theme.surface
            border.color: Theme.border
        }

        SequentialAnimation on opacity {
            id: fade

            running: false
            NumberAnimation {
                to: 1
                duration: 120
            }
            PauseAnimation {
                duration: 1200
            }
            NumberAnimation {
                to: 0
                duration: 400
            }
        }
    }

    Connections {
        target: root.store

        function onShapeSnapped(shape: int): void {
            switch (shape) {
            case CanvasStore.Shape.Circle:
                snapHint.text = qsTr("Snapped to a circle");
                break;
            case CanvasStore.Shape.Rectangle:
                snapHint.text = qsTr("Snapped to a rectangle");
                break;
            case CanvasStore.Shape.Triangle:
                snapHint.text = qsTr("Snapped to a triangle");
                break;
            }
            fade.restart();
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
