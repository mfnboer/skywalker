import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyPage {
    required property var skywalker
    required property var timeline
    readonly property int margin: 10
    readonly property string sideBarTitle: skywalker.notificationListModel.priority ? qsTr("Priority notifcations") : qsTr("Notifications")

    signal closed

    id: page

    header: SimpleHeader {
        text: sideBarTitle
        visible: !root.showSideBar
        onBack: page.closed()
    }

    footer: SkyFooter {
        timeline: page.timeline
        skywalker: page.skywalker
        activePage: QEnums.UI_PAGE_NOTIFICATIONS
        onHomeClicked: root.viewTimeline()
        onNotificationsClicked: positionViewAtBeginning()
        onSearchClicked: root.viewSearchView()
        onMessagesClicked: root.viewChat()
        footerVisible: !root.showSideBar
    }

    SkyTabBar {
        id: tabBar
        y: !root.showSideBar ? 0 : guiSettings.headerMargin
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

            onContentYChanged: {
                const lastVisibleIndex = getLastVisibleIndex()

                if (count - lastVisibleIndex < 10 && !model?.getFeedInProgress) {
                    console.debug("Get next notification page")
                    skywalker.getNotificationsNextPage(false)
                }
            }

            FlickableRefresher {
                inProgress: allList.model?.getFeedInProgress
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
                running: allList.model?.getFeedInProgress
            }

            function doMoveToNotification(index) {
                const firstVisibleIndex = getFirstVisibleIndex()
                const lastVisibleIndex = getLastVisibleIndex()
                console.debug("Move to notification:", index, "first:", firstVisibleIndex, "last:", lastVisibleIndex, "count:", count)
                positionViewAtIndex(index, ListView.Center)
                return (firstVisibleIndex <= index && lastVisibleIndex >= index)
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

            onContentYChanged: {
                const lastVisibleIndex = getLastVisibleIndex()

                if (count - lastVisibleIndex < 10 && !model?.getFeedInProgress) {
                    console.debug("Get next mentions page")
                    skywalker.getNotificationsNextPage(true)
                }
            }

            FlickableRefresher {
                inProgress: mentionList.model?.getFeedInProgress
                topOvershootFun: () => {
                    skywalker.getNotifications(50, true, false)
                    skywalker.getNotifications(50, false, true)
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
                running: mentionList.model?.getFeedInProgress
            }

            function doMoveToMention(index) {
                const firstVisibleIndex = getFirstVisibleIndex()
                const lastVisibleIndex = getLastVisibleIndex()
                console.debug("Move to mention:", index, "first:", firstVisibleIndex, "last:", lastVisibleIndex, "count:", count)
                positionViewAtIndex(index, ListView.Center)
                return (firstVisibleIndex <= index && lastVisibleIndex >= index)
            }
        }
    }

    SvgPlainButton {
        id: moreOptions
        parent: page.header.visible ? page.header : page
        anchors.top: parent.top
        anchors.topMargin: guiSettings.headerMargin
        anchors.right: parent.right
        anchors.rightMargin: page.margin
        svg: SvgOutline.moreVert
        accessibleName: qsTr("notification options")
        onClicked: moreMenu.open()

        SkyMenu {
            id: moreMenu
            onAboutToShow: root.enablePopupShield(true)
            onAboutToHide: root.enablePopupShield(false)

            CloseMenuItem {
                text: qsTr("<b>Options</b>")
                Accessible.name: qsTr("close options menu")
            }
            AccessibleMenuItem {
                text: qsTr("Settings")
                onTriggered: root.editNotificationSettings()
                MenuItemSvg { svg: SvgOutline.settings }
            }
        }
    }

    function moveToNotification(index, mentions) {
        console.debug("Move to:", index, "mentionsOnly:", mentions)

        if (index < 0)
            return

        if (mentions)
            mentionList.moveToIndex(index, mentionList.doMoveToMention)
        else
            allList.moveToIndex(index, allList.doMoveToNotification)
    }

    function positionViewAtBeginning() {
        swipeView.currentItem.positionViewAtBeginning()
    }
}
