import QtQuick
import QtQuick.Layouts
import App.Controls

Dialog {
    width: 360
    padding: Theme.spacing * 2
    standardButtons: Dialog.Close

    component DetailLabel: Label {
        Layout.fillWidth: true
        font.pixelSize: 12
        color: Theme.mutedText
        wrapMode: Text.Wrap
        horizontalAlignment: Text.AlignHCenter
    }

    contentItem: ColumnLayout {
        spacing: Theme.spacing

        Image {
            Layout.alignment: Qt.AlignHCenter
            source: "app.svg"
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

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 0

            DetailLabel {
                text: "%1 · %2".arg(AppController.buildDetails.gitSha,
                                    AppController.buildDetails.buildType)
            }

            DetailLabel {
                text: "%1 · Qt %2".arg(AppController.buildDetails.compiler,
                                       AppController.buildDetails.qtVersion)
            }

            DetailLabel {
                text: AppController.buildDetails.system
            }

            DetailLabel {
                text: qsTr("Configured: %1").arg(AppController.buildDetails.timestamp)
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

        Button {
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("Open licenses folder")
            onClicked: AppController.openLicensesDirectory()
        }

        Button {
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("Open log folder")
            onClicked: AppController.openLogDirectory()
        }
    }
}
