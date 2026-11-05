// SPDX-FileCopyrightText: 2026 Dennis Oberst
// SPDX-License-Identifier: MIT-0

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import App.Frontend.Ui

// Roles arrive as required properties, user intent leaves as signals: the delegate never writes data.
ItemDelegate {
    id: root

    required property int index
    required property string title
    required property bool done

    signal toggleRequested
    signal removeRequested

    width: ListView.view?.width ?? 0
    onClicked: root.toggleRequested()

    contentItem: RowLayout {
        spacing: Theme.spacing

        CheckBox {
            checked: root.done
            onToggled: root.toggleRequested()
        }

        Label {
            Layout.fillWidth: true
            text: root.title
            font.strikeout: root.done
            opacity: root.done ? 0.5 : 1.0
            elide: Text.ElideRight
        }

        IconButton {
            iconName: "trash"
            toolTip: qsTr("Remove")
            onClicked: root.removeRequested()
        }
    }
}
