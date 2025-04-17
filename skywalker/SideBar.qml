import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Pane {
    required property var timeline
    property var skywalker: root.getSkywalker()
    property bool homeActive: false
    property bool notificationsActive: false
    property bool searchActive: false
    property bool feedsActive: false
    property bool messagesActive: false
    property bool showHomeFeedBadge: false
    property bool floatingButtons: false

    signal homeClicked()
    signal notificationsClicked()
    signal searchClicked()
    signal feedsClicked()
    signal messagesClicked()
    signal addConvoClicked()

    id: sideBar
    padding: 0

    ColumnLayout {
        width: parent.width

        PostFeedHeader {
            id: timelineHeader
            width: undefined
            height: undefined
            Layout.preferredHeight: 100
            Layout.fillWidth: true
            skywalker: sideBar.skywalker
            feedName: skywalker.timelineModel.feedName
            showAsHome: true
            isHomeFeed: true
            showMoreOptions: true
            isSideBar: true
            visible: root.currentStackItem() instanceof FavoritesSwipeView && root.currentStackItem().currentView instanceof TimelinePage

            onAddUserView: root.getTimelineView().addUserView()
            onAddHashtagView: root.getTimelineView().addHashtagView()
            onAddFocusHashtagView: root.getTimelineView().addFocusHashtagView()
            onAddMediaView: root.getTimelineView().showMediaView()
            onAddVideoView: root.getTimelineView().showVideoView()
        }

        PostFeedHeader {
            property var postFeedView: (root.currentStackItem() instanceof FavoritesSwipeView && root.currentStackItem().currentView instanceof PostFeedView) ? root.currentStackItem().currentView : null

            id: postFeedHeader
            width: undefined
            height: undefined
            Layout.preferredHeight: 100
            Layout.fillWidth: true
            skywalker: sideBar.skywalker
            feedName: postFeedView ? postFeedView.headerItem.feedName : ""
            feedAvatar: postFeedView ? postFeedView.headerItem.feedAvatar : ""
            defaultSvg: postFeedView ? postFeedView.headerItem.defaultSvg : SvgFilled.feed
            contentMode: postFeedView ? postFeedView.headerItem.contentMode : QEnums.CONTENT_MODE_UNSPECIFIED
            underlyingContentMode: postFeedView ? postFeedView.headerItem.underlyingContentMode : QEnums.CONTENT_MODE_UNSPECIFIED
            showAsHome: postFeedView ? postFeedView.showAsHome : false
            showLanguageFilter: postFeedView ? postFeedView.headerItem.showLanguageFilter : false
            filteredLanguages: postFeedView ? postFeedView.headerItem.filteredLanguages : []
            showPostWithMissingLanguage: postFeedView ? postFeedView.headerItem.showPostWithMissingLanguage : true
            showViewOptions: true
            isSideBar: true
            visible: Boolean(postFeedView)

            onViewChanged: (newContentMode) => {
                postFeedView.headerItem.contentMode = newContentMode
                postFeedView.headerItem.viewChanged(newContentMode)
            }
        }

        Item {
            Layout.preferredHeight: 100
            visible: !timelineHeader.visible && !postFeedHeader.visible
        }

        RowLayout {
            Layout.fillWidth: true

            SkyFooterButton {
                Layout.preferredHeight: 50
                Layout.preferredWidth: 50
                svg: homeActive ? SvgFilled.home : SvgOutline.home
                counter: homeActive && timeline ? timeline.unreadPosts : 0
                counterBackgroundColor: guiSettings.backgroundColor
                counterTextColor: guiSettings.textColor
                showAltBadge: showHomeFeedBadge
                altBadgeSvg: SvgOutline.feed
                Accessible.name: getHomeSpeech()
                onClicked: homeClicked()
            }

            AccessibleText {
                Layout.fillWidth: true
                elide: Text.ElideRight
                font.bold: homeActive
                text: qsTr("Home")

                MouseArea {
                    anchors.fill: parent
                    onClicked: homeClicked()
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true

            SkyFooterButton {
                Layout.preferredHeight: 50
                Layout.preferredWidth: 50
                svg: searchActive ? SvgFilled.search : SvgOutline.search
                Accessible.name: qsTr("search")
                onClicked: searchClicked()
            }

            AccessibleText {
                Layout.fillWidth: true
                elide: Text.ElideRight
                font.bold: searchActive
                text: qsTr("Search")

                MouseArea {
                    anchors.fill: parent
                    onClicked: searchClicked()
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true

            SkyFooterButton {
                Layout.preferredHeight: 50
                Layout.preferredWidth: 50
                svg: feedsActive ? SvgFilled.feed : SvgOutline.feed
                Accessible.name: qsTr("feeds")
                onClicked: feedsClicked()
            }

            AccessibleText {
                Layout.fillWidth: true
                elide: Text.ElideRight
                font.bold: feedsActive
                text: qsTr("Feeds")

                MouseArea {
                    anchors.fill: parent
                    onClicked: feedsClicked()
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true

            SkyFooterButton {
                Layout.preferredHeight: 50
                Layout.preferredWidth: 50
                svg: messagesActive ? SvgFilled.directMessage : SvgOutline.directMessage
                counter: skywalker.chat.unreadCount
                Accessible.name: skywalker.chat.unreadCount === 0 ? qsTr("direct messages") : qsTr(`${skywalker.chat.unreadCount} new direct messages`)
                onClicked: messagesClicked()
            }

            AccessibleText {
                Layout.fillWidth: true
                elide: Text.ElideRight
                font.bold: messagesActive
                text: qsTr("Direct messages")

                MouseArea {
                    anchors.fill: parent
                    onClicked: messagesClicked()
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true

            SkyFooterButton {
                Layout.preferredHeight: 50
                Layout.preferredWidth: 50
                svg: notificationsActive ? SvgFilled.notifications : SvgOutline.notifications
                counter: root.getSkywalker().unreadNotificationCount
                Accessible.name: root.getSkywalker().unreadNotificationCount === 0 ? qsTr("notifications") : qsTr(`${skywalker.unreadNotificationCount} new notifications`)
                onClicked: notificationsClicked()
            }

            AccessibleText {
                Layout.fillWidth: true
                elide: Text.ElideRight
                font.bold: notificationsActive
                text: qsTr("Notifications")

                MouseArea {
                    anchors.fill: parent
                    onClicked: notificationsClicked()
                }
            }
        }

        Avatar {
            Layout.topMargin: 20
            Layout.preferredWidth: 80
            Layout.preferredHeight: Layout.preferredWidth
            Layout.alignment: Qt.AlignHCenter
            author: skywalker.user
            onClicked: root.showSettingsDrawer()
            onPressAndHold: root.showSwitchUserDrawer()

            Accessible.role: Accessible.ButtonMenu
            Accessible.name: qsTr("Skywalker menu")
            Accessible.description: Accessible.name
            Accessible.onPressAction: clicked()
        }
    }

    function getHomeSpeech() {
        if (!homeActive)
            return qsTr("show feed")

        if (!timeline)
            return ""

        if (timeline.unreadPosts === 0)
            return qsTr("You are at the top of your time line")

        if (timeline.unreadPosts === 1)
            return qsTr("1 post from the top of your time line")

        return qsTr(`${timeline.unreadPosts} posts from top of your timeline, press to go to top`)
    }
}
