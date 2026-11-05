// SPDX-FileCopyrightText: 2026 Dennis Oberst
// SPDX-License-Identifier: MIT-0

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import App.Backend.Timer
import App.Frontend.Ui

// Flux view: reads the store's state, and turns every user intent into a dispatched action.
Page {
    id: root

    required property TimerStore store

    function formatTime(seconds: int): string {
        const minutes = Math.floor(seconds / 60);
        return `${minutes}:${String(seconds % 60).padStart(2, "0")}`;
    }

    ColumnLayout {
        anchors.centerIn: parent
        spacing: Theme.spacing * 2

        ProgressRing {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: 240
            Layout.preferredHeight: 240
            progress: root.store.progress
            thickness: 14
            color: Material.accent
            trackColor: Material.frameColor

            Behavior on progress {
                NumberAnimation {
                    duration: 150
                }
            }

            Label {
                anchors.centerIn: parent
                text: root.formatTime(root.store.remainingSeconds)
                font.pixelSize: 44
                font.weight: Font.DemiBold
                font.features: {
                    "tnum": 1
                }
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter

            IconButton {
                iconName: root.store.running ? "pause" : "play"
                toolTip: root.store.running ? qsTr("Pause") : qsTr("Start")
                onClicked: root.store.dispatch(root.store.running ? TimerStore.Action.Pause : TimerStore.Action.Start)
            }

            IconButton {
                iconName: "rotate-ccw"
                toolTip: qsTr("Reset")
                onClicked: root.store.dispatch(TimerStore.Action.Reset)
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter

            SpinBox {
                from: 1
                to: 120
                value: root.store.durationMinutes
                onValueModified: root.store.dispatch(TimerStore.Action.SetDuration, value)
            }

            Label {
                text: qsTr("minutes")
            }
        }
    }
}
