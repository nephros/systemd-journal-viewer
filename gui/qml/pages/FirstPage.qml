import QtQuick 2.0
import Sailfish.Silica 1.0
import org.omprussia.systemd.journal 1.0

Page {
    id: page

    Component.onCompleted: {
        journalModel.ping()
    }

    Component.onDestruction: {
        journalModel.quit()
    }

    JournalModel {
        id: journalModel
    }

    SilicaListView {
        id: listView
        model: journalModel
        anchors.fill: parent
        header: PageHeader {
            title: qsTr("Journal Viewer")
        }
        spacing: Theme.paddingSmall
        verticalLayoutDirection: ListView.BottomToTop

        PushUpMenu {
            MenuItem {
                text: "Skip tail 100"
                onClicked: {
                    journalModel.skipTail(100)
                }
            }
            MenuItem {
                text: "Add match lipstick"
                onClicked: {
                    journalModel.addMatch("_EXE=/usr/bin/lipstick")
                }
            }
            MenuItem {
                text: "Flush matches"
                onClicked: {
                    journalModel.flushMatches()
                }
            }
            MenuItem {
                text: "Save"
                onClicked: {
                    journalModel.save("/home/nemo")
                }
            }
        }

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

