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
    property var userSettings: skywalker.getUserSettings()
    readonly property int favoritesY: getFavoritesY()
    readonly property int extraFooterMargin: 0

    signal closed

    id: feedView
    width: parent.width
    model: searchUtils.getSearchPostFeedModel(SearchSortOrder.LATEST)
    virtualFooterHeight: userSettings.favoritesBarPosition === QEnums.FAVORITES_BAR_POSITION_BOTTOM ? guiSettings.tabBarHeight : 0

    Accessible.name: searchFeed.name

    header: PostFeedHeader {
        skywalker: feedView.skywalker
        feedName: searchFeed.name
        defaultSvg: searchFeed.isHashtag() ? SvgOutline.hashtag : SvgOutline.search
        feedAvatar: ""
        showAsHome: feedView.showAsHome
        showLanguageFilter: searchFeed.languageList.length > 0
        filteredLanguages: searchFeed.languageList
        showPostWithMissingLanguage: false
        showFavoritesPlaceHolder: userSettings.favoritesBarPosition === QEnums.FAVORITES_BAR_POSITION_TOP
        visible: !root.showSideBar

        onClosed: feedView.closed()
        onFeedAvatarClicked: root.viewSearchViewFeed(searchFeed)
    }
    headerPositioning: ListView.PullBackHeader

    delegate: PostFeedViewDelegate {
        width: feedView.width
    }

    onCountChanged: {
        updateUnreadPosts()
    }

    onMovementEnded: {
        updateUnreadPosts()
    }

    FlickableRefresher {
        inProgress: feedView.model.getFeedInProgress
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
        running: feedView.model.getFeedInProgress
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

    function getFavoritesY() {
        switch (userSettings.favoritesBarPosition) {
        case QEnums.FAVORITES_BAR_POSITION_TOP:
            return headerItem ? headerItem.favoritesY - (contentY - headerItem.y) : 0
        case QEnums.FAVORITES_BAR_POSITION_BOTTOM:
            return virtualFooterY
        }

        return 0
    }

    function updateUnreadPosts() {
        const firstIndex = getFirstVisibleIndex()
        feedView.unreadPosts = Math.max(firstIndex, 0)
    }

    function atStart() {
        return atYBeginning
    }

    function moveToHome() {
        positionViewAtBeginning()
        updateUnreadPosts()
    }

    function search() {
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
