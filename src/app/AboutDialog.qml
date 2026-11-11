import QtQuick
import QtQuick.Layouts
import App.Controls

Dialog {
    width: 440
    padding: Theme.spacing * 2
    standardButtons: Dialog.Close

    component DetailLabel: Label {
        Layout.fillWidth: true
        font.pixelSize: 12
        color: Theme.mutedText
        wrapMode: Text.Wrap
    }

    contentItem: ColumnLayout {
        spacing: Theme.spacing

        Image {
            Layout.alignment: Qt.AlignHCenter
            source: "icons/app.svg"
            sourceSize: Qt.size(72, 72)
        }

        Label {
            Layout.fillWidth: true
            text: Application.displayName
            font.pixelSize: 22
            font.weight: Font.DemiBold
            horizontalAlignment: Text.AlignHCenter
        }

        Label {
            Layout.fillWidth: true
            text: qsTr("Version %1").arg(Application.version)
            horizontalAlignment: Text.AlignHCenter
        }

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: details.implicitHeight + Theme.spacing * 2
            radius: Theme.radius
            color: Theme.hover
            border.color: Theme.border

            GridLayout {
                id: details
                anchors.fill: parent
                anchors.margins: Theme.spacing
                columns: 2
                columnSpacing: Theme.spacing
                rowSpacing: Theme.spacing / 2

                DetailLabel {
                    text: qsTr("Build")
                }
                DetailLabel {
                    text: "%1 · %2".arg(AppController.buildDetails.buildType,
                                        AppController.buildDetails.gitSha)
                }
                DetailLabel {
                    text: qsTr("Compiler")
                }
                DetailLabel {
                    text: AppController.buildDetails.compiler
                }
                DetailLabel {
                    text: qsTr("System")
                }
                DetailLabel {
                    text: AppController.buildDetails.system
                }
                DetailLabel {
                    text: "Qt"
                }
                DetailLabel {
                    text: AppController.buildDetails.qtVersion
                }
                DetailLabel {
                    text: qsTr("Configured")
                }
                DetailLabel {
                    text: AppController.buildDetails.timestamp
                }
            }
        }

        MenuSeparator {
            Layout.fillWidth: true
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            Button {
                text: qsTr("Source code")
                icon.source: Theme.icon("external-link")
                onClicked: {
                    if (!Qt.openUrlExternally("https://github.com/deeedob/qt-kickstart"))
                        console.warn("Could not open the source repository");
                }
            }
            Button {
                text: qsTr("Guide")
                icon.source: Theme.icon("book-open")
                onClicked: {
                    if (!Qt.openUrlExternally("https://deeedob.github.io/qt-kickstart/"))
                        console.warn("Could not open the guide");
                }
            }
        }

        Label {
            Layout.fillWidth: true
            text: qsTr(
                      "Built with Qt, used under the LGPL version 3. The licenses of all third-party components ship with the application, in its licenses folder.")
            font.pixelSize: 12
            color: Theme.mutedText
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            Button {
                text: qsTr("Open licenses folder")
                onClicked: AppController.openLicensesDirectory()
            }
            Button {
                text: qsTr("Open log folder")
                onClicked: AppController.openLogDirectory()
            }
        }
    }
}
