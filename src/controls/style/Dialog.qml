import QtQuick
import QtQuick.Templates as T

T.Dialog {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset, implicitContentWidth
                            + leftPadding + rightPadding, implicitHeaderWidth, implicitFooterWidth)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding + (
                                 implicitHeaderHeight > 0 ? implicitHeaderHeight + spacing : 0) + (
                                 implicitFooterHeight > 0 ? implicitFooterHeight + spacing : 0))

    anchors.centerIn: T.Overlay.overlay
    modal: true
    padding: 20
    spacing: 4

    background: Rectangle {
        radius: 2 * Theme.radius
        color: Theme.surface
        border.color: Theme.border
    }

    header: T.Label {
        text: control.title
        visible: control.title !== ""
        elide: Text.ElideRight
        leftPadding: 20
        rightPadding: 20
        topPadding: 20
        font.family: Theme.fontFamily
        font.pixelSize: 16
        font.weight: Font.DemiBold
        color: Theme.text
    }

    footer: DialogButtonBox {
        visible: count > 0
    }

    T.Overlay.modal: Rectangle {
        color: Color.transparent("black", 0.4)
    }

    T.Overlay.modeless: Rectangle {
        color: Color.transparent("black", 0.12)
    }
}
