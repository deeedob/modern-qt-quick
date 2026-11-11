import QtQuick
import QtQuick.Controls.impl
import QtQuick.Templates as T

// An icon-only button shows its text as a tooltip, so the text doubles as the accessible name.
T.ToolButton {
    id: control

    property bool showToolTip: true
    focusPolicy: Qt.StrongFocus

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset, implicitContentWidth
                            + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    padding: 8
    spacing: 6
    display: T.AbstractButton.IconOnly
    icon.width: Theme.iconSize
    icon.height: Theme.iconSize

    contentItem: IconLabel {
        spacing: control.spacing
        display: control.display
        icon: control.icon
        text: control.text
        font: control.font
        defaultIconColor: Theme.text
        color: defaultIconColor
        opacity: control.enabled ? 1.0 : 0.35
    }

    background: Rectangle {
        implicitWidth: Theme.controlHeight
        implicitHeight: Theme.controlHeight
        radius: Theme.radius
        color: control.down || control.checked ? Theme.pressed : control.hovered ? Theme.hover :
                                                                                   "transparent"

        border.width: control.visualFocus ? 2 : 0
        border.color: Theme.accent
    }

    ToolTip {
        objectName: "toolTip"
        text: control.text
        visible: control.showToolTip && control.display === T.AbstractButton.IconOnly
                 && control.text !== "" && control.hovered
        delay: 600
    }
}
