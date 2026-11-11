import QtQuick
import QtQuick.Controls.impl
import QtQuick.Templates as T

// highlighted marks the primary action; flat buttons only show a background on interaction.
T.Button {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset, implicitContentWidth
                            + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    padding: 8
    horizontalPadding: 16
    spacing: 6
    icon.width: Theme.iconSize
    icon.height: Theme.iconSize

    contentItem: IconLabel {
        spacing: control.spacing
        display: control.display
        icon: control.icon
        text: control.text
        font.family: control.font.family
        font.pixelSize: control.font.pixelSize
        font.weight: Font.Medium
        defaultIconColor: control.highlighted ? Theme.accentText : Theme.text
        color: defaultIconColor
    }

    background: Rectangle {
        implicitWidth: 80
        implicitHeight: Theme.controlHeight
        radius: Theme.radius
        visible: !control.flat || control.down || control.hovered
        color: control.highlighted ? (control.down ? Theme.accentPressed : Theme.accent) :
                                     control.down ? Theme.pressed : control.hovered ? Theme.hover :
                                                                                      Theme.surface
        border.width: control.visualFocus ? 2 : control.highlighted || control.flat ? 0 : 1
        border.color: control.visualFocus ? Theme.accent : Theme.border
        opacity: control.enabled ? 1.0 : 0.5
    }
}
