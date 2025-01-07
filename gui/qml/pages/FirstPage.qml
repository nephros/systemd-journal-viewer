import QtQuick 2.0
import Sailfish.Silica 1.0
import Nemo.DBus 2.0
import org.coderus.systemd.journal 1.0

Page {
    id: page

    onStatusChanged: {
        if (status === PageStatus.Active && pageStack.depth === 1 && !forwardNavigation) {
            pageStack.pushAttached("OptionsPage.qml", {
                                       journalModel: journalModel,
                                       acceptDestination: page,
                                       acceptDestinationAction: PageStackAction.Pop
                                   });
        }
    }

    Component.onCompleted: {
        journalModel.ping()
    }

    Component.onDestruction: {
        journalModel.quit()
    }

    DBusInterface {
        id: dbus
        service: "org.coderus.systemd.journal"
        path: "/"
        iface: "org.coderus.systemd.journal"
    }

    JournalModel {
        id: journalModel
        onLastEntryChanged: {
            appWindow.updateLastEntry(journalModel.lastEntry)
        }
    }

    BusyLabel { id: busyLabel
        text: qsTr("Saving…")
    }

    Timer {
        id: popupTimer
        interval: 3000
        repeat: false
        property string message
        onRunningChanged: busyLabel.running = running
        onTriggered: popup(message)
    }

    PageHeader { id: header
        z: 100
        clip: true
        title: qsTr("Journal Viewer")
    }

    SilicaListView {
        id: listView
        model: journalModel

        anchors.top: header.bottom
        width: parent.width
        height: parent.height - header.height
        clip: true

        spacing: Theme.paddingSmall
        verticalLayoutDirection: ListView.BottomToTop

        opacity: busyLabel.running ? Theme.opacityFaint : 1.0
        Behavior on opacity { FadeAnimator {} }

        PushUpMenu {
            MenuItem {
                text: qsTr("Save journal database")
                onClicked: {
                    dbus.call("saveJournal", [])
                    popupTimer.message = qsTr("Journal saved to Documents!")
                    popupTimer.start()
                }
            }

            MenuItem {
                text: qsTr("Save log to file")
                onClicked: {
                    var dialog = pageStack.push(Qt.resolvedUrl("FileBrowser.qml"), {
                                                    acceptDestination: page,
                                                    acceptDestinationAction: PageStackAction.Pop,
                                                    callback: function(path) {
                                                        popupTimer.message = qsTr("Log saved to %1!").arg(path)
                                                        popupTimer.start()
                                                        journalModel.save(path)
                                                    }
                                                })
                }
            }
        }

        delegate: ListItem {
            id: delegate
            contentHeight: journalColumn.height

            menu: ContextMenu {
                MenuLabel {
                    text: Qt.formatDateTime(new Date(model["__TIMESTAMP"]), "dd.MM.yy hh:mm:ss.zzz")
                }
                MenuItem {
                    text: qsTr("Copy to clipboard")
                    onClicked: {
                        journalModel.copyItem(index)
                    }
                }
            }

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
                    font.bold: true
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

        ViewPlaceholder {
             enabled: listView.count == 0
             text: qsTr("Waiting for messages…")
             hintText: qsTr("Swipe right to adjust filters.")
        }

        VerticalScrollDecorator {}
    }
}

