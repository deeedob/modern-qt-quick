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
    property bool aspectLinked: false
    property real aspectRatio: 1.0

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

                ToolButton {
                    id: paperButton
                    objectName: "paperSettingsButton"
                    icon.source: Theme.icon("settings-2")
                    text: qsTr("Canvas settings")
                    onClicked: paperSettings.open()

                    Popup {
                        id: paperSettings
                        x: paperButton.width - width
                        y: paperButton.height + Theme.spacing / 2
                        padding: Theme.spacing
                        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

                        background: Rectangle {
                            color: Theme.surface
                            radius: Theme.radius
                            border.color: Theme.border
                        }

                        contentItem: ColumnLayout {
                            spacing: Theme.spacing / 2

                            Label {
                                text: qsTr("Canvas")
                                font.weight: Font.DemiBold
                            }

                            GridLayout {
                                columns: 3
                                columnSpacing: Theme.spacing / 2
                                rowSpacing: Theme.spacing / 2

                                Label {
                                    text: qsTr("Width")
                                }
                                SpinBox {
                                    objectName: "pageWidth"
                                    Layout.fillWidth: true
                                    from: 100
                                    to: 4096
                                    editable: true
                                    value: root.store.pageWidth
                                    onValueModified: root.store.setPageSize(value, root.aspectLinked ? Math.max(100, Math.min(4096, Math.round(value / root.aspectRatio))) : root.store.pageHeight)
                                }
                                ToolButton {
                                    objectName: "aspectLink"
                                    checkable: true
                                    checked: root.aspectLinked
                                    icon.source: Theme.icon(root.aspectLinked ? "link-2" : "unlink-2")
                                    text: root.aspectLinked ? qsTr("Unlock aspect ratio") : qsTr("Lock aspect ratio")
                                    onClicked: {
                                        root.aspectRatio = root.store.pageWidth / root.store.pageHeight;
                                        root.aspectLinked = !root.aspectLinked;
                                    }
                                }
                                Label {
                                    text: qsTr("Height")
                                }
                                SpinBox {
                                    objectName: "pageHeight"
                                    Layout.fillWidth: true
                                    from: 100
                                    to: 4096
                                    editable: true
                                    value: root.store.pageHeight
                                    onValueModified: root.store.setPageSize(root.aspectLinked ? Math.max(100, Math.min(4096, Math.round(value * root.aspectRatio))) : root.store.pageWidth, value)
                                }
                                Item {
                                    Layout.preferredWidth: Theme.controlHeight
                                }

                                ComboBox {
                                    Layout.fillWidth: true
                                    Layout.columnSpan: 3
                                    model: [qsTr("Plain"), qsTr("Dots"), qsTr("Ruled")]
                                    currentIndex: root.store.paperBackground
                                    onActivated: index => root.store.setPaperBackground(index)
                                }

                                Label {
                                    visible: root.store.paperBackground !== 0
                                    text: qsTr("Spacing")
                                }
                                SpinBox {
                                    visible: root.store.paperBackground !== 0
                                    Layout.fillWidth: true
                                    from: 8
                                    to: 128
                                    editable: true
                                    value: root.store.paperSpacing
                                    onValueModified: root.store.setPaperPattern(value, root.store.paperThickness)
                                }
                                Item {
                                    visible: root.store.paperBackground !== 0
                                    Layout.preferredWidth: Theme.controlHeight
                                }

                                Label {
                                    visible: root.store.paperBackground !== 0
                                    text: qsTr("Thickness")
                                }
                                SpinBox {
                                    visible: root.store.paperBackground !== 0
                                    Layout.fillWidth: true
                                    from: 1
                                    to: 8
                                    editable: true
                                    value: root.store.paperThickness
                                    onValueModified: root.store.setPaperPattern(root.store.paperSpacing, value)
                                }
                                Item {
                                    visible: root.store.paperBackground !== 0
                                    Layout.preferredWidth: Theme.controlHeight
                                }
                            }
                        }
                    }
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

    // Zoom belongs to this view, not to the sketch, so it lives here instead of in the store.
    property real zoom: 1.0
    property bool zoomActive: false
    readonly property real fitZoom: Math.min(viewport.width / paper.width, viewport.height / paper.height)
    readonly property real minimumZoom: fitZoom * 0.85

    onZoomChanged: {
        root.zoomActive = true;
        zoomIdle.restart();
    }

    Timer {
        id: zoomIdle
        interval: 1800
        onTriggered: root.zoomActive = false
    }

    // Clamps directly: Flickable.returnToBounds() would start an animation on every pinch step.
    function scrollTo(x: real, y: real): void {
        viewport.contentX = Math.max(0, Math.min(x, viewport.contentWidth - viewport.width));
        viewport.contentY = Math.max(0, Math.min(y, viewport.contentHeight - viewport.height));
    }

    // Keeps the paper point under `position`, in viewport coordinates, where it is.
    function zoomBy(factor: real, position: point): void {
        const next = Math.min(Math.max(root.zoom * factor, root.minimumZoom), 4);
        const ratio = next / root.zoom;
        root.zoom = next;
        root.scrollTo((viewport.contentX + position.x) * ratio - position.x, (viewport.contentY + position.y) * ratio - position.y);
    }

    function fitPage(): void {
        root.zoom = root.fitZoom;
        root.scrollTo((viewport.contentWidth - viewport.width) / 2, (viewport.contentHeight - viewport.height) / 2);
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
                    model: root.store.paperBackground === 2 ? Math.ceil(paper.height / root.store.paperSpacing) : 0

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
                onStrokeFinished: points => root.store.addStroke(points, root.penColor, root.penWidth)
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
            color: Theme.accentText
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

        function onHistoryMoved(redo: bool): void {
            snapHint.text = redo ? qsTr("Redone") : qsTr("Undone");
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
