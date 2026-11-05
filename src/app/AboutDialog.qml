// SPDX-FileCopyrightText: 2026 Dennis Oberst
// SPDX-License-Identifier: MIT-0

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import App.Frontend.Ui

Dialog {
    id: root

    anchors.centerIn: Overlay.overlay
    width: Math.min(360, (parent?.width ?? 360) - Theme.spacing * 4)
    modal: true
    padding: Theme.spacing * 2

    background: Rectangle {
        color: root.Material.dialogColor
        radius: 16
        border.width: 1
        border.color: root.Material.frameColor
    }

    contentItem: ColumnLayout {
        spacing: Theme.spacing

        Image {
            Layout.alignment: Qt.AlignHCenter
            source: "app.svg"
            sourceSize: Qt.size(72, 72)
        }

        Label {
            Layout.fillWidth: true
            text: AppController.displayName
            font.pixelSize: 22
            font.weight: Font.DemiBold
            horizontalAlignment: Text.AlignHCenter
        }

        Label {
            Layout.fillWidth: true
            text: qsTr("Version %1").arg(AppController.version)
            horizontalAlignment: Text.AlignHCenter
        }

        Label {
            Layout.fillWidth: true
            text: AppController.buildDetails
            font.pixelSize: 12
            opacity: 0.6
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
        }

        MenuSeparator {
            Layout.fillWidth: true
        }

        Label {
            Layout.fillWidth: true
            text: qsTr("Built with Qt, licensed under the LGPL version 3. See THIRD_PARTY_NOTICES.txt.")
            font.pixelSize: 12
            opacity: 0.8
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: Theme.spacing / 2

            Button {
                text: qsTr("Open log folder")
                flat: true
                onClicked: AppController.openLogDirectory()
            }

            Item {
                Layout.fillWidth: true
            }

            Button {
                text: qsTr("Close")
                highlighted: true
                onClicked: root.close()
            }
        }
    }
}
