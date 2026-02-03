import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker
import atproto.lib

// When you use moveToIndex(), you must set enough cacheBuffer space. Position a list
// to an index, may it to position a few items before the index the first time (size mismatch).
// Therefore positioning is called a few times. If there is enough buffer then the item to
// position to will be already created if we are close, and the next positioning call will be
// accurate. If not, then exisiting items will be destroyed. Positioning will be off again,
// and so on.
PostListView {
    required property searchfeed searchFeed
    property bool pinnedFeed: false
    readonly property int unreadPosts: mediaTilesLoader.item ? mediaTilesLoader.item.unreadPosts : listUnreadPosts
    readonly property int extraFooterMargin: 0

    signal closed

    id: feedView
    width: parent.width
    inSync: true
    reverseSyncFun: () => { moveToIndex(count - 1, doMoveToPost); finishSync() }
    resyncFun: () => setInSync(newLastVisibleIndex, newLastVisibleOffsetY, true)
    syncFun: (index, offsetY) => setInSync(index, offsetY)

    Accessible.name: searchFeed.name

    header: PostFeedHeader {
        feedName: searchFeed.name
        defaultSvg: guiSettings.searchFeedDefaultAvatar(searchFeed)
        feedAvatar: ""
        contentMode: initialContentMode
        showAsHome: feedView.showAsHome
        showLanguageFilter: searchFeed.languageList.length > 0
        filteredLanguages: searchFeed.languageList
        showPostWithMissingLanguage: false
        showViewOptions: true
        showFavoritesPlaceHolder: userSettings.favoritesBarPosition === QEnums.FAVORITES_BAR_POSITION_TOP
        visible: !root.showSideBar

        onClosed: feedView.closed()
        onFeedAvatarClicked: showOptionsMenu()
        onViewChanged: (contentMode) => changeView(contentMode)
    }
    headerPositioning: ListView.PullBackHeader

    delegate: PostFeedViewDelegate {
        width: feedView.width
        swipeMode: [QEnums.CONTENT_MODE_VIDEO, QEnums.CONTENT_MODE_MEDIA].includes(model.contentMode)
        extraFooterHeight: extraHeaderFooterLoader.active && !model.reverseFeed ? extraHeaderFooterLoader.height : 0
        extraHeaderHeight: extraHeaderFooterLoader.active && model.reverseFeed ? extraHeaderFooterLoader.height : 0

        onActivateSwipe: (imgIndex, previewImg) => {
            let view = feedView
            root.viewMediaFeed(model, index, imgIndex, previewImg,
                (newIndex, mediaIndex, closeCb) => {
                    view.positionViewAtIndex(newIndex, ListView.Beginning)
                    view.itemAtIndex(newIndex).closeMedia(mediaIndex, closeCb)
                })
        }

        Loader {
            id: extraHeaderFooterLoader
            y: model.reverseFeed ? 0 : parent.height - height
            active: model && model.isFilterModel() && isLastPost && !endOfFeed

            sourceComponent: FeedViewLoadMore {
                listView: feedView
            }
        }
    }

    onMovementEnded: {
        if (!inSync)
            return

        const firstVisibleIndex = getFirstVisibleIndex()
        const lastVisibleIndex = getLastVisibleIndex()

        if (lastVisibleIndex !== -1 && model) {
            const lastVisibleOffsetY = calcVisibleOffsetY(lastVisibleIndex)
            skywalker.searchFeedMovementEnded(model.getModelId(), model.contentMode, lastVisibleIndex, lastVisibleOffsetY)
        }

        setAnchorItem(firstVisibleIndex, lastVisibleIndex)
        updateOnMovement()
    }

    FlickableRefresher {
        reverseFeed: model && model.reverseFeed
        inProgress: model && model.getFeedInProgress
        topOvershootFun: reverseFeed ? feedView.getNextPage() : () => feedView.syncSearch()
        bottomOvershootFun: () => reverseFeed ? () => feedView.syncSearch() : feedView.getNextPage()
        topText: reverseFeed ? qsTr("Pull up to refresh feed") : qsTr("Pull down to refresh feed")
        enableScrollToTop: !showAsHome
    }

    EmptyListIndication {
        id: emptyListIndication
        y: parent.headerItem ? parent.headerItem.height : 0
        svg: SvgOutline.noPosts
        text: qsTr("Feed is empty")
        list: feedView

        onRetry: feedView.syncSearch()
    }

    BusyIndicator {
        id: busyIndicator
        anchors.centerIn: parent
        running: model && feedView.model.getFeedInProgress
    }

    Loader {
        anchors.top: emptyListIndication.bottom
        active: model && model.isFilterModel() && count === 0 && !model.endOfFeed && !Boolean(model.error)
        sourceComponent: FeedViewLoadMore {
            listView: feedView
        }
    }

    // TODO: duplicate code
    Rectangle {
        y: headerItem ? headerItem.height : 0
        width: parent.width
        height: parent.height - (headerItem ? headerItem.height : 0) - (footerItem && footerItem.visible ? footerItem.height : 0)
        color: guiSettings.backgroundColor
        visible: !inSync && (rewindStatus.rewindPagesLoaded > 0 || rewindStatus.isFirstRewind)

        Column {
            width: parent.width - 20
            anchors.centerIn: parent

            AccessibleText {
                anchors.horizontalCenter: parent.horizontalCenter
                font.pointSize: guiSettings.scaledFont(2)
                text: qsTr("Rewinding feed")
            }

            RewindStatus {
                property bool isFirstRewind: true

                id: rewindStatus
                width: parent.width
            }
        }
    }

    SearchUtils {
        id: searchUtils
        skywalker: feedView.skywalker
        searchPageSize: mediaTilesLoader.active ? 100 : 0

        Component.onDestruction: {
            // The destuctor of SearchUtils is called too late by the QML engine
            // Remove models now before the Skywalker object is destroyed.
            searchUtils.removeModels()
        }
    }

    FeedOptionsMenu {
        id: optionsMenu
        postFeedModel: underlyingModel
        feed: searchFeed

        onShowFeed: root.viewSearchViewFeed(searchFeed)
        onNewReverseFeed: (reverse) => changeReverseFeed(reverse)
        onEnableSyncSearchFeed: (enable) => searchUtils.syncFeed(searchFeed.searchQuery, enable)
    }

    function changeReverseFeed(reverse) {
        userSettings.setSearchFeedReverse(skywalker.getUserDid(), searchFeed.name, reverse)

        console.debug("Reverse feed changed:", reverse, searchFeed.name)
        const [reverseIndex, offsetY] = calcReverseVisibleIndexAndOffsetY(reverse)
        underlyingModel.reverseFeed = reverse
        setInSync(reverseIndex, offsetY)
    }

    function showOptionsMenu() {
        optionsMenu.show()
    }

    function moveToHome() {
        console.debug("Move to home:", model.feedName)

        if (model.reverseFeed)
            moveToIndex(count - 1, doMoveToPost)
        else
            positionViewAtBeginning()

        const homeIndex = model.reverseFeed ? count - 1 : 0
        setAnchorItem(homeIndex, homeIndex)

        if (mediaTilesLoader.item)
            mediaTilesLoader.item.moveToHome()

        skywalker.searchFeedMovementEnded(model.getModelId(), model.contentMode, homeIndex, 0)

        updateUnreadPosts()
    }

    function finishSync() {
        syncDone()
        rewindStatus.isFirstRewind = false
        updateUnreadPosts()
        resetHeaderPosition()
    }

    function setInSync(index, offsetY = 0, resync = false) {
        console.debug("Sync:", model.feedName, "index:", index, "count:", count, "offsetY:", offsetY)

        if (mediaTilesLoader.active) {
            console.debug("Media tiles loader active, sync:", model.feedName)

            if (!resync)
                mediaTilesLoader.item.setInSync(index)

            finishSync()
            return
        }

        const homeIndex = model.reverseFeed ? count - 1 : 0

        if (index === homeIndex && offsetY === 0) {
            moveToHome()
            finishSync()
        }
        else if (index >= 0) {
            moveToIndex(index, doMoveToPost, () => { contentY -= offsetY; finishSync() })
        }
        else {
            if (reverseFeed)
                positionViewAtBeginning()
            else
                moveToIndex(count - 1, doMoveToPost)

            finishSync()
        }
    }

    function syncToHome() {
        moveToHome()
        finishSync()
    }

    function handleSyncStart(maxPages, timestamp) {
        console.debug("Sync start:", model.feedName, "maxPages:", maxPages, "timestamp:", timestamp)
        rewindStatus.startRewind(maxPages, timestamp)
        inSync = false

        if (mediaTilesLoader.item)
            mediaTilesLoader.item.stopSync()
    }

    function handleSyncProgress(pages, timestamp) {
        console.debug("Sync progress:", model.feedName, "pages:", pages, "timestamp:", timestamp)
        rewindStatus.updateRewindProgress(pages, timestamp)
    }

    function getNextPage() {
        searchUtils.getNextPageSearchPosts(searchFeed.searchQuery, SearchSortOrder.LATEST,
                                searchFeed.authorHandle, searchFeed.mentionHandle,
                                searchFeed.since, !isNaN(searchFeed.since.getTime()),
                                searchFeed.until, !isNaN(searchFeed.until.getTime()),
                                searchFeed.language)
    }

    function syncSearch() {
        searchUtils.syncSearchPosts(searchFeed.searchQuery,
                                    searchFeed.authorHandle, searchFeed.mentionHandle,
                                    searchFeed.since, !isNaN(searchFeed.since.getTime()),
                                    searchFeed.until, !isNaN(searchFeed.until.getTime()),
                                    searchFeed.language)
    }

    function forceDestroy() {
        searchUtils.onFeedSyncStart.disconnect(handleSyncStart)
        searchUtils.onFeedSyncProgress.disconnect(handleSyncProgress)
        searchUtils.onFeedSyncOk.disconnect(setInSync)
        searchUtils.onFeedSyncFailed.disconnect(syncToHome)

        searchUtils.clearAllSearchResults()
        searchUtils.removeModels()
        destroy()
    }

    Component.onCompleted: {
        console.debug("Search feed view:", searchFeed.name)
        // Model set here. Setting it right away causes getSearchPostFeedModel crash with an
        // assert as the skywalker property is not set yet.
        const m = searchUtils.getSearchPostFeedModel(SearchSortOrder.LATEST, searchFeed.name)
        m.onFirstPage.connect(() => { syncSearch() })
        m.onNextPage.connect(() => { getNextPage() })
        setModel(m)

        searchUtils.onFeedSyncStart.connect(handleSyncStart)
        searchUtils.onFeedSyncProgress.connect(handleSyncProgress)
        searchUtils.onFeedSyncOk.connect(setInSync)
        searchUtils.onFeedSyncFailed.connect(syncToHome)

        const viewMode = userSettings.getSearchFeedViewMode(skywalker.getUserDid(), searchFeed.name)

        if (viewMode !== QEnums.CONTENT_MODE_UNSPECIFIED) {
            initialContentMode = viewMode
            changeView(viewMode)
        }

        syncSearch()
    }
}
