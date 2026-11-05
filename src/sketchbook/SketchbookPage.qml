pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import AppStyle

// An MVVM view: binds to the view model's list and calls its commands. It knows no files.
Page {
    id: root

    required property SketchbookViewModel viewModel

    // Asks the window to show the canvas, where the opened sketch is now loaded.
    signal opened

    padding: Theme.spacing

    ColumnLayout {
        anchors.fill: parent
        spacing: Theme.spacing

        Button {
            text: qsTr("New sketch")
            icon.source: Theme.icon("plus")
            highlighted: true
            onClicked: {
                root.viewModel.create();
                root.opened();
            }
        }

        ListView {
            id: list

            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 2
            model: root.viewModel.model

            // Roles arrive as required properties; user intent leaves through the view model.
            delegate: ItemDelegate {
                id: row

                required property int index
                required property string name
                required property date modified
                required property int strokeCount

                width: list.width
                text: name
                onClicked: {
                    root.viewModel.open(index);
                    root.opened();
                }

                contentItem: RowLayout {
                    spacing: Theme.spacing

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2

                        Label {
                            Layout.fillWidth: true
                            text: row.name
                            font.weight: Font.Medium
                            elide: Text.ElideRight
                        }

                        Label {
                            Layout.fillWidth: true
                            text: qsTr("%n stroke(s)", "", row.strokeCount) + " · " + row.modified.toLocaleString(Qt.locale(), Locale.ShortFormat)
                            color: Theme.mutedText
                            font.pixelSize: 12
                            elide: Text.ElideRight
                        }
                    }

                    ToolButton {
                        icon.source: Theme.icon("pencil")
                        text: qsTr("Rename")
                        onClicked: renameDialog.show(row.index, row.name)
                    }

                    ToolButton {
                        icon.source: Theme.icon("trash")
                        text: qsTr("Delete")
                        onClicked: root.viewModel.remove(row.index)
                    }
                }
            }

            Label {
                anchors.centerIn: parent
                visible: list.count === 0
                text: qsTr("No sketches yet")
                color: Theme.mutedText
            }
        }
    }

    Dialog {
        id: renameDialog

        property int row: -1

        function show(row: int, name: string): void {
            renameDialog.row = row;
            nameField.text = name;
            open();
            nameField.selectAll();
            nameField.forceActiveFocus();
        }

        width: 360
        title: qsTr("Rename sketch")
        standardButtons: Dialog.Ok | Dialog.Cancel
        onAccepted: root.viewModel.rename(row, nameField.text)

        contentItem: TextField {
            id: nameField

            onAccepted: renameDialog.accept()
        }
    }
}
