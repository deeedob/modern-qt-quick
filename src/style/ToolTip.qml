import QtQuick
import QtQuick.Templates as T

T.ToolTip {
    id: control

    x: parent ? (parent.width - implicitWidth) / 2 : 0
    y: parent ? parent.height + 4 : 0

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset, implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset, implicitContentHeight + topPadding + bottomPadding)

    margins: 6
    padding: 6
    horizontalPadding: 10
    closePolicy: T.Popup.CloseOnEscape | T.Popup.CloseOnPressOutsideParent | T.Popup.CloseOnReleaseOutsideParent

    contentItem: Text {
        text: control.text
        font.family: Theme.fontFamily
        font.pixelSize: 12
        wrapMode: Text.Wrap
        color: Theme.window
    }

    background: Rectangle {
        radius: Theme.radius / 2
        color: Theme.text
    }
}
