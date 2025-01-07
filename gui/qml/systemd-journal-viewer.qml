import QtQuick 2.0
import Sailfish.Silica 1.0
import Nemo.Notifications 1.0
import "pages"

ApplicationWindow
{
    id: appWindow

    initialPage: Component { WelcomePage { } }
    cover: Qt.resolvedUrl("cover/CoverPage.qml")
    allowedOrientations: Orientation.All
    _defaultPageOrientations: Orientation.All

    property var lastEntry: false
    function updateLastEntry(entry) {
        lastEntry = entry
    }

    function popup(message) {
      popupNotification.previewSummary = message
      popupNotification.publish()
    }
    Notification { id: popupNotification
        appName: qsTr("Journal Viewer")
        isTransient: true
        appIcon: "systemd-journal-viewer"
        icon: "icon-s-device-download"
    }
}

