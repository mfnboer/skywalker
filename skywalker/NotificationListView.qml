import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyListView {
    required property var skywalker
    required property var timeline

    signal closed

    id: notificationListView
    model: skywalker.notificationListModel

    header: SimpleHeader {
        text: skywalker.notificationListModel.priority ? qsTr("Priority notifcations") : qsTr("Notifications")
        onBack: notificationListView.closed()

        SvgButton {
            id: moreOptions
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            svg: SvgOutline.moreVert
            accessibleName: qsTr("notification options")
            onClicked: moreMenu.open()

            Menu {
                id: moreMenu
                modal: true

                onAboutToShow: root.enablePopupShield(true)
                onAboutToHide: root.enablePopupShield(false)

                CloseMenuItem {
                    text: qsTr("<b>Options</b>")
                    Accessible.name: qsTr("close options menu")
                }
                MenuWithInfoItem {
                    text: qsTr("Priority only")
                    info: qsTr("When priority only is enabled, replies and quotes from users you do not follow will not be shown.")
                    checkable: true
                    checked: skywalker.notificationListModel.priority

                    onToggled:{
                        skywalker.updateNotificationPreferences(checked)
                    }
                }
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
        onFeedsClicked: root.viewFeedsView()
        onMessagesClicked: root.viewChat()
    }
    footerPositioning: ListView.OverlayFooter

    delegate: NotificationViewDelegate {
        width: notificationListView.width
    }

    FlickableRefresher {
        inProgress: skywalker.getNotificationsInProgress
        topOvershootFun: () => skywalker.getNotifications(25, true)
        bottomOvershootFun: () => skywalker.getNotificationsNextPage()
        topText: qsTr("Pull down to refresh notifications")
    }

    EmptyListIndication {
        y: parent.headerItem ? parent.headerItem.height : 0
        svg: SvgOutline.noPosts
        text: skywalker.notificationListModel.priority ? qsTr("No priority notifications") : qsTr("No notifications")
        list: notificationListView
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
