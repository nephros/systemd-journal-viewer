import QtQuick 2.0
import Sailfish.Silica 1.0
import org.nemomobile.dbus 2.0
import org.omprussia.systemd.journal 1.0

Page {
    id: page

    Component.onDestruction: {
        journal.call("quit", [])
    }

    onStatusChanged: {
        if (status == PageStatus.Active) {
            journal.call("ping", [])
        }
    }

    DBusInterface {
        id: journal
        bus: DBus.SessionBus
        service: "ru.omprussia.systemd.journal"
        iface: "ru.omprussia.systemd.journal"
        path: "/"
        signalsEnabled: true

        function dataReceived(journalData) {
            listView.model.insert(0, journalData)
            var logLine = "%1 %2: %3".arg(Qt.formatDateTime(new Date(journalData["__TIMESTAMP"] / 1000), "dd.MM.yy hh:mm:ss.zzz"))
                                     .arg(journalData["SYSLOG_IDENTIFIER"] + (!!journalData["_PID"] ? ("[" + journalData["_PID"] + "]") : ""))
                                     .arg((!!journalData["CODE_FILE"] ? (journalData["CODE_FILE"] + ":" + journalData["CODE_FUNC"] + " ") : "")
                                          + journalData["MESSAGE"])
            fileLog.writeLine(logLine)
        }
    }

    FileLog {
        id: fileLog
        fileName: "journal-log-%1".arg(Qt.formatDateTime(new Date(), "ddMMyy-hhmmss-zzz"))
    }

    SilicaListView {
        id: listView
        model: ListModel {}
        anchors.fill: parent
        header: PageHeader {
            title: fileLog.fileName
        }
        spacing: Theme.paddingSmall
        verticalLayoutDirection: ListView.BottomToTop

        delegate: BackgroundItem {
            id: delegate
            contentHeight: journalColumn.height
            height: journalColumn.height

            Column {
                id: journalColumn
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.margins: Theme.horizontalPageMargin

                Label {
                    width: parent.width
                    wrapMode: Text.WrapAnywhere
                    text: (!!model["SYSLOG_IDENTIFIER"] ? model["SYSLOG_IDENTIFIER"] : "")
                          + ":"
                          + (!!model["_PID"] ? model["_PID"] : "")
                    color: delegate.highlighted ? Theme.highlightColor : Theme.primaryColor
                    font.family: "Courier"
                    font.pixelSize: Theme.fontSizeTiny
                    visible: !!model["SYSLOG_IDENTIFIER"] || !!model["_PID"]
                    font.bold :true
                }

                Label {
                    width: parent.width
                    wrapMode: Text.WrapAnywhere
                    text: (!!model["CODE_FILE"] ? model["CODE_FILE"] : "")
                          + ":"
                          + (!!model["CODE_FUNC"] ? model["CODE_FUNC"] : "")
                    color: delegate.highlighted ? Theme.highlightColor : Theme.primaryColor
                    font.family: "Courier"
                    font.pixelSize: Theme.fontSizeTiny
                    visible: !!model["CODE_FUNC"] || !!model["CODE_FILE"]
                }

                Label {
                    width: parent.width
                    wrapMode: Text.WrapAnywhere
                    text: model["MESSAGE"]
                    color: delegate.highlighted ? Theme.highlightColor : Theme.primaryColor
                    font.family: "Courier"
                    font.pixelSize: Theme.fontSizeTiny
                }
            }
        }
        VerticalScrollDecorator {}
    }
}

