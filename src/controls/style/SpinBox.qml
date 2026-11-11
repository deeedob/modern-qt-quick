import QtQuick
import QtQuick.Templates as T

T.SpinBox {
    id: control

    implicitWidth: 150
    implicitHeight: Theme.controlHeight
    leftPadding: 36
    rightPadding: 36

    validator: IntValidator {
        locale: control.locale.name
        bottom: Math.min(control.from, control.to)
        top: Math.max(control.from, control.to)
    }

    contentItem: TextInput {
        text: control.displayText
        font: control.font
        color: Theme.text
        selectionColor: Theme.accent
        selectedTextColor: Theme.accentText
        horizontalAlignment: TextInput.AlignHCenter
        verticalAlignment: TextInput.AlignVCenter
        readOnly: !control.editable
        validator: control.validator
        inputMethodHints: control.inputMethodHints
        clip: true
    }

    up.indicator: Rectangle {
        x: control.mirrored ? 0 : control.width - width
        width: 34
        height: control.height
        radius: Theme.radius
        color: control.up.pressed ? Theme.pressed : control.up.hovered ? Theme.hover : "transparent"
        Text {
            anchors.centerIn: parent
            text: "+"
            font.family: control.font.family
            font.pixelSize: 20
            color: control.enabled && (control.wrap || control.value < Math.max(control.from,
                                                                                control.to))
                   ? Theme.text : Theme.mutedText
        }
    }

    down.indicator: Rectangle {
        x: control.mirrored ? control.width - width : 0
        width: 34
        height: control.height
        radius: Theme.radius
        color: control.down.pressed ? Theme.pressed : control.down.hovered ? Theme.hover :
                                                                             "transparent"
        Text {
            anchors.centerIn: parent
            text: "−"
            font.family: control.font.family
            font.pixelSize: 20
            color: control.enabled && (control.wrap || control.value > Math.min(control.from,
                                                                                control.to))
                   ? Theme.text : Theme.mutedText
        }
    }

    background: Rectangle {
        radius: Theme.radius
        color: Theme.surface
        border.width: control.activeFocus ? 2 : 1
        border.color: control.activeFocus ? Theme.accent : Theme.border
    }
}
