import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0
import QtQuick.Window 2.2
import AppUpdateController 1.0

Window {
    id: root
    property int space: 10
    property real progressValue: 0
    width: 600
    height: 400

    AppUpdateController {
        id: updater
    }

    Rectangle {
        anchors.fill: parent
        color: "gray"
    }

    Column {
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 10
        Row {
            spacing: 20
            Image {
                width: 60
                height: width
                source: "qrc:/icons/nupdate.png"
            }
            Column {
                anchors.verticalCenter: parent.verticalCenter
                Label {
                    text: "ONLINE UPDATE"
                }

                Label {
                    text: qsTr("current ver: %1").arg(updater.getAppVersion())
                }
            }
        }

        GroupBox {

            title: qsTr("update setting")
            Column {
                spacing: 5

                CheckBox {
                    text: "Show all notifications"
                    checked: updater.notifyFinish
                    onClicked: {
                      updater.notifyFinish = !checked
                    }
                }
                CheckBox {
                    text: "Notify me when an update is available"
                    checked: updater.notifyUpdate
                }
                CheckBox {
                    text: "Enable integrated downloader"
                    checked: updater.downloadEnabled
                }
                CheckBox {
                    text: "Do not use the QSU library to read the appcast"
                    checked: updater.useCustomAppcast
                }
                CheckBox {
                    text: "Mandatory Update"
                    checked: updater.mandatoryUpdate
                }
            }
        }

        GroupBox {

            title: qsTr("Change Log")
            TextEdit {
                     id: edit
                     focus: true
                     wrapMode: TextEdit.Wrap
                     text: updater.changeLog.length >0?updater.changeLog :qsTr("no log")
            }

        }
    }

    Row  {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20
        spacing: 10
        Button {
            text:"help"
        }
        Button {

            text: "CheckForUpdates"
            onClicked: updater.checkForUpdates()

        }
    }

}

