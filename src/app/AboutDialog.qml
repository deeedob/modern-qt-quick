pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import App.Controls

Dialog {
    id: aboutDialog

    objectName: "aboutDialog"
    width: Math.min(480, parent.width - Theme.spacing * 2)
    padding: Theme.spacing * 2
    standardButtons: Dialog.Close
    height: Math.min(implicitHeight, parent.height - Theme.spacing * 2)

    component DetailLabel: Label {
        Layout.fillWidth: true
        color: Theme.text
        wrapMode: Text.Wrap
    }

    component InfoBox: GroupBox {
        id: infoBox

        required property color fillColor

        Layout.fillWidth: true
        padding: Theme.spacing

        label: Label {
            x: infoBox.leftPadding
            width: infoBox.availableWidth
            text: infoBox.title
            font.weight: Font.DemiBold
            horizontalAlignment: Text.AlignHCenter
            padding: Theme.spacing / 2

            background: Rectangle {
                color: infoBox.fillColor
            }
        }

        contentItem: GridLayout {
            columns: 2
            columnSpacing: Theme.spacing
            rowSpacing: Theme.spacing / 2
        }

        background: Rectangle {
            radius: Theme.radius
            color: infoBox.fillColor
            border.color: Theme.border
        }
    }

    contentItem: ScrollView {
        id: aboutScroll

        objectName: "aboutScroll"
        implicitHeight: Math.min(contentColumn.implicitHeight, 560)
        contentWidth: availableWidth

        ColumnLayout {
            id: contentColumn

            width: parent.width
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

            InfoBox {
                objectName: "buildInfoBox"
                title: qsTr("Build info")
                fillColor: Theme.surface

                DetailLabel {
                    text: qsTr("Build")
                }
                DetailLabel {
                    text: "%1 · %2".arg(AppController.buildDetails.buildType,
                                        AppController.buildDetails.gitSha)
                }
                DetailLabel {
                    text: qsTr("Configured (UTC)")
                }
                DetailLabel {
                    text: AppController.buildDetails.timestamp
                }
                DetailLabel {
                    text: qsTr("Compiler")
                }
                DetailLabel {
                    text: AppController.buildDetails.compiler
                }
                DetailLabel {
                    text: qsTr("Target platform")
                }
                DetailLabel {
                    text: AppController.buildDetails.system
                }
                DetailLabel {
                    text: qsTr("Build Qt")
                }
                DetailLabel {
                    text: AppController.buildDetails.qtVersion
                }
                DetailLabel {
                    text: qsTr("Qt linkage")
                }
                DetailLabel {
                    text: AppController.buildDetails.sharedQt ? qsTr("Shared") : qsTr("Static")
                }
            }

            InfoBox {
                objectName: "runtimeInfoBox"
                title: qsTr("Runtime info")
                fillColor: Theme.hover

                DetailLabel {
                    text: qsTr("Operating system")
                }
                DetailLabel {
                    text: AppController.runtimeInfo.operatingSystem
                }
                DetailLabel {
                    text: qsTr("Kernel version")
                }
                DetailLabel {
                    text: AppController.runtimeInfo.kernelVersion
                }
                DetailLabel {
                    text: qsTr("Window system")
                }
                DetailLabel {
                    text: AppController.runtimeInfo.windowSystem
                }
            }

            MenuSeparator {
                Layout.fillWidth: true
            }

            ColumnLayout {
                Layout.fillWidth: true

                Button {
                    Layout.fillWidth: true
                    text: qsTr("Open log folder")
                    onClicked: AppController.openLogDirectory()
                }
                Button {
                    Layout.fillWidth: true
                    text: qsTr("Open licenses folder")
                    onClicked: AppController.openLicensesDirectory()
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

            // template:begin
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
            // template:end
        }
    }
}
