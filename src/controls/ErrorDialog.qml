pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

Dialog {
    id: root

    property string source: ""
    required property string message

    width: Math.min(440, parent ? parent.width - Theme.spacing * 2 : 440)
    padding: Theme.spacing * 2
    contentItem: ColumnLayout {
        spacing: Theme.spacing

        Label {
            id: sourceLabel
            objectName: "sourceLabel"

            Layout.fillWidth: true
            text: root.source
            textFormat: Text.PlainText
            color: Theme.mutedText
            wrapMode: Text.Wrap
            visible: text.length > 0
        }

        Label {
            id: messageLabel
            objectName: "messageLabel"

            Layout.fillWidth: true
            text: root.message
            textFormat: Text.PlainText
            wrapMode: Text.Wrap
        }
    }
}
