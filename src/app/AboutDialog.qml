import QtQuick
import QtQuick.Layouts
import App.Controls

Dialog {
    width: 360
    padding: Theme.spacing * 2
    standardButtons: Dialog.Close

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

        Label {
            Layout.fillWidth: true
            text: AppController.buildDetails
            font.pixelSize: 12
            color: Theme.mutedText
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
        }

        MenuSeparator {
            Layout.fillWidth: true
        }

        Label {
            Layout.fillWidth: true
            text: qsTr("Built with Qt, used under the LGPL version 3. The licenses of all third-party components ship with the application, in its licenses folder.")
            font.pixelSize: 12
            color: Theme.mutedText
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
        }

        Button {
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("Open licenses folder")
            flat: true
            onClicked: AppController.openLicensesDirectory()
        }

        Button {
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("Open log folder")
            flat: true
            onClicked: AppController.openLogDirectory()
        }
    }
}
