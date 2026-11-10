import QtQuick
import QtQuick.Controls.impl
import QtQuick.Templates as T

T.ItemDelegate {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset, implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset, implicitContentHeight + topPadding + bottomPadding)

    padding: 12
    verticalPadding: 8
    spacing: 8

    contentItem: IconLabel {
        spacing: control.spacing
        alignment: Qt.AlignLeft
        display: control.display
        icon: control.icon
        text: control.text
        font: control.font
        defaultIconColor: Theme.text
        color: defaultIconColor
    }

    background: Rectangle {
        implicitHeight: 48
        radius: Theme.radius
        color: control.down ? Theme.pressed : control.hovered || control.highlighted ? Theme.hover : "transparent"
    }
}
