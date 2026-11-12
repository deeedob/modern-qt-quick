pragma ComponentBehavior: Bound

import App.Controls
import QtQuick
import QtQuick.Window

Item {
    id: root

    required property NotificationCenter center
    readonly property bool hostActive: Window.window ? Window.window.active : true

    width: parent ? parent.width : 0
    height: parent ? parent.height : 0

    Component.onCompleted: {
        if (root.center.currentDecision)
            decisionDialog.open();
    }

    Component.onDestruction: root.center.shutdown()

    Binding {
        target: root.center
        property: "hostActive"
        value: root.hostActive
    }

    Timer {
        id: decisionCloseTimer

        interval: 0
        onTriggered: {
            if (!root.center.currentDecision)
                decisionDialog.close();
        }
    }

    ListView {
        id: cardList
        objectName: "cardList"

        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottomMargin: Theme.spacing * 3
        width: Math.min(320, Math.max(0, root.width - Theme.spacing * 4))
        height: Math.min(contentHeight, Math.min(216, root.height * 0.4))
        spacing: Theme.spacing
        model: root.center
        clip: true
        boundsBehavior: Flickable.StopAtBounds

        WheelHandler {
            acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
            target: null
            onWheel: event => {
                const delta = event.pixelDelta.y || event.angleDelta.y / 2;

                cardList.contentY = Math.max(0, Math.min(cardList.contentHeight - cardList.height,
                                                         cardList.contentY - delta));
                event.accepted = true;
            }
        }

        ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AsNeeded
        }

        delegate: Item {
            required property Message notification

            width: cardList.width
            height: card.implicitHeight

            Card {
                id: card

                anchors.left: parent.left
                anchors.right: parent.right
                notification: parent.notification
                viewport: cardList
            }
        }
    }

    Button {
        id: countButton
        objectName: "notificationCount"

        anchors.right: cardList.right
        anchors.bottom: cardList.top
        anchors.bottomMargin: Theme.spacing / 2
        visible: cardList.contentHeight > cardList.height + 1
        width: 32
        height: 32
        padding: 0
        horizontalPadding: 0
        verticalPadding: 0
        background: Rectangle {
            radius: width / 2
            color: countButton.hovered ? Theme.hover : Theme.surface
            border.color: Theme.border
        }
        text: hovered || visualFocus ? "×" : root.center.count.toString()
        Accessible.name: qsTr("Dismiss passive notifications")
        onClicked: root.center.dismissPassive()
    }

    DecisionDialog {
        id: decisionDialog
        objectName: "decisionDialog"

        notification: root.center.currentDecision
    }

    Connections {
        target: root.center

        function onRowsInserted(): void {
            cardList.positionViewAtBeginning();
        }

        function onCurrentDecisionChanged(): void {
            if (root.center.currentDecision) {
                decisionCloseTimer.stop();
                if (!decisionDialog.visible)
                    decisionDialog.open();
                return;
            }
            decisionCloseTimer.restart();
        }
    }
}
