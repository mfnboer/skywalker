import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Pane {
    required property var timeline
    property Skywalker skywalker: root.getSkywalker()
    property UserSettings userSettings: skywalker.getUserSettings()
    property bool homeActive: false
    property bool notificationsActive: false
    property bool searchActive: false
    property bool messagesActive: false
    property var rootItem: root.currentStackItem()
    property bool isBasePage: root.currentStack().depth === 1
    readonly property list<favoritefeedview> favorites: skywalker.favoriteFeeds.userOrderedPinnedFeeds
    readonly property var postFeedView: (rootItem instanceof FavoritesSwipeView && rootItem.currentView instanceof PostFeedView) ?
            rootItem.currentView :
            (rootItem instanceof PostFeedView ? rootItem : null)
    readonly property var searchFeedView: (rootItem instanceof FavoritesSwipeView && rootItem.currentView instanceof SearchFeedView) ? rootItem.currentView : null
    readonly property var activeFeedView: postFeedView ? postFeedView : (searchFeedView ? searchFeedView : null)


    signal homeClicked()
    signal notificationsClicked()
    signal searchClicked()
    signal messagesClicked()
    signal addConvoClicked()

    id: sideBar
    padding: 0

    background: Rectangle {
        color: guiSettings.sideBarColor
    }

    RowLayout {
        id: topRow
        width: parent.width

        Avatar {
            Layout.leftMargin: 10
            Layout.topMargin: 10
            Layout.bottomMargin: 10
            Layout.preferredWidth: 40
            author: skywalker.user
            visible: isBasePage

            onClicked: root.showSettingsDrawer()
            onPressAndHold: root.showSwitchUserDrawer()

            Accessible.role: Accessible.ButtonMenu
            Accessible.name: qsTr("Skywalker menu")
            Accessible.description: Accessible.name
            Accessible.onPressAction: clicked()
        }

        Item {
            Layout.fillWidth: true
        }

        LanguageFilterButton {
            filteredLanguages: activeFeedView ? activeFeedView.headerItem.filteredLanguages : []
            showPostWithMissingLanguage: activeFeedView ? activeFeedView.headerItem.showPostWithMissingLanguage : true
            visible: activeFeedView && activeFeedView.headerItem.showLanguageFilter
        }

        // View mode
        SvgPlainButton {
            svg: activeFeedView ? guiSettings.getContentModeSvg(activeFeedView.headerItem.contentMode) : SvgOutline.chat
            iconColor: guiSettings.headerTextColor
            accessibleName: qsTr("view mode")
            visible: Boolean(activeFeedView)

            onClicked: feedViewMenu.open()

            ContentViewMenu {
                id: feedViewMenu
                contentMode: activeFeedView ? activeFeedView.headerItem.contentMode : QEnums.CONTENT_MODE_UNSPECIFIED
                underlyingContentMode: activeFeedView ? activeFeedView.headerItem.underlyingContentMode : QEnums.CONTENT_MODE_UNSPECIFIED

                onViewChanged: (newContentMode) => {
                    activeFeedView.headerItem.contentMode = newContentMode
                    activeFeedView.headerItem.viewChanged(newContentMode)
                }
            }
        }

        // Home
        SkyFooterButton {
            Layout.rightMargin: 10
            Layout.preferredHeight: guiSettings.sideBarHeaderHeight
            Layout.preferredWidth: Layout.preferredHeight
            svg: homeActive ? (timeline && timeline.reverseFeed ? SvgFilled.homeUnderlined : SvgFilled.home) : SvgOutline.home
            counter: homeActive && timeline ? timeline.unreadPosts : 0
            counterBackgroundColor: guiSettings.sideBarColor
            counterBorderColor: guiSettings.sideBarColor
            counterTextColor: guiSettings.textColor
            Accessible.name: getHomeSpeech()
            visible: isBasePage
            onClicked: homeClicked()
        }
    }

    Flickable {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: topRow.bottom
        anchors.bottom: postButton.top
        anchors.bottomMargin: 10
        clip: true
        contentWidth: parent.width
        contentHeight: sideBarColumn.height
        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds

        ColumnLayout {
            id: sideBarColumn
            width: parent.width
            spacing: 0

            FeedAvatar {
                property var postFeedView: rootItem instanceof PostFeedView ? rootItem : null

                Layout.topMargin: 10
                Layout.preferredWidth: 80
                Layout.preferredHeight: Layout.preferredWidth
                Layout.leftMargin: 50
                avatarUrl: postFeedView ? postFeedView.headerItem.feedAvatar : ""
                contentMode: postFeedView ? postFeedView.headerItem.contentMode : QEnums.CONTENT_MODE_UNSPECIFIED
                badgeOutlineColor: guiSettings.headerColor
                unknownSvg: postFeedView ? postFeedView.headerItem.defaultSvg : SvgFilled.feed
                visible: Boolean(postFeedView)

                onClicked: (clickPoint) => {
                    const mousePoint = clickPoint ?
                        mapToItem(postFeedView.headerItem, clickPoint) :
                        mapToItem(postFeedView.headerItem, 0, 0)

                    postFeedView.headerItem.feedAvatarClicked(mousePoint)
                }

                Accessible.role: Accessible.Button
                Accessible.name: postFeedView ? postFeedView.headerItem.feedName : ""
                Accessible.description: Accessible.name
                Accessible.onPressAction: postFeedView.headerItem.feedAvatarClicked(Qt.point(0, 0))
            }

            MessagesListHeader {
                property var messagesListView: rootItem instanceof MessagesListView ? rootItem : null
                property convoview nullConvo

                width: undefined
                height: undefined
                Layout.preferredHeight: guiSettings.sideBarHeaderHeight
                Layout.fillWidth: true
                rightPadding: messagesHeaderButton.visible ? messagesHeaderButton.width : 0
                color: guiSettings.sideBarColor
                convo: visible ? messagesListView.convo : nullConvo
                isSideBar: true
                visible: Boolean(messagesListView)

                onBack: rootItem.closed()

                SvgPlainButton {
                    id: messagesHeaderButton
                    anchors.right: parent.right
                    anchors.top: parent.top
                    svg: visible ? rootItem.sideBarButtonSvg : SvgOutline.info
                    accessibleName: typeof rootItem?.sideBarButtonName == 'string' ? rootItem.sideBarButtonName : ""
                    visible: typeof rootItem?.sideBarButtonSvg != 'undefined'
                    onClicked: typeof rootItem?.sideBarButtonClicked == 'function' ? rootItem.sideBarButtonClicked(simpleHeader, Qt.point(x, y)) : () => {}
                }
            }

            SimpleHeader {
                id: simpleHeader
                width: undefined
                height: undefined
                Layout.preferredHeight: guiSettings.sideBarHeaderHeight
                Layout.fillWidth: true
                rightPadding: simpleHeaderButton.visible ? simpleHeaderButton.width : 0
                color: guiSettings.sideBarColor
                text: visible ? rootItem.sideBarTitle : ""
                userDid: typeof rootItem?.userDid == 'string' ? rootItem.userDid : ""
                subTitle: typeof rootItem?.sideBarSubTitle == 'string' ? rootItem.sideBarSubTitle : ""
                isSideBar: true
                visible: typeof rootItem?.sideBarTitle == 'string' && typeof rootItem.sideBarDescription == 'undefined'

                SvgPlainButton {
                    id: simpleHeaderButton
                    anchors.right: parent.right
                    anchors.top: parent.top
                    svg: visible ? rootItem.sideBarButtonSvg : SvgOutline.info
                    accessibleName: typeof rootItem?.sideBarButtonName == 'string' ? rootItem.sideBarButtonName : ""
                    visible: typeof rootItem?.sideBarButtonSvg != 'undefined'
                    onClicked: typeof rootItem?.sideBarButtonClicked == 'function' ? rootItem.sideBarButtonClicked(simpleHeader, Qt.point(x, y)) : () => {}
                }

                onBack: {
                    if (typeof rootItem.cancel == 'function')
                        rootItem.cancel()
                    else
                        rootItem.closed()
                }
            }

            SimpleDescriptionHeader {
                id: simpleDescriptionHeader
                width: undefined
                height: undefined
                Layout.preferredHeight: guiSettings.sideBarHeaderHeight
                Layout.fillWidth: true
                rightPadding: simpleDescriptionHeaderButton.visible ? simpleDescriptionHeaderButton.width : 0
                color: guiSettings.sideBarColor
                title: visible ? rootItem.sideBarTitle : ""
                description: visible ? rootItem.sideBarDescription : ""
                userDid: typeof rootItem?.userDid == 'string' ? rootItem.userDid : ""
                isSideBar: true
                visible: typeof rootItem?.sideBarTitle == 'string' &&  typeof rootItem.sideBarDescription == 'string'

                SvgPlainButton {
                    id: simpleDescriptionHeaderButton
                    anchors.right: parent.right
                    anchors.top: parent.top
                    svg: visible ? rootItem.sideBarButtonSvg : SvgOutline.info
                    accessibleName: typeof rootItem?.sideBarButtonName == 'string' ? rootItem.sideBarButtonName : ""
                    visible: typeof rootItem?.sideBarButtonSvg != 'undefined'
                    onClicked: typeof rootItem?.sideBarButtonClicked == 'function' ? rootItem.sideBarButtonClicked(simpleDescriptionHeader, Qt.point(x, y)) : () => {}
                }

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
                    showGroupIcon: rootItem instanceof MessagesListView && rootItem.convo.kind === QEnums.CONVO_KIND_GROUP
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

                    onClicked: {
                        if (typeof rootItem.sideBarFeedUri == 'string')
                            skywalker.getFeedGenerator(rootItem.sideBarFeedUri)
                    }
                }
            }

            // Following feed
            RowLayout {
                readonly property bool active: rootItem instanceof FavoritesSwipeView && rootItem.currentView instanceof TimelinePage

                id: followingEntry
                width: parent.width
                spacing: 12
                visible: isBasePage

                SkyFooterButton {
                    Layout.preferredHeight: guiSettings.sideBarHeaderHeight
                    Layout.preferredWidth: Layout.preferredHeight
                    svg: followingEntry.active ? SvgFilled.menu : SvgOutline.menu
                    Accessible.name: qsTr("more options")
                    onClicked: {
                        if (followingEntry.active)
                            moreMenu.open()
                        else
                            followingEntry.activate()
                    }

                    TimelineOptionsMenu {
                        id: moreMenu
                        reverseFeed: skywalker.timelineModel.reverseFeed

                        onAddUserView: root.getTimelineView().addUserView()
                        onAddHashtagView: root.getTimelineView().addHashtagView()
                        onAddFocusHashtagView: root.getTimelineView().addFocusHashtagView()
                        onAddMediaView: root.getTimelineView().showMediaView()
                        onAddVideoView: root.getTimelineView().showVideoView()
                        onFilterStatistics: root.viewContentFilterStats(skywalker.timelineModel)
                        onNewReverseFeed: (reverse) => root.getTimelineView().setReverseFeed(reverse)
                    }
                }

                AccessibleText {
                    Layout.fillWidth: true
                    verticalAlignment: Text.AlignVCenter
                    rightPadding: 10
                    elide: Text.ElideRight
                    font.bold: followingEntry.active
                    text: qsTr("Following", "timeline title")

                    SkyMouseArea {
                        anchors.fill: parent
                        onClicked: followingEntry.activate()
                    }
                }

                function activate() {
                    root.viewHomeFeed()
                }
            }

            // Search
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

                    SkyMouseArea {
                        anchors.fill: parent
                        onClicked: searchClicked()
                    }
                }
            }

            // DM
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

                    SkyMouseArea {
                        anchors.fill: parent
                        onClicked: messagesClicked()
                    }
                }
            }

            // Notifications
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

                    SkyMouseArea {
                        anchors.fill: parent
                        onClicked: notificationsClicked()
                    }
                }
            }

            // Favorites
            Repeater {
                Layout.fillWidth: true
                model: favorites

                RowLayout {
                    required property favoritefeedview modelData
                    readonly property bool active: activeFeedView && activeFeedView.feedUri === modelData.uri

                    id: favoriteEntry
                    width: parent.width
                    spacing: 12
                    visible: isBasePage

                    FeedAvatar {
                        Layout.margins: 5
                        Layout.preferredHeight: guiSettings.sideBarHeaderHeight - 10
                        Layout.preferredWidth: Layout.preferredHeight
                        avatarUrl: modelData.avatarThumb
                        unknownSvg: guiSettings.favoriteDefaultAvatar(modelData)
                        contentMode: modelData.contentMode

                        onClicked: (clickPoint) => {
                            if (favoriteEntry.active)
                                activeFeedView.showFeedOptions(clickPoint, favoriteEntry)
                            else
                                favoriteEntry.activate()
                        }

                        onPressAndHold: showFavoritesSorter()
                    }

                    SkyCleanedTextLine {
                        Layout.fillWidth: true
                        verticalAlignment: Text.AlignVCenter
                        rightPadding: 10
                        elide: Text.ElideRight
                        font.bold: active
                        plainText: modelData.name

                        SkyMouseArea {
                            anchors.fill: parent
                            onClicked: favoriteEntry.activate()
                            onPressAndHold: showFavoritesSorter()
                        }
                    }

                    function activate() {
                        root.showFavorite(modelData)

                        if (!homeActive)
                            homeClicked()
                    }
                }
            }

            // Sort favorites
            RowLayout {
                width: parent.width
                spacing: 12
                visible: isBasePage && favorites.length > 1

                SkyFooterButton {
                    Layout.preferredHeight: guiSettings.sideBarHeaderHeight
                    Layout.preferredWidth: Layout.preferredHeight
                    color: guiSettings.buttonColor
                    svg: SvgFilled.settings
                    Accessible.name: qsTr("Sort favorites")
                    onClicked: showFavoritesSorter()
                }

                AccessibleText {
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                    font.bold: searchActive
                    text: qsTr("Sort favorites")

                    SkyMouseArea {
                        anchors.fill: parent
                        onClicked: showFavoritesSorter()
                    }
                }
            }
        }
    }

    FooterPostButton {
        id: postButton
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20
        width: 50
        height: width

        messagesActive: sideBar.messagesActive
        hashtagSearch: sideBar.searchActive && rootItem instanceof SearchView && rootItem.isHashtagSearch
        cashtagSearch: sideBar.searchActive && rootItem instanceof SearchView && rootItem.isCashtagSearch
        searchView: rootItem instanceof SearchView ? rootItem : null
        authorView: rootItem instanceof AuthorView ? rootItem : null
        postThreadView: rootItem instanceof PostThreadView ? rootItem : null
        visible: isBasePage || rootItem instanceof AuthorView || (rootItem instanceof PostThreadView && !rootItem.isUnrolledThread)

        onAddConvoClicked: sideBar.addConvoClicked()
    }

    Item {
        id: lockButton
        anchors.right: parent.right
        anchors.rightMargin: 20
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20
        width: 24
        height: 24

        SkySvg {
            width: height
            height: parent.height
            svg: userSettings.sideBarLocked ? SvgFilled.lock : SvgFilled.lockOpen
        }

        SkyMouseArea {
            anchors.fill: parent
            onClicked: {
                userSettings.sideBarLocked = !userSettings.sideBarLocked
                skywalker.showStatusMessage(
                    userSettings.sideBarLocked ? qsTr("Side bar size locked") : qsTr("Side bar size unlocked"),
                    QEnums.STATUS_LEVEL_INFO)
            }
        }
    }

    function showFavoritesSorter() {
        if (favorites.length > 1)
            root.showFavoritesSorter()
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
