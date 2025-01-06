import QtQuick 2.0
import Sailfish.Silica 1.0
import org.coderus.systemd.journal 1.0
import org.nemomobile.dbus 2.0

Page {
    id: page

    DBusInterface {
        id: dbus
        service: "org.coderus.systemd.journal"
        path: "/"
        iface: "org.coderus.systemd.journal"
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: content.height

        Column {
            id: content
            width: parent.width
            spacing: Theme.paddingSmall

            PageHeader {
                title: qsTr("System Journal")
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Read logs")
                onClicked: pageStack.replace(Qt.resolvedUrl("FirstPage.qml"))
            }

            Button {
                id: saveButton
                anchors.horizontalCenter: parent.horizontalCenter
                property string label: qsTr("Save journal")
                property string notification: qsTr("Journal saved to Documents!")
                text: label
                onClicked: {
                    dbus.call("saveJournal", [])
                    saveTimer.start()
                    text = notification
                }

                Timer {
                    id: saveTimer
                    repeat: false
                    interval: 3000
                    onTriggered: {
                        saveButton.text =saveButton.label
                    }
                }
            }
        }
    }
}
