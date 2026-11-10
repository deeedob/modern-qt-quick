pragma ComponentBehavior: Bound

import QtQuick
import App.Controls

Item {
    id: root

    required property string name
    property bool editing: false
    signal renameRequested(string name)

    function focusEditor(): void {
        if (root.editing) {
            titleField.forceActiveFocus();
            titleField.selectAll();
        }
    }

    implicitWidth: 180
    implicitHeight: Theme.controlHeight

    Label {
        id: titleLabel
        objectName: "sketchTitleLabel"
        anchors.fill: parent
        visible: !root.editing
        text: root.name
        font.pixelSize: 16
        font.weight: Font.DemiBold
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight

        MouseArea {
            anchors.fill: parent
            onDoubleClicked: {
                root.editing = true;
                titleField.text = root.name;
                Qt.callLater(root.focusEditor);
            }
        }
    }

    TextField {
        id: titleField
        objectName: "sketchNameField"
        anchors.fill: parent
        visible: root.editing
        placeholderText: qsTr("Sketch name")
        onEditingFinished: {
            if (root.editing)
                root.renameRequested(text);
            root.editing = false;
        }
        Keys.onEscapePressed: {
            root.editing = false;
            titleField.focus = false;
        }
    }
}
