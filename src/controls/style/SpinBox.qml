import QtQuick
import QtQuick.Templates as T

T.SpinBox {
    id: control

    property bool linkedFocus: false
    property string suffix: ""

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
        id: input

        text: control.displayText
        font: control.font
        color: Theme.text
        selectionColor: Theme.accent
        selectedTextColor: Theme.accentText
        horizontalAlignment: TextInput.AlignHCenter
        verticalAlignment: TextInput.AlignVCenter
        rightPadding: suffixLabel.visible ? suffixLabel.width + 4 : 0
        readOnly: !control.editable
        validator: control.validator
        inputMethodHints: control.inputMethodHints
        clip: true

        Text {
            id: suffixLabel

            x: (input.width - input.rightPadding + input.contentWidth) / 2 + 4
            anchors.verticalCenter: parent.verticalCenter
            text: control.suffix
            visible: text !== ""
            font: input.font
            color: Theme.mutedText
        }
    }

    up.indicator: Rectangle {
        x: control.mirrored ? 2 : control.width - width - 2
        y: 2
        width: 32
        height: control.height - 4
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
        x: control.mirrored ? control.width - width - 2 : 2
        y: 2
        width: 32
        height: control.height - 4
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
        border.width: control.activeFocus || control.linkedFocus ? 2 : 1
        border.color: control.activeFocus || control.linkedFocus ? Theme.accent : Theme.border
    }
}
