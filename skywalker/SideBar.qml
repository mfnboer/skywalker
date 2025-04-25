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
    property var rootItem: root.currentStackItem()
    property bool isBasePage: root.currentStack().depth === 1

    signal homeClicked()
    signal notificationsClicked()
    signal searchClicked()
    signal feedsClicked()
    signal messagesClicked()
    signal addConvoClicked()

    id: sideBar
    padding: 0

    background: Rectangle {
        color: guiSettings.sideBarColor
    }

    Flickable {
        anchors.fill: parent
        clip: true
        contentWidth: parent.width
        contentHeight: sideBarColumn.height
        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds

        ColumnLayout {
            id: sideBarColumn
            width: parent.width
            spacing: 0

            PostFeedHeader {
                id: timelineHeader
                width: undefined
                height: undefined
                Layout.preferredHeight: guiSettings.sideBarHeaderHeight
                Layout.fillWidth: true
                color: guiSettings.sideBarColor
                skywalker: sideBar.skywalker
                feedName: skywalker.timelineModel.feedName
                showAsHome: true
                isHomeFeed: true
                showMoreOptions: true
                isSideBar: true
                visible: rootItem instanceof FavoritesSwipeView && rootItem.currentView instanceof TimelinePage

                onAddUserView: root.getTimelineView().addUserView()
                onAddHashtagView: root.getTimelineView().addHashtagView()
                onAddFocusHashtagView: root.getTimelineView().addFocusHashtagView()
                onAddMediaView: root.getTimelineView().showMediaView()
                onAddVideoView: root.getTimelineView().showVideoView()
            }

            PostFeedHeader {
                property var postFeedView: (rootItem instanceof FavoritesSwipeView && rootItem.currentView instanceof PostFeedView) ?
                        rootItem.currentView :
                        (rootItem instanceof PostFeedView ? rootItem : null)

                id: postFeedHeader
                width: undefined
                height: undefined
                Layout.preferredHeight: guiSettings.sideBarHeaderHeight
                Layout.fillWidth: true
                color: guiSettings.sideBarColor
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

                onClosed: postFeedView.closed()
                onFeedAvatarClicked: postFeedView.showFeed()

                onViewChanged: (newContentMode) => {
                    postFeedView.headerItem.contentMode = newContentMode
                    postFeedView.headerItem.viewChanged(newContentMode)
                }
            }

            FeedAvatar {
                property var postFeedView: rootItem instanceof PostFeedView ? rootItem : null

                Layout.topMargin: 10
                Layout.preferredWidth: 80
                Layout.preferredHeight: Layout.preferredWidth
                Layout.leftMargin: 50
                //Layout.alignment: Qt.AlignHCenter
                avatarUrl: postFeedView ? postFeedView.headerItem.feedAvatar : ""
                contentMode: postFeedView ? postFeedView.headerItem.contentMode : QEnums.CONTENT_MODE_UNSPECIFIED
                badgeOutlineColor: guiSettings.headerColor
                unknownSvg: postFeedView ? postFeedView.headerItem.defaultSvg : SvgFilled.feed
                visible: Boolean(postFeedView)

                onClicked: postFeedView.headerItem.feedAvatarClicked()

                Accessible.role: Accessible.Button
                Accessible.name: postFeedView ? postFeedView.headerItem.feedName : ""
                Accessible.description: Accessible.name
                Accessible.onPressAction: postFeedView.headerItem.feedAvatarClicked()
            }

            PostFeedHeader {
                property var searchFeedView: (rootItem instanceof FavoritesSwipeView && rootItem.currentView instanceof SearchFeedView) ? rootItem.currentView : null

                id: searchFeedHeader
                width: undefined
                height: undefined
                Layout.preferredHeight: guiSettings.sideBarHeaderHeight
                Layout.fillWidth: true
                color: guiSettings.sideBarColor
                skywalker: sideBar.skywalker
                feedName: searchFeedView ? searchFeedView.headerItem.feedName : ""
                defaultSvg: searchFeedView ? searchFeedView.headerItem.defaultSvg : SvgFilled.search
                feedAvatar: ""
                showAsHome: searchFeedView ? searchFeedView.showAsHome : false
                showLanguageFilter: searchFeedView ? searchFeedView.headerItem.showLanguageFilter : false
                filteredLanguages: searchFeedView ? searchFeedView.headerItem.filteredLanguages : []
                showPostWithMissingLanguage: false
                isSideBar: true
                visible: Boolean(searchFeedView)

                onFeedAvatarClicked: root.viewSearchViewFeed(searchFeedView.searchFeed)
            }

            MessagesListHeader {
                property var messagesListView: rootItem instanceof MessagesListView ? rootItem : null
                property convoview nullConvo

                width: undefined
                height: undefined
                Layout.preferredHeight: guiSettings.sideBarHeaderHeight
                Layout.fillWidth: true
                color: guiSettings.sideBarColor
                convo: visible ? messagesListView.convo : nullConvo
                isSideBar: true
                visible: Boolean(messagesListView)

                onBack: rootItem.closed()
            }

            SimpleHeader {
                width: undefined
                height: undefined
                Layout.preferredHeight: guiSettings.sideBarHeaderHeight
                Layout.fillWidth: true
                color: guiSettings.sideBarColor
                text: visible ? rootItem.sideBarTitle : ""
                subTitle: typeof rootItem?.sideBarSubTitle == 'string' ? rootItem.sideBarSubTitle : ""
                isSideBar: true
                visible: typeof rootItem?.sideBarTitle == 'string' && typeof rootItem.sideBarDescription == 'undefined'

                onBack: {
                    if (typeof rootItem.cancel == 'function')
                        rootItem.cancel()
                    else
                        rootItem.closed()
                }
            }

            SimpleDescriptionHeader {
                width: undefined
                height: undefined
                Layout.preferredHeight: guiSettings.sideBarHeaderHeight
                Layout.fillWidth: true
                color: guiSettings.sideBarColor
                title: visible ? rootItem.sideBarTitle : ""
                description: visible ? rootItem.sideBarDescription : ""
                isSideBar: true
                visible: typeof rootItem?.sideBarTitle == 'string' &&  typeof rootItem.sideBarDescription == 'string'

                onClosed: rootItem.closed()
            }

            SkySvg {
                Layout.topMargin: 20 + Layout.preferredHeight
                Layout.preferredWidth: 80
                Layout.preferredHeight: Layout.preferredWidth
                Layout.alignment: Qt.AlignHCenter
                svg: visible ? rootItem.sideBarSvg : SvgOutline.add
                visible: typeof rootItem?.sideBarSvg != 'undefined' && !isBasePage
            }

            Loader {
                Layout.topMargin: 20
                Layout.preferredWidth: 80
                Layout.preferredHeight: Layout.preferredWidth
                Layout.alignment: Qt.AlignHCenter
                active: typeof rootItem?.sideBarAuthor != 'undefined'
                visible: active

                sourceComponent: Avatar {
                    property basicprofile nullAuthor

                    author: visible ? rootItem.sideBarAuthor : nullAuthor
                    visible: typeof rootItem?.sideBarAuthor != 'undefined'

                    onClicked: {
                        if (!(rootItem instanceof AuthorView) || rootItem.author.did !== author.did)
                        skywalker.getDetailedProfile(author.did)
                    }
                }
            }

            Loader {
                Layout.topMargin: 20
                Layout.preferredWidth: 80
                Layout.preferredHeight: Layout.preferredWidth
                Layout.alignment: Qt.AlignHCenter
                active: typeof rootItem?.sideBarListAvatarUrl == 'string'
                visible: active

                sourceComponent: ListAvatar {
                    avatarUrl: visible ? rootItem.sideBarListAvatarUrl : ""
                    visible: typeof rootItem?.sideBarListAvatarUrl == 'string'
                }
            }

            Loader {
                Layout.topMargin: 20
                Layout.preferredWidth: 80
                Layout.preferredHeight: Layout.preferredWidth
                Layout.alignment: Qt.AlignHCenter
                active: typeof rootItem?.sideBarFeedAvatarUrl == 'string'
                visible: active

                sourceComponent: FeedAvatar {
                    avatarUrl: visible ? rootItem.sideBarFeedAvatarUrl : ""
                    visible: typeof rootItem?.sideBarFeedAvatarUrl == 'string'
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 12
                visible: isBasePage

                SkyFooterButton {
                    Layout.preferredHeight: guiSettings.sideBarHeaderHeight
                    Layout.preferredWidth: Layout.preferredHeight
                    svg: homeActive ? SvgFilled.home : SvgOutline.home
                    counter: homeActive && timeline ? timeline.unreadPosts : 0
                    counterBackgroundColor: guiSettings.sideBarColor
                    counterBorderColor: guiSettings.sideBarColor
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
                spacing: 12
                visible: isBasePage

                SkyFooterButton {
                    Layout.preferredHeight: guiSettings.sideBarHeaderHeight
                    Layout.preferredWidth: Layout.preferredHeight
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
                spacing: 12
                visible: isBasePage

                SkyFooterButton {
                    Layout.preferredHeight: guiSettings.sideBarHeaderHeight
                    Layout.preferredWidth: Layout.preferredHeight
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
                spacing: 12
                visible: isBasePage

                SkyFooterButton {
                    Layout.preferredHeight: guiSettings.sideBarHeaderHeight
                    Layout.preferredWidth: Layout.preferredHeight
                    svg: messagesActive ? SvgFilled.directMessage : SvgOutline.directMessage
                    counter: skywalker.chat.unreadCount
                    counterBorderColor: guiSettings.sideBarColor
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
                spacing: 12
                visible: isBasePage

                SkyFooterButton {
                    Layout.preferredHeight: guiSettings.sideBarHeaderHeight
                    Layout.preferredWidth: Layout.preferredHeight
                    svg: notificationsActive ? SvgFilled.notifications : SvgOutline.notifications
                    counter: root.getSkywalker().unreadNotificationCount
                    counterBorderColor: guiSettings.sideBarColor
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
                Layout.topMargin: 10
                Layout.bottomMargin: 10
                Layout.preferredWidth: 80
                Layout.preferredHeight: Layout.preferredWidth
                Layout.alignment: Qt.AlignHCenter
                author: skywalker.user
                visible: isBasePage

                onClicked: root.showSettingsDrawer()
                onPressAndHold: root.showSwitchUserDrawer()

                Accessible.role: Accessible.ButtonMenu
                Accessible.name: qsTr("Skywalker menu")
                Accessible.description: Accessible.name
                Accessible.onPressAction: clicked()
            }
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
