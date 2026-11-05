// SPDX-FileCopyrightText: 2026 Dennis Oberst
// SPDX-License-Identifier: MIT-0

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import App.Backend.Tasks
import App.Backend.Timer
import App.Frontend.Tasks
import App.Frontend.Timer
import App.Frontend.Ui

// main() injects the dependencies as required properties; only AppController is a singleton.
ApplicationWindow {
    id: window

    required property TasksViewModel tasks
    required property TimerStore timer

    width: 420
    height: 640
    visible: true
    title: AppController.displayName
    font.family: Theme.fontFamily
    Material.theme: Material.System
    Material.accent: Theme.accent

    header: ToolBar {
        // Blend the bar into the window instead of the style's primary color.
        Material.primary: window.Material.background
        Material.elevation: 0

        RowLayout {
            anchors.fill: parent
            anchors.rightMargin: Theme.spacing / 2

            TabBar {
                id: tabs

                Layout.fillWidth: true

                TabButton {
                    text: qsTr("Tasks")
                }

                TabButton {
                    text: qsTr("Focus")
                }
            }

            IconButton {
                iconName: "languages"
                toolTip: qsTr("Switch language")
                onClicked: AppController.language = AppController.language === "de" ? "en" : "de"
            }

            IconButton {
                iconName: "info"
                toolTip: qsTr("About")
                onClicked: about.open()
            }
        }
    }

    StackLayout {
        anchors.fill: parent
        currentIndex: tabs.currentIndex

        TasksPage {
            viewModel: window.tasks
        }

        TimerPage {
            store: window.timer
        }
    }

    AboutDialog {
        id: about
    }
}
