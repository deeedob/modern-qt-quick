pragma ComponentBehavior: Bound
pragma Translator: CanvasPage

import QtQuick
import QtQuick.Layouts
import App.Controls

ToolButton {
    id: root

    required property CanvasStore store
    property bool aspectLinked: false
    property real aspectRatio: 1.0
    objectName: "paperSettingsButton"
    icon.source: Theme.icon("settings-2")
    text: qsTr("Canvas settings")
    onClicked: paperSettings.open()

    Popup {
        id: paperSettings
        x: root.width - width
        y: root.height + Theme.spacing / 2
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
                    onValueModified: root.store.setPageSize(value, root.aspectLinked ? Math.max(100,
                                                                                                Math.min(4096,
                                                                                                         Math.round(
                                                                                                             value / root.aspectRatio))) :
                                                                                       root.store.pageHeight)
                }
                ToolButton {
                    objectName: "aspectLink"
                    checkable: true
                    checked: root.aspectLinked
                    icon.source: Theme.icon(root.aspectLinked ? "link-2" : "unlink-2")
                    text: root.aspectLinked ? qsTr("Unlock aspect ratio") : qsTr(
                                                  "Lock aspect ratio")
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
                    onValueModified: root.store.setPageSize(root.aspectLinked ? Math.max(100,
                                                                                         Math.min(4096,
                                                                                                  Math.round(
                                                                                                      value * root.aspectRatio))) :
                                                                                root.store.pageWidth,
                                                            value)
                }
                Item {
                    Layout.preferredWidth: Theme.controlHeight
                }

                ComboBox {
                    Layout.fillWidth: true
                    Layout.columnSpan: 3
                    model: [qsTr("Plain"), qsTr("Dots"), qsTr("Ruled")]
                    currentIndex: [Paper.Plain, Paper.Dots, Paper.Ruled].indexOf(
                        root.store.paperBackground)
                    onActivated: index => root.store.setPaperBackground([Paper.Plain, Paper.Dots,
                                                                         Paper.Ruled][index])
                }

                Label {
                    visible: root.store.paperBackground !== Paper.Plain
                    text: qsTr("Spacing")
                }
                SpinBox {
                    visible: root.store.paperBackground !== Paper.Plain
                    Layout.fillWidth: true
                    from: 8
                    to: 128
                    editable: true
                    value: root.store.paperSpacing
                    onValueModified: root.store.setPaperPattern(value, root.store.paperThickness)
                }
                Item {
                    visible: root.store.paperBackground !== Paper.Plain
                    Layout.preferredWidth: Theme.controlHeight
                }

                Label {
                    visible: root.store.paperBackground !== Paper.Plain
                    text: qsTr("Thickness")
                }
                SpinBox {
                    visible: root.store.paperBackground !== Paper.Plain
                    Layout.fillWidth: true
                    from: 1
                    to: 8
                    editable: true
                    value: root.store.paperThickness
                    onValueModified: root.store.setPaperPattern(root.store.paperSpacing, value)
                }
                Item {
                    visible: root.store.paperBackground !== Paper.Plain
                    Layout.preferredWidth: Theme.controlHeight
                }
            }
        }
    }
}
