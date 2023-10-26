import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

ListView {

    required property var skywalker
    required property var timeline
    property int margin: 8

    property bool inTopOvershoot: false
    property bool inBottomOvershoot: false

    signal closed

    id: notificationListView
    spacing: 0
    model: skywalker.notificationListModel
    ScrollIndicator.vertical: ScrollIndicator {}

    header: Rectangle {
        width: parent.width
        height: guiSettings.headerHeight
        z: guiSettings.headerZLevel
        color: guiSettings.headerColor

        RowLayout {
            id: headerRow
            width: parent.width
            height: guiSettings.headerHeight

            Text {
                id: headerTexts
                Layout.alignment: Qt.AlignVCenter
                leftPadding: 10
                font.bold: true
                font.pointSize: guiSettings.scaledFont(10/8)
                color: "white"
                text: qsTr("Notifications")
            }
        }
    }
    headerPositioning: ListView.OverlayHeader

    footer: SkyFooter {
        timeline: notificationListView.timeline
        skywalker: notificationListView.skywalker
        notificationsActive: true
        onHomeClicked: root.viewTimeline()
        onNotificationsClicked: positionViewAtBeginning()
        onSearchClicked: root.viewSearchView()
    }
    footerPositioning: ListView.OverlayFooter

    delegate: NotificationViewDelegate {
        viewWidth: notificationListView.width
    }

    onVerticalOvershootChanged: {
        if (verticalOvershoot < 0)  {
            if (!inTopOvershoot && !skywalker.getNotificationsInProgress) {
                skywalker.getNotifications(25, true)
            }

            inTopOvershoot = true
        } else {
            inTopOvershoot = false
        }

        if (verticalOvershoot > 0) {
            if (!inBottomOvershoot && !skywalker.getNotificationsInProgress) {
                skywalker.getNotificationsNextPage()
            }

            inBottomOvershoot = true;
        } else {
            inBottomOvershoot = false;
        }
    }

    BusyIndicator {
        id: busyIndicator
        anchors.centerIn: parent
        running: skywalker.getNotificationsInProgress
    }

    GuiSettings {
        id: guiSettings
    }
}
