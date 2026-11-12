pragma ComponentBehavior: Bound

import App.Controls
import QtQuick

Button {
    id: root

    required property Message notification
    required property int actionIndex
    readonly property bool validAction: root.notification && root.actionIndex >= 0
                                        && root.actionIndex < root.notification.actions.length
    readonly property string actionId: root.validAction
                                       ? root.notification.actions[root.actionIndex].id : ""

    objectName: "notificationActionButton"
    text: root.validAction ? root.notification.actions[root.actionIndex].text : ""
    enabled: root.validAction && root.notification.actions[root.actionIndex].enabled
    font.family: Theme.fontFamily

    Keys.onEscapePressed: {
        if (root.notification)
            root.notification.dismiss();
    }

    onClicked: {
        if (root.validAction)
            root.notification.choose(root.actionId);
    }
}
