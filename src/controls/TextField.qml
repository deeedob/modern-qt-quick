import QtQuick
import QtQuick.Controls.impl
import QtQuick.Templates as T

T.TextField {
    id: control

    implicitWidth: implicitBackgroundWidth + leftInset + rightInset || Math.max(contentWidth, placeholder.implicitWidth) + leftPadding + rightPadding
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset, contentHeight + topPadding + bottomPadding, placeholder.implicitHeight + topPadding + bottomPadding)

    padding: 8
    leftPadding: 12
    rightPadding: 12

    color: Theme.text
    selectionColor: Theme.accent
    selectedTextColor: Theme.accentText
    placeholderTextColor: Theme.mutedText
    verticalAlignment: TextInput.AlignVCenter

    PlaceholderText {
        id: placeholder

        x: control.leftPadding
        y: control.topPadding
        width: control.width - (control.leftPadding + control.rightPadding)
        height: control.height - (control.topPadding + control.bottomPadding)
        text: control.placeholderText
        font: control.font
        color: control.placeholderTextColor
        verticalAlignment: control.verticalAlignment
        visible: !control.length && !control.preeditText
        elide: Text.ElideRight
        renderType: control.renderType
    }

    background: Rectangle {
        implicitWidth: 200
        implicitHeight: Theme.controlHeight
        radius: Theme.radius
        color: Theme.surface
        border.width: control.activeFocus ? 2 : 1
        border.color: control.activeFocus ? Theme.accent : Theme.border
    }
}
