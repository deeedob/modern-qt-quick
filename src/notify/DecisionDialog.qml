pragma ComponentBehavior: Bound

import App.Controls
import QtQuick
import QtQuick.Layouts

Dialog {
    id: root

    required property Message notification
    readonly property color severityColor: {
        if (!root.notification)
            return Theme.information;
        switch (root.notification.severity) {
        case Message.Success:
            return Theme.success;
        case Message.Warning:
            return Theme.warning;
        case Message.Error:
            return Theme.error;
        default:
            return Theme.information;
        }
    }

    modal: true
    focus: true
    closePolicy: Popup.NoAutoClose
    anchors.centerIn: Overlay.overlay
    width: Math.min(520, parent ? parent.width - Theme.spacing * 2 : 520)
    height: Math.min(implicitHeight, parent ? parent.height - Theme.spacing * 2 : implicitHeight)
    padding: Theme.spacing * 2
    spacing: Theme.spacing

    onRejected: {
        if (root.notification)
            root.notification.dismiss();
    }

    function focusContent(): void {
        const action = actionRepeater.itemAt(0);
        if (action)
            action.forceActiveFocus();
        else
            messageScroll.forceActiveFocus();
    }

    onOpened: root.focusContent()
    onNotificationChanged: {
        if (root.visible)
            Qt.callLater(root.focusContent);
    }

    background: Rectangle {
        radius: Theme.radius * 2
        color: Theme.surface
        border.width: 3
        border.color: root.severityColor
    }

    contentItem: ScrollView {
        id: messageScroll

        clip: true
        contentWidth: availableWidth
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        Keys.onEscapePressed: {
            if (root.notification)
                root.notification.dismiss();
        }

        ColumnLayout {
            width: messageScroll.availableWidth
            spacing: Theme.spacing

            RowLayout {
                Layout.fillWidth: true
                spacing: Theme.spacing

                Label {
                    text: {
                        if (!root.notification)
                            return "i";
                        switch (root.notification.severity) {
                        case Message.Success:
                            return "✓";
                        case Message.Warning:
                            return "!";
                        case Message.Error:
                            return "×";
                        default:
                            return "i";
                        }
                    }
                    color: root.severityColor
                    font.family: Theme.fontFamily
                    font.pixelSize: 18
                    font.weight: Font.Bold
                }

                Label {
                    objectName: "notificationTitle"
                    Layout.fillWidth: true
                    text: root.notification ? root.notification.title : ""
                    visible: text.length > 0
                    color: Theme.text
                    font.family: Theme.fontFamily
                    font.pixelSize: 16
                    font.weight: Font.DemiBold
                    wrapMode: Text.Wrap
                }
            }

            Label {
                objectName: "notificationMessage"
                Layout.fillWidth: true
                visible: root.notification && root.notification.message.length > 0
                text: root.notification ? root.notification.message : ""
                color: Theme.text
                font.family: Theme.fontFamily
                font.pixelSize: 14
                wrapMode: Text.Wrap
                textFormat: Text.PlainText
            }
        }
    }

    footer: Flow {
        objectName: "notificationActionFooter"
        visible: root.notification && root.notification.actions.length > 0
        width: root.width - root.leftPadding - root.rightPadding
        spacing: Theme.spacing / 2
        Keys.onEscapePressed: {
            if (root.notification)
                root.notification.dismiss();
        }

        Repeater {
            id: actionRepeater
            objectName: "actionRepeater"

            model: root.notification ? root.notification.actions.length : 0

            delegate: ActionButton {
                required property int index

                notification: root.notification
                actionIndex: index
            }
        }
    }
}
