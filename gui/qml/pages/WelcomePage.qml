import QtQuick 2.0
import Sailfish.Silica 1.0
import org.coderus.systemd.journal 1.0
import Nemo.DBus 2.0

Page {
    id: page

    DBusInterface {
        id: dbus
        service: "org.coderus.systemd.journal"
        path: "/"
        iface: "org.coderus.systemd.journal"
    }

    BusyLabel { id: busyLabel
        text: qsTr("Saving…")
        running: saveTimer.running
    }

    Timer {
        id: saveTimer
        repeat: false
        interval: 3000
        property string notification: qsTr("Journal saved to Documents!")
        onTriggered: {
            popup(notification)
        }
    }
    PageHeader {
        title: qsTr("System Journal")
    }
    Column {
        id: content
        anchors.centerIn: parent
        width: parent.width
        spacing: Theme.paddingLarge

        opacity: busyLabel.running ? Theme.opacityFaint : 1.0
        Behavior on opacity { FadeAnimator {} }

        Button {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Read logs")
            onClicked: pageStack.replace(Qt.resolvedUrl("FirstPage.qml"))
        }

        Button {
            id: saveButton
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Save journal")
            enabled: !saveTimer.running
            onClicked: {
                dbus.call("saveJournal", [])
                saveTimer.start()
            }
        }
    }
}
