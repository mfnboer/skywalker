import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker
import atproto.lib

SkyListView {
    required property var skywalker
    required property searchfeed searchFeed
    property bool showAsHome: false
    property int unreadPosts: 0

    signal closed

    id: feedView
    width: parent.width
    model: searchUtils.getSearchPostFeedModel(SearchSortOrder.LATEST)

    Accessible.name: searchFeed.name

    header: PostFeedHeader {
        skywalker: feedView.skywalker
        feedName: searchFeed.name
        defaultSvg: searchFeed.isHashtag() ? SvgOutline.hashtag : SvgOutline.search
        feedAvatar: ""
        showAsHome: feedView.showAsHome
        // TODO showLanguageFilter: feedView.model.languageFilterConfigured
        //filteredLanguages: feedView.model.filteredLanguages
        //showPostWithMissingLanguage: feedView.model.showPostWithMissingLanguage

        onClosed: feedView.closed()
        onFeedAvatarClicked: {
            // TODO: show SearchView.qml
        }
    }
    headerPositioning: ListView.OverlayHeader

    footer: SkyFooter {
        visible: showAsHome
        timeline: feedView
        skywalker: feedView.skywalker
        homeActive: true
        showHomeFeedBadge: true
        onHomeClicked: feedView.positionViewAtBeginning()
        onNotificationsClicked: root.viewNotifications()
        onSearchClicked: root.viewSearchView()
        onFeedsClicked: root.viewFeedsView()
        onMessagesClicked: root.viewChat()
    }
    footerPositioning: ListView.OverlayFooter

    delegate: PostFeedViewDelegate {
        width: feedView.width
    }

    FlickableRefresher {
        inProgress: searchUtils.searchPostsLatestInProgress
        topOvershootFun: () => feedView.search()
        bottomOvershootFun: () => feedView.getNextPage()
        topText: qsTr("Pull down to refresh feed")
        enableScrollToTop: !showAsHome
    }

    EmptyListIndication {
        y: parent.headerItem ? parent.headerItem.height : 0
        svg: SvgOutline.noPosts
        text: qsTr("Feed is empty")
        list: feedView
    }

    BusyIndicator {
        id: busyIndicator
        anchors.centerIn: parent
        running: searchUtils.searchPostsLatestInProgress
    }

    SearchUtils {
        id: searchUtils
        skywalker: feedView.skywalker

        Component.onDestruction: {
            // The destuctor of SearchUtils is called too late by the QML engine
            // Remove models now before the Skywalker object is destroyed.
            searchUtils.removeModels()
        }
    }

    function search() {
        console.debug("SINCE:", searchFeed.since)
        searchUtils.searchPosts(searchFeed.searchQuery, SearchSortOrder.LATEST,
                                searchFeed.authorHandle, searchFeed.mentionHandle,
                                searchFeed.since, !isNaN(searchFeed.since.getTime()),
                                searchFeed.until, !isNaN(searchFeed.until.getTime()),
                                searchFeed.language)
    }

    function getNextPage() {
        searchUtils.getNextPageSearchPosts(searchFeed.searchQuery, SearchSortOrder.LATEST,
                                searchFeed.authorHandle, searchFeed.mentionHandle,
                                searchFeed.since, !isNaN(searchFeed.since.getTime()),
                                searchFeed.until, !isNaN(searchFeed.until.getTime()),
                                searchFeed.language)
    }

    function forceDestroy() {
        searchUtils.clearAllSearchResults()
        feedView.model = null
        searchUtils.removeModels()
        destroy()
    }

    Component.onCompleted: {
        search()
    }
}
