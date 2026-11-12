pragma ComponentBehavior: Bound

import App.Controls
import QtQuick
import QtQuick.Layouts

FocusScope {
    id: root
    objectName: "notificationCard"

    required property Message notification
    required property Flickable viewport
    readonly property real viewportY: root.parent.y - root.viewport.contentY
    readonly property bool presented: root.visible && root.viewportY + root.height > 0
                                      && root.viewportY < root.viewport.height
    readonly property color severityColor: {
        switch (root.notification ? root.notification.severity : Message.Information) {
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

    implicitWidth: card.implicitWidth
    implicitHeight: card.implicitHeight
    height: implicitHeight

    function updatePresentation(): void {
        if (root.notification)
            root.notification.setPresentationState(root.presented, hoverHandler.hovered
                                                   || root.activeFocus);
    }

    Component.onCompleted: root.updatePresentation()
    Component.onDestruction: {
        if (root.notification)
            root.notification.setPresentationState(false, false);
    }
    onActiveFocusChanged: root.updatePresentation()
    onPresentedChanged: root.updatePresentation()
    onVisibleChanged: root.updatePresentation()

    Rectangle {
        id: card
        objectName: "notificationSurface"

        implicitWidth: 320
        implicitHeight: content.implicitHeight + Theme.spacing * 2
        width: root.width
        height: implicitHeight
        radius: Theme.radius
        color: Qt.rgba(Theme.surface.r, Theme.surface.g, Theme.surface.b, hoverHandler.hovered
                       ? 0.98 : 0.90)
        border.width: root.notification && root.notification.presentation === Message.Persistent
                      ? 2 : 1
        border.color: (root.notification && root.notification.presentation === Message.Persistent)
                      || hoverHandler.hovered ? root.severityColor : Theme.surface

        Behavior on color {
            ColorAnimation {
                duration: 150
            }
        }
        Behavior on border.color {
            ColorAnimation {
                duration: 150
            }
        }

        ColumnLayout {
            id: content

            anchors.fill: parent
            anchors.margins: Theme.spacing
            spacing: Theme.spacing

            RowLayout {
                Layout.fillWidth: true
                spacing: Theme.spacing

                Rectangle {
                    Layout.preferredWidth: 26
                    Layout.preferredHeight: 26
                    radius: width / 2
                    color: Qt.rgba(root.severityColor.r, root.severityColor.g, root.severityColor.b,
                                   0.18)

                    Label {
                        anchors.centerIn: parent
                        text: {
                            switch (root.notification ? root.notification.severity :
                                                        Message.Information) {
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
                        Accessible.name: {
                            switch (root.notification ? root.notification.severity :
                                                        Message.Information) {
                            case Message.Success:
                                return qsTr("Success");
                            case Message.Warning:
                                return qsTr("Warning");
                            case Message.Error:
                                return qsTr("Error");
                            default:
                                return qsTr("Information");
                            }
                        }
                        color: root.severityColor
                        font.family: Theme.fontFamily
                        font.pixelSize: 14
                        font.weight: Font.Bold
                    }
                }

                Label {
                    objectName: "notificationTitle"
                    Layout.fillWidth: true
                    text: root.notification ? (root.notification.title
                                               || root.notification.message) : ""
                    color: Theme.text
                    font.family: Theme.fontFamily
                    font.pixelSize: 14
                    font.weight: Font.DemiBold
                    wrapMode: Text.Wrap
                }

                Item {
                    Layout.preferredWidth: 32
                    Layout.preferredHeight: 32

                    Canvas {
                        id: timerRing
                        anchors.fill: parent
                        onPaint: {
                            const ctx = getContext("2d");
                            ctx.clearRect(0, 0, width, height);
                            const remaining = root.notification ? root.notification.timeRemaining() :
                                                                  -1;
                            if (remaining < 0)
                                return;
                            ctx.strokeStyle = root.severityColor;
                            ctx.lineWidth = 2;
                            ctx.beginPath();
                            ctx.arc(width / 2, height / 2, width / 2 - 2, -Math.PI / 2, -Math.PI / 2
                                    + Math.PI * 2 * remaining, false);
                            ctx.stroke();
                        }
                        Timer {
                            interval: 50
                            repeat: true
                            running: root.presented && root.notification !== null
                            onTriggered: timerRing.requestPaint()
                        }
                    }
                    Button {
                        objectName: "dismissNotification"
                        anchors.fill: parent
                        padding: 0
                        horizontalPadding: 0
                        verticalPadding: 0
                        flat: true
                        text: "×"
                        Accessible.name: qsTr("Dismiss notification")
                        onClicked: {
                            if (root.notification)
                                root.notification.dismiss();
                        }
                    }
                }
            }

            Label {
                objectName: "notificationMessage"
                Layout.fillWidth: true
                visible: root.notification && root.notification.title.length > 0
                         && root.notification.message.length > 0
                text: root.notification ? root.notification.message : ""
                color: Theme.text
                font.family: Theme.fontFamily
                font.pixelSize: 13
                wrapMode: Text.Wrap
                textFormat: Text.PlainText
            }

            Flow {
                objectName: "notificationActions"

                Layout.fillWidth: true
                visible: root.notification && root.notification.actions.length > 0
                spacing: Theme.spacing / 2

                Repeater {
                    id: actionRepeater
                    objectName: "notificationActionRepeater"

                    model: root.notification ? root.notification.actions.length : 0

                    delegate: ActionButton {
                        required property int index

                        notification: root.notification
                        actionIndex: index
                    }
                }
            }
        }
    }

    HoverHandler {
        id: hoverHandler

        onHoveredChanged: root.updatePresentation()
    }
}
