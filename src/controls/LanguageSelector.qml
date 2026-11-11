pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import App.Controls

ToolButton {
    id: root

    readonly property list<string> languageCodes: ["en", "de", "es"]
    required property string language
    signal languageSelected(string language)

    objectName: "languageButton"
    icon.source: Theme.icon("languages")
    text: qsTr("Language")
    showToolTip: !languageMenu.visible
    onHoveredChanged: {
        if (hovered && !languageMenu.visible)
            languageHover.restart();
    }
    onClicked: {
        languageMenu.hoverOpened = false;
        languageMenu.open();
        if (languageMenu.opened)
            menuContent.forceActiveFocus();
    }

    Timer {
        id: languageHover
        interval: 400
        onTriggered: {
            if (root.hovered && !languageMenu.visible) {
                languageMenu.hoverOpened = true;
                languageMenu.open();
            }
        }
    }

    Popup {
        id: languageMenu
        objectName: "languageMenu"
        property bool hoverOpened: false
        property int highlightedIndex: 0
        x: root.width - width
        y: root.height + Theme.spacing / 2
        padding: Theme.spacing / 2
        focus: !hoverOpened
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent
        onOpened: {
            highlightedIndex = Math.max(0, root.languageCodes.indexOf(root.language));
            if (!hoverOpened)
                menuContent.forceActiveFocus();
        }
        onClosed: hoverOpened = false

        background: Rectangle {
            color: Theme.surface
            radius: Theme.radius
            border.color: Theme.border
        }

        contentItem: ColumnLayout {
            id: menuContent
            spacing: 0
            HoverHandler {
                id: menuHover
            }

            Keys.onPressed: event => {
                if (event.key === Qt.Key_Down || event.key === Qt.Key_Up) {
                    languageMenu.highlightedIndex = (languageMenu.highlightedIndex + (event.key
                                                                                      === Qt.Key_Down
                                                                                      ? 1 : 2))
                            % root.languageCodes.length;
                    event.accepted = true;
                } else if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter || event.key
                           === Qt.Key_Space) {
                    root.languageSelected(root.languageCodes[languageMenu.highlightedIndex]);
                    languageMenu.close();
                    event.accepted = true;
                }
            }

            Repeater {
                model: root.languageCodes.length
                delegate: ItemDelegate {
                    id: languageItem
                    required property int index
                    readonly property string languageCode: root.languageCodes[index]
                    objectName: "language_" + languageCode
                    Layout.fillWidth: true
                    implicitWidth: 180
                    text: index === 0 ? qsTr("English") : index === 1 ? qsTr("Deutsch") : qsTr(
                                                                            "Español")
                    highlighted: menuContent.activeFocus && index === languageMenu.highlightedIndex
                    onClicked: {
                        root.languageSelected(languageItem.languageCode);
                        languageMenu.close();
                    }
                    contentItem: RowLayout {
                        Label {
                            text: root.language === languageItem.languageCode ? "✓" : ""
                            Layout.preferredWidth: 18
                            color: Theme.accent
                        }
                        Rectangle {
                            implicitWidth: 30
                            implicitHeight: 22
                            radius: Theme.radius / 2
                            color: Theme.hover
                            Label {
                                anchors.centerIn: parent
                                text: languageItem.languageCode.toUpperCase()
                                font.pixelSize: 10
                                font.weight: Font.DemiBold
                            }
                        }
                        Label {
                            Layout.fillWidth: true
                            text: languageItem.text
                        }
                    }
                }
            }
        }

        Timer {
            interval: 600
            running: languageMenu.visible && languageMenu.hoverOpened && !root.hovered &&
                     !menuHover.hovered
            onTriggered: languageMenu.close()
        }
    }
}
