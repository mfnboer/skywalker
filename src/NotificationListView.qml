import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

ListView {
    required property var skywalker
    required property var timeline

    signal closed

    id: notificationListView
    spacing: 0
    model: skywalker.notificationListModel
    flickDeceleration: guiSettings.flickDeceleration
    ScrollIndicator.vertical: ScrollIndicator {}

    header: SimpleHeader {
        text: qsTr("Notifications")
        onBack: notificationListView.closed()
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

    FlickableRefresher {
        inProgress: skywalker.getNotificationsInProgress
        verticalOvershoot: notificationListView.verticalOvershoot
        topOvershootFun: () => skywalker.getNotifications(25, true)
        bottomOvershootFun: () => skywalker.getNotificationsNextPage()
        topText: qsTr("Pull down to refresh notifications")
    }

    SvgImage {
        id: noPostImage
        width: 150
        height: 150
        y: height + (parent.headerItem ? parent.headerItem.height : 0)
        anchors.horizontalCenter: parent.horizontalCenter
        color: Material.color(Material.Grey)
        svg: svgOutline.noPosts
        visible: notificationListView.count === 0
    }
    Text {
        id: noPostText
        y: noPostImage.y
        anchors.horizontalCenter: parent.horizontalCenter
        font.pointSize: guiSettings.scaledFont(10/8)
        color: Material.color(Material.Grey)
        elide: Text.ElideRight
        text: qsTr("No notifications")
        visible: notificationListView.count === 0
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
