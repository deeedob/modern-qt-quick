// SPDX-FileCopyrightText: 2026 Dennis Oberst
// SPDX-License-Identifier: MIT-0

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import App.Backend.Tasks
import App.Frontend.Ui

// MVVM view: binds to view model properties and forwards user input to its commands.
Page {
    id: root

    required property TasksViewModel viewModel

    padding: Theme.spacing

    function addTask(): void {
        root.viewModel.add(newTask.text);
        newTask.clear();
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: Theme.spacing

        TextField {
            Layout.fillWidth: true
            placeholderText: qsTr("Search tasks")
            text: root.viewModel.searchText
            onTextEdited: root.viewModel.searchText = text
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ListView {
                id: list

                anchors.fill: parent
                clip: true
                model: root.viewModel.model
                delegate: TaskDelegate {
                    onToggleRequested: root.viewModel.toggle(index)
                    onRemoveRequested: root.viewModel.remove(index)
                }
            }

            Label {
                anchors.centerIn: parent
                visible: list.count === 0
                text: root.viewModel.searchText.length > 0 ? qsTr("No matching tasks") : qsTr("Nothing to do")
                opacity: 0.6
            }
        }

        RowLayout {
            Layout.fillWidth: true

            TextField {
                id: newTask

                Layout.fillWidth: true
                placeholderText: qsTr("New task")
                onAccepted: root.addTask()
            }

            IconButton {
                iconName: "plus"
                toolTip: qsTr("Add task")
                enabled: newTask.text.trim().length > 0
                onClicked: root.addTask()
            }
        }

        Label {
            text: qsTr("%n open task(s)", "", root.viewModel.openCount)
            opacity: 0.7
        }
    }
}
