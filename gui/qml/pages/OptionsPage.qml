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
        if (matchModel.count > 0) {
            for (var i = 0; i < matchModel.count; i++) {
                var matchItem = matchModel.get(i)
                journalModel.addMatch("%1=%2".arg(matchItem.matchKey).arg(matchItem.matchValue))
            }
        }
        if (lastMessages.currentIndex == 0) {
            journalModel.skipTail(parseInt(lastNumber.text))
        } else {
            journalModel.seekTimestamp(sinceDate.selectedDate.getTime())
        }
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: content.height

        Column {
            id: content
            width: parent.width
            spacing: Theme.paddingSmall

            DialogHeader {
                title: qsTr("Options")
                acceptText: qsTr("Apply changes")
                cancelText: qsTr("Cancel")
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
                validator: IntValidator { bottom: 0; top: 999999 }
                text: "50"
                placeholderText: "50"
                label: qsTr("Load N last messages")
            }

            ValueButton {
                id: sinceDate
                label: qsTr("Select date and time")
                property var selectedDate: new Date()
                function setDate(value) {
                    selectedDate = value
                }
                function setTime(hour, minute) {
                    selectedDate.setHours(hour)
                    selectedDate.setMinutes(minute)
                    selectedDateChanged()
                }
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
                        property string identifier: "SYSLOG_IDENTIFIER"
                    }
                    MenuItem {
                        text: qsTr("Executable path")
                        property string identifier: "_EXE"
                    }
                    MenuItem {
                        text: qsTr("Exact message")
                        property string identifier: "MESSAGE"
                    }
                    MenuItem {
                        text: qsTr("Custom match rule")
                        property string identifier: ""
                    }
                }
            }

            TextField {
                id: newMatchRule
                width: parent.width
                enabled: journalModel
                visible: newMatchFilter.currentIndex >= 0
                placeholderText: label
                inputMethodHints: Qt.ImhNoAutoUppercase
                label: newMatchFilter.currentItem ? newMatchFilter.currentItem.text : ""
                onFocusChanged: {
                    if (!focus) {
                        newMatchRule.reset()
                    }
                }
                onVisibleChanged: {
                    if (visible) {
                        forceActiveFocus()
                    }
                }
                EnterKey.onClicked: {
                    if (newMatchFilter.currentItem.identifier) {
                        matchModel.append({matchKey: newMatchFilter.currentItem.identifier, matchValue: text})
                    } else {
                        var keyIndex = text.indexOf("=")
                        if (keyIndex > 0) {
                            var matchKey = text.substr(0, keyIndex)
                            var matchValue = text.substr(keyIndex + 1)
                        }
                        matchModel.append({matchKey: matchKey, matchValue: matchValue})
                    }
                    newMatchRule.reset()
                }
                function reset() {
                    text = ""
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
            acceptDestinationAction: PageStackAction.Replace
            onAccepted: {
                sinceDate.setDate(selectedDate)
            }
            onRejected: {
                sinceDate.setDate(new Date())
            }
        }
    }

    Component {
        id: timePickerComponent
        TimePickerDialog {
            hour: sinceDate.selectedDate.getHours()
            minute: sinceDate.selectedDate.getMinutes()
            hourMode: DateTime.TwentyFourHours
            onAccepted: {
                sinceDate.setTime(hour, minute)
            }
        }
    }
}
