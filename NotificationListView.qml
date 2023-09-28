import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

ListView {

    required property var skywalker
    required property var timeline
    property int margin: 8

    property bool inTopOvershoot: false
    property bool gettingNewNotifications: false
    property bool inBottomOvershoot: false
    property bool gettingNextPage: false

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
    }
    footerPositioning: ListView.OverlayFooter

    delegate: NotificationViewDelegate {
        viewWidth: notificationListView.width
    }

    onVerticalOvershootChanged: {
        if (verticalOvershoot < 0)  {
            if (!inTopOvershoot && !skywalker.getNotificationsInProgress) {
                gettingNewNotifications = true
                skywalker.getNotifications(25, true)
            }

            inTopOvershoot = true
        } else {
            inTopOvershoot = false
        }

        if (verticalOvershoot > 0) {
            if (!inBottomOvershoot && !skywalker.getNotificationsInProgress) {
                gettingNextPage = true
                skywalker.getNotificationsNextPage()
            }

            inBottomOvershoot = true;
        } else {
            inBottomOvershoot = false;
        }
    }

    BusyIndicator {
        id: busyTopIndicator
        y: parent.y + guiSettings.headerHeight
        anchors.horizontalCenter: parent.horizontalCenter
        running: gettingNewNotifications
    }

    BusyIndicator {
        id: busyBottomIndicator
        y: parent.y + parent.height - height - guiSettings.footerHeight
        anchors.horizontalCenter: parent.horizontalCenter
        running: gettingNextPage
    }

    GuiSettings {
        id: guiSettings
    }

    Component.onCompleted: {
        skywalker.onGetNotificationsInProgressChanged.connect(() => {
            if (!skywalker.getNotificationsInProgress) {
                gettingNewNotifications = false
                gettingNextPage = false
            }
        })
    }
}
