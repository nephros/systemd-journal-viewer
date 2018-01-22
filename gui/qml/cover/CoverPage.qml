import QtQuick 2.0
import Sailfish.Silica 1.0

CoverBackground {
    Item {
        id: fillItem
        anchors.fill: parent
        anchors.topMargin: Theme.paddingSmall
        clip: true

        Column {
            id: journalColumn
            anchors.fill: parent
            anchors.leftMargin: Theme.horizontalPageMargin / 2
            anchors.rightMargin: Theme.horizontalPageMargin / 2

            Label {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Journal Viewer")
                color: Theme.secondaryColor
                font.family: "Courier"
                font.pixelSize: Theme.fontSizeExtraSmall
            }

            Label {
                width: parent.width
                wrapMode: Text.WrapAnywhere
                text: lastEntry ? Qt.formatDateTime(new Date(lastEntry["__TIMESTAMP"]), "hh:mm:ss.zzz") : ""
                color: Theme.secondaryColor
                font.family: "Courier"
                font.pixelSize: Theme.fontSizeTiny
                visible: lastEntry && !!lastEntry["__TIMESTAMP"]
            }

            Label {
                width: parent.width
                wrapMode: Text.WrapAnywhere
                text: (lastEntry && !!lastEntry["SYSLOG_IDENTIFIER"] ? lastEntry["SYSLOG_IDENTIFIER"] : "")
                      + ":"
                      + (!!lastEntry["_PID"] ? lastEntry["_PID"] : "")
                color: Theme.primaryColor
                font.family: "Courier"
                font.pixelSize: Theme.fontSizeTiny
                visible: lastEntry && (!!lastEntry["SYSLOG_IDENTIFIER"] || !!lastEntry["_PID"])
                font.bold: true
            }

            Label {
                width: parent.width
                wrapMode: Text.WrapAnywhere
                text: (lastEntry && !!lastEntry["CODE_FILE"] ? lastEntry["CODE_FILE"] : "")
                      + ":"
                      + (!!lastEntry["CODE_FUNC"] ? lastEntry["CODE_FUNC"] : "")
                color: Theme.primaryColor
                font.family: "Courier"
                font.pixelSize: Theme.fontSizeTiny
                visible: lastEntry && (!!lastEntry["CODE_FUNC"] || !!lastEntry["CODE_FILE"])
            }

            Label {
                width: parent.width
                wrapMode: Text.WrapAnywhere
                text: lastEntry ? !!lastEntry["MESSAGE"] : ""
                visible: lastEntry && !!lastEntry["MESSAGE"]
                color: Theme.primaryColor
                font.family: "Courier"
                font.pixelSize: Theme.fontSizeTiny
            }
        }
    }

    OpacityRampEffect {
        id: rampEffect
        direction: OpacityRamp.TopToBottom
        sourceItem: fillItem
        slope: 2
        offset: 0.5
        enabled: journalColumn.height > fillItem.height
    }
}

