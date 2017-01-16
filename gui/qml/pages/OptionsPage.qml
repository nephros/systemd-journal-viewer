import QtQuick 2.0
import Sailfish.Silica 1.0

Dialog {
    id: dialog

    property var journalModel

    ListModel {
        id: matchModel
    }

    canAccept: journalModel

    onAccepted: {
        journalModel.flushMatches()
        if (lastMessages.currentIndex = 0) {
            journalModel.skipTail(parseInt(lastNumber.text))
        } else {
            journalModel.seekTimestamp(sinceDate.selectedDate.getTime())
        }
        if (matchModel.count > 0) {
            for (var i = 0; i < matchModel.count; i++) {
                var matchItem = matchModel.get(i)
                journalModel.addMatch("%1=%2".arg(matchItem.matchKey).arg(matchItem.matchValue))
            }
        }
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: content.height

        Column {
            id: content
            width: parent.width

            DialogHeader {
                title: qsTr("Options")
                acceptText: qsTr("Apply changes")
            }

            ComboBox {
                id: lastMessages
                label: qsTr("Load previous messages")
                menu: ContextMenu {
                    MenuItem {
                        text: qsTr("N last messages")
                        onClicked: {
                            lastNumber.visible = true
                            sinceDate.visible = false
                        }
                    }
                    MenuItem {
                        text: qsTr("Messages since date")
                        onClicked: {
                            lastNumber.visible = false
                            sinceDate.visible = true
                        }
                    }
                }
            }

            TextField {
                id: lastNumber
                width: parent.width
                inputMethodHints: Qt.ImhDigitsOnly
                validator: IntValidator { bottom: 0; top: 9999 }
                text: "50"
                placeholderText: "50"
                label: qsTr("Load N last messages")
            }

            ValueButton {
                id: sinceDate
                label: qsTr("Select date and time")
                property date selectedDate: new Date()
                value: Qt.formatDateTime(selectedDate, "dd-MM-yyyy hh:mm:ss")
                visible: false
                onClicked: {
                    pageStack.push(datePickerComponent, { date: sinceDate.selectedDate })
                }
            }

            Repeater {
                model: matchModel
                delegate: Component {
                    ListItem {
                        id: delegate
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.margins: Theme.horizontalPageMargin
                        menu: ContextMenu {
                            MenuItem {
                                text: qsTr("Remove")
                                onClicked: {
                                    matchModel.remove(index)
                                }
                            }
                        }
                        contentHeight: visible ? Math.max(column.height + 2*Theme.paddingMedium, Theme.itemSizeSmall) : 0
                        Column {
                            id: column

                            anchors {
                                left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter
                                leftMargin: root.leftMargin; rightMargin: root.rightMargin
                            }
                            Flow {
                                id: flow

                                width: parent.width
                                move: Transition { NumberAnimation { properties: "x,y"; easing.type: Easing.InOutQuad; duration: root._duration } }

                                Label {
                                    id: titleText
                                    color: root.down ? Theme.highlightColor : Theme.primaryColor
                                    width: Math.min(implicitWidth + Theme.paddingMedium, parent.width)
                                    truncationMode: TruncationMode.Fade
                                    text: matchKey
                                }

                                Label {
                                    id: valueText
                                    color: Theme.highlightColor
                                    width: Math.min(implicitWidth, parent.width)
                                    truncationMode: TruncationMode.Fade
                                    text: matchValue
                                }
                            }
                        }
                    }
                }
            }

            ComboBox {
                id: newMatchFilter
                label: qsTr("Add match filter")
                currentIndex: -1
                property string defaultValue: qsTr("Click to add match filter")
                value: (currentItem !== null && currentItem.text !== defaultValue) ? currentItem.text : defaultValue
                menu: ContextMenu {
                    MenuItem {
                        text: qsTr("Syslog identifier")
                        onClicked: {
                            newMatchRule.extraKey = "SYSLOG_IDENTIFIER"
                            newMatchRule.visible = true
                        }
                    }
                    MenuItem {
                        text: qsTr("Executable path")
                        onClicked: {
                            newMatchRule.extraKey = "_EXE"
                            newMatchRule.visible = true
                        }
                    }
                    MenuItem {
                        text: qsTr("Exact message")
                        onClicked: {
                            newMatchRule.extraKey = "MESSAGE"
                            newMatchRule.visible = true
                        }
                    }
                    MenuItem {
                        text: qsTr("Custom match rule")
                        onClicked: {
                            newMatchRule.extraKey = ""
                            newMatchRule.visible = true
                        }
                    }
                }
            }

            TextField {
                id: newMatchRule
                width: parent.width
                enabled: journalModel
                property string extraKey
                visible: false
                placeholderText: label
                inputMethodHints: Qt.ImhNoAutoUppercase
                label: newMatchFilter.currentItem ? newMatchFilter.currentItem.text : ""
                EnterKey.onClicked: {
                    if (extraKey) {
                        matchModel.append({matchKey: extraKey, matchValue: text})
                        extraKey = ""
                    } else {
                        var keyIndex = text.indexOf("=")
                        if (keyIndex > 0) {
                            var matchKey = text.substr(0, keyIndex)
                            var matchValue = text.substr(keyIndex + 1)
                        }
                        matchModel.append({matchKey: matchKey, matchValue: matchValue})
                    }
                    text = ""
                    visible = false
                    newMatchFilter.currentIndex = -1
                }
            }

            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: Theme.paddingMedium

                Button {
                    text: qsTr("Clear matches")
                    enabled: journalModel
                    visible: matchModel.count > 0
                    onClicked: {
                        journalModel.flushMatches()
                        matchModel.clear()
                    }
                }

                Button {
                    text: qsTr("Clear log")
                    enabled: journalModel
                    onClicked: {
                        journalModel.clear()
                        pageStack.navigateBack()
                    }
                }
            }
        }

        VerticalScrollDecorator {}
    }



    Component {
        id: datePickerComponent
        DatePickerDialog {
            acceptDestination: timePickerComponent
            acceptDestinationProperties: {
                "time": date,
                "hour": date.getHours(),
                "minute": date.getMinutes()
            }
            acceptDestinationAction: PageStackAction.Replace
        }
    }

    Component {
        id: timePickerComponent
        TimePickerDialog {
            hourMode: DateTime.TwentyFourHours

            onAccepted: {
                var result = time
                result.setHours(hour)
                result.setMinutes(minute)
                result.setMinutes(0)
                result.setMilliseconds(0)
                sinceDate.selectedDate = result
            }
        }
    }
}
