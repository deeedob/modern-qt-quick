pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Basic as Basic

Basic.ComboBox {
    id: control

    implicitHeight: Theme.controlHeight

    indicator: Text {
        x: control.mirrored ? control.leftPadding : control.width - width - control.rightPadding
        y: (control.height - height) / 2
        text: "⌄"
        font.pixelSize: 20
        color: Theme.text
    }

    background: Rectangle {
        implicitWidth: 140
        implicitHeight: Theme.controlHeight
        radius: Theme.radius
        color: control.down ? Theme.pressed : control.hovered ? Theme.hover : Theme.surface
        border.width: control.visualFocus ? 2 : 1
        border.color: control.visualFocus ? Theme.accent : Theme.border
    }

    popup.background: Rectangle {
        radius: Theme.radius
        color: Theme.surface
        border.color: Theme.border
    }
}
