import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyPage {
    required property var skywalker
    required property var timeline
    readonly property int margin: 10

    signal closed

    id: page

    header: SimpleHeader {
        text: skywalker.notificationListModel.priority ? qsTr("Priority notifcations") : qsTr("Notifications")
        onBack: page.closed()

        SvgPlainButton {
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

                    onToggled: {
                        skywalker.updateNotificationPreferences(checked)
                    }
                }
            }
        }
    }

    footer: SkyFooter {
        timeline: page.timeline
        skywalker: page.skywalker
        notificationsActive: true
        onHomeClicked: root.viewTimeline()
        onNotificationsClicked: positionViewAtBeginning()
        onSearchClicked: root.viewSearchView()
        onFeedsClicked: root.viewFeedsView()
        onMessagesClicked: root.viewChat()
        visible: root.isPortrait
    }

    SkyTabBar {
        id: tabBar
        width: parent.width
        Material.background: guiSettings.backgroundColor
        leftPadding: page.margin
        rightPadding: page.margin

        AccessibleTabButton {
            id: tabAll
            text: qsTr("All")
            width: implicitWidth;
        }
        AccessibleTabButton {
            id: tabMentions
            text: qsTr("Mentions")
            width: implicitWidth;
        }
    }

    Rectangle {
        id: tabSeparator
        anchors.top: tabBar.bottom
        width: parent.width
        height: 1
        color: guiSettings.separatorColor
    }

    SwipeView {
        id: swipeView
        anchors.top: tabSeparator.bottom
        anchors.bottom: parent.bottom
        width: parent.width
        currentIndex: tabBar.currentIndex

        onCurrentIndexChanged: tabBar.setCurrentIndex(currentIndex)

        SkyListView {
            id: allList
            Layout.preferredWidth: parent.width
            Layout.preferredHeight: parent.height
            model: skywalker.notificationListModel
            clip: true

            delegate: NotificationViewDelegate {
                width: page.width
            }

            SwipeView.onIsCurrentItemChanged: {
                if (!SwipeView.isCurrentItem)
                    cover()
            }

            FlickableRefresher {
                inProgress: skywalker.getNotificationsInProgress
                topOvershootFun: () => {
                    skywalker.getNotifications(25, true, false)
                    skywalker.getNotifications(25, false, true)
                }
                bottomOvershootFun: () => skywalker.getNotificationsNextPage(false)
                topText: qsTr("Pull down to refresh")
            }

            EmptyListIndication {
                y: parent.headerItem ? parent.headerItem.height : 0
                svg: SvgOutline.noPosts
                text: skywalker.notificationListModel.priority ? qsTr("No priority notifications") : qsTr("No notifications")
                list: page
            }

            BusyIndicator {
                anchors.centerIn: parent
                running: skywalker.getNotificationsInProgress
            }

            function doMoveToNotification(index) {
                const lastVisibleIndex = getLastVisibleIndex()
                console.debug("Move to notification:", index, "last:", lastVisibleIndex)
                positionViewAtIndex(index, ListView.End)
                return (lastVisibleIndex >= index - 1 && lastVisibleIndex <= index + 1)
            }
        }

        SkyListView {
            id: mentionList
            Layout.preferredWidth: parent.width
            Layout.preferredHeight: parent.height
            model: skywalker.mentionListModel
            clip: true

            delegate: NotificationViewDelegate {
                width: page.width
            }

            SwipeView.onIsCurrentItemChanged: {
                if (!SwipeView.isCurrentItem)
                    cover()
            }

            FlickableRefresher {
                inProgress: skywalker.getMentionsInProgress
                topOvershootFun: () => {
                    skywalker.getNotifications(25, true, false)
                    skywalker.getNotifications(25, false, true)
                }
                bottomOvershootFun: () => skywalker.getNotificationsNextPage(true)
                topText: qsTr("Pull down to refresh")
            }

            EmptyListIndication {
                y: parent.headerItem ? parent.headerItem.height : 0
                svg: SvgOutline.noPosts
                text: qsTr("No mentions")
                list: page
            }

            BusyIndicator {
                anchors.centerIn: parent
                running: skywalker.getMentionsInProgress
            }

            function doMoveToMention(index) {
                const lastVisibleIndex = getLastVisibleIndex()
                console.debug("Move to mention:", index, "last:", lastVisibleIndex)
                positionViewAtIndex(index, ListView.End)
                return (lastVisibleIndex >= index - 1 && lastVisibleIndex <= index + 1)
            }
        }
    }

    function moveToNotification(index, mentions) {
        if (mentions)
            mentionList.moveToIndex(index, mentionList.doMoveToMention)
        else
            allList.moveToIndex(index, allList.doMoveToNotification)
    }

    function positionViewAtBeginning() {
        swipeView.currentItem.positionViewAtBeginning()
    }
}
