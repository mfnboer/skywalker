import QtQuick
import QtQuick.Controls
import skywalker

PostListView {
    required property int modelId
    property bool showFavorites: false
    readonly property int unreadPosts: mediaTilesLoader.item ? mediaTilesLoader.item.unreadPosts : listUnreadPosts
    readonly property int extraFooterMargin: 0
    readonly property string feedName: underlyingModel ? underlyingModel.feedName : ""


    signal closed

    id: postFeedView
    width: parent.width
    model: skywalker.getPostFeedModel(modelId)
    inSync: true
    acceptsInteractions: underlyingModel ? underlyingModel.feedAcceptsInteractions : false
    reverseSyncFun: () => { moveToIndex(count - 1, doMoveToPost); finishSync() }
    resyncFun: () => setInSync(modelId, newLastVisibleIndex, newLastVisibleOffsetY)
    syncFun: (index, offsetY) => setInSync(modelId, index, offsetY)
    feedDid: underlyingModel ? underlyingModel.feedDid : ""

    Accessible.name: feedName

    header: PostFeedHeader {
        userDid: postFeedView.userDid
        reverseFeed: underlyingModel ? underlyingModel.reverseFeed : false
        feedName: postFeedView.feedName
        feedAvatar: postFeedView.getFeedAvatar()
        defaultSvg: postFeedView.getFeedDefaultAvatar()
        contentMode: initialContentMode
        underlyingContentMode: underlyingModel ? underlyingModel.contentMode : QEnums.CONTENT_MODE_UNSPECIFIED
        showAsHome: postFeedView.showAsHome
        showLanguageFilter: underlyingModel ? underlyingModel.languageFilterConfigured : false
        filteredLanguages: underlyingModel ? underlyingModel.filteredLanguages : []
        showPostWithMissingLanguage: underlyingModel ? underlyingModel.showPostWithMissingLanguage :true
        showViewOptions: true
        showFavoritesPlaceHolder: showFavorites && userSettings.favoritesBarPosition === QEnums.FAVORITES_BAR_POSITION_TOP
        visible: !root.showSideBar

        onClosed: postFeedView.closed()
        onFeedAvatarClicked: showFeedOptions()
        onViewChanged: (contentMode) => changeView(contentMode)

        onReverseFeedChanged: console.debug("REVERSE FEED HEADER CHAGED:", reverseFeed)
    }
    headerPositioning: ListView.PullBackHeader

    delegate: PostFeedViewDelegate {
        width: postFeedView.width
        feedAcceptsInteractions: postFeedView.acceptsInteractions
        feedDid: postFeedView.feedDid
        swipeMode: [QEnums.CONTENT_MODE_VIDEO, QEnums.CONTENT_MODE_MEDIA].includes(model.contentMode)
        extraFooterHeight: extraHeaderFooterLoader.active && !model.reverseFeed ? extraHeaderFooterLoader.height : 0
        extraHeaderHeight: extraHeaderFooterLoader.active && model.reverseFeed ? extraHeaderFooterLoader.height : 0

        onActivateSwipe: (imgIndex, previewImg) => {
            let view = postFeedView
            root.viewMediaFeed(model, index, imgIndex, previewImg,
                (newIndex, mediaIndex, closeCb) => {
                    view.positionViewAtIndex(newIndex, ListView.Beginning)
                    view.itemAtIndex(newIndex).closeMedia(mediaIndex, closeCb)
                },
                userDid)
        }

        Loader {
            id: extraHeaderFooterLoader
            y: model.reverseFeed ? 0 : parent.height - height
            active: model && model.isFilterModel() && isLastPost && !endOfFeed

            sourceComponent: FeedViewLoadMore {
                userDid: postFeedView.userDid
                listView: postFeedView
            }
        }
    }

    onMovementEnded: {
        if (!inSync)
            return

        const firstVisibleIndex = getFirstVisibleIndex()
        const lastVisibleIndex = getLastVisibleIndex()

        if (lastVisibleIndex !== -1 && modelId !== -1) {
            const lastVisibleOffsetY = calcVisibleOffsetY(lastVisibleIndex)
            skywalker.feedMovementEnded(modelId, lastVisibleIndex, lastVisibleOffsetY)
        }

        setAnchorItem(firstVisibleIndex, lastVisibleIndex)
        updateOnMovement()
    }

    FlickableRefresher {
        reverseFeed: model && model.reverseFeed
        inProgress: model && model.getFeedInProgress
        verticalOvershoot: postFeedView.verticalOvershoot
        topOvershootFun: reverseFeed ? () => model.getFeedNextPage(skywalker) : () => model.getFeed(skywalker)
        bottomOvershootFun: reverseFeed ? () => model.getFeed(skywalker) : () => model.getFeedNextPage(skywalker)
        topText: reverseFeed ? qsTr("Pull up to refresh feed") : qsTr("Pull down to refresh feed")
        enableScrollToTop: !showAsHome
    }

    EmptyListIndication {
        id: emptyListIndication
        y: parent.headerItem ? parent.headerItem.height : 0
        svg: SvgOutline.noPosts
        text: qsTr("Feed is empty")
        list: postFeedView

        onRetry: model.getFeed(skywalker)
    }

    BusyIndicator {
        id: busyIndicator
        anchors.centerIn: parent
        running: model && model.getFeedInProgress
    }

    Loader {
        anchors.top: emptyListIndication.bottom
        active: model && model.isFilterModel() && count === 0 && !model.endOfFeed && !Boolean(model.error)
        sourceComponent: FeedViewLoadMore {
            userDid: postFeedView.userDid
            listView: postFeedView
        }
    }

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

    FeedOptionsMenu {
        id: feedOptionsMenu
        userDid: postFeedView.userDid
        postFeedModel: underlyingModel
        feed: underlyingModel?.getGeneratorView()

        onShowFeed: postFeedView.showFeed()
        onNewReverseFeed: (reverse) => changeReverseFeed(reverse)
    }

    ListFeedOptionsMenu {
        id: listFeedOptionsMenu
        userDid: postFeedView.userDid
        postFeedModel: underlyingModel
        list: underlyingModel?.getListView()

        onShowFeed: postFeedView.showFeed()
        onNewReverseFeed: (reverse) => changeReverseFeed(reverse)
    }

    function changeReverseFeed(reverse) {
        userSettings.setFeedReverse(skywalker.getUserDid(), underlyingModel.feedUri, reverse)

        console.debug("Reverse feed changed:", reverse, feedName)
        const [reverseIndex, offsetY] = calcReverseVisibleIndexAndOffsetY(reverse)
        underlyingModel.reverseFeed = reverse
        setInSync(modelId, reverseIndex, offsetY)
    }

    function getFeedDefaultAvatar() {
        if (!underlyingModel)
            return SvgFilled.feed

        switch (underlyingModel.feedType) {
        case QEnums.FEED_GENERATOR:
            return guiSettings.feedDefaultAvatar(underlyingModel.getGeneratorView())
        case QEnums.FEED_LIST:
            return SvgFilled.list
        default:
            console.warn("Unexpected feed type:", underlyingModel.feedType)
            return SvgFilled.feed
        }
    }

    function getFeedAvatar() {
        if (!underlyingModel)
            return ""

        switch (underlyingModel.feedType) {
        case QEnums.FEED_GENERATOR:
            return guiSettings.feedContentVisible(underlyingModel.getGeneratorView(), userDid) ?
                underlyingModel.getGeneratorView().avatarThumb : ""
        case QEnums.FEED_LIST:
            return guiSettings.feedContentVisible(underlyingModel.getListView(), userDid) ?
                underlyingModel.getListView().avatarThumb : ""
        default:
            console.warn("Unexpected feed type:", underlyingModel.feedType)
            return ""
        }
    }

    function showFeed() {
        if (!underlyingModel)
            return

        switch (underlyingModel.feedType) {
        case QEnums.FEED_GENERATOR:
            skywalker.getFeedGenerator(underlyingModel.getGeneratorView().uri)
            break
        case QEnums.FEED_LIST:
            root.viewListByUri(underlyingModel.getListView().uri, false)
            break
        default:
            console.warn("Unexpected feed type:", underlyingModel.feedType)
            break
        }
    }

    function showFeedOptions() {
        if (!underlyingModel)
            return

        switch (underlyingModel.feedType) {
        case QEnums.FEED_GENERATOR:
            feedOptionsMenu.show()
            break
        case QEnums.FEED_LIST:
            listFeedOptionsMenu.show()
            break
        default:
            console.warn("Unexpected feed type:", underlyingModel.feedType)
            break
        }
    }

    function moveToHome() {
        console.debug("Move to home:", feedName)

        if (model.reverseFeed)
            positionViewAtEnd()
        else
            positionViewAtBeginning()

        const homeIndex = model.reverseFeed ? count - 1 : 0
        setAnchorItem(homeIndex, homeIndex)

        if (mediaTilesLoader.item)
            mediaTilesLoader.item.moveToHome()

        if (modelId != -1)
            skywalker.feedMovementEnded(modelId, homeIndex, 0)

        updateUnreadPosts()
    }

    function finishSync() {
        syncDone()
        rewindStatus.isFirstRewind = false
        updateUnreadPosts()
        resetHeaderPosition()
    }

    function setInSync(id, index, offsetY = 0) {
        if (id !== modelId)
            return

        if (mediaTilesLoader.active) {
            console.debug("Media tiles loader active, sync:", model.feedName)
            mediaTilesLoader.item.setInSync(index)
            finishSync()
            return
        }

        console.debug("Sync:", model.feedName, "index:", index, "count:", count, "offsetY:", offsetY)
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
                positionViewAtEnd()

            finishSync()
        }
    }

    function syncToHome(id) {
        if (id !== modelId)
            return

        finishSync()
        moveToHome()
    }

    function handleSyncStart(id, maxPages, timestamp) {
        if (id !== modelId)
            return

        console.debug("Sync start:", model.feedName, "maxPages:", maxPages, "timestamp:", timestamp)
        rewindStatus.startRewind(maxPages, timestamp)
        inSync = false

        if (mediaTilesLoader.item)
            mediaTilesLoader.item.stopSync()
    }

    function handleSyncProgress(id, pages, timestamp) {
        if (id !== modelId)
            return

        console.debug("Sync progress:", model.feedName, "pages:", pages, "timestamp:", timestamp)
        rewindStatus.updateRewindProgress(pages, timestamp)
    }

    function forceDestroy() {
        if (modelId !== -1) {
            skywalker.removePostFeedModel(modelId)
            modelId = -1
            destroy()
        }
    }

    Component.onDestruction: {
        skywalker.onFeedSyncStart.disconnect(handleSyncStart)
        skywalker.onFeedSyncProgress.disconnect(handleSyncProgress)
        skywalker.onFeedSyncOk.disconnect(setInSync)
        skywalker.onFeedSyncFailed.disconnect(syncToHome)

        if (modelId !== -1)
            skywalker.removePostFeedModel(modelId)
    }

    Component.onCompleted: {
        skywalker.onFeedSyncStart.connect(handleSyncStart)
        skywalker.onFeedSyncProgress.connect(handleSyncProgress)
        skywalker.onFeedSyncOk.connect(setInSync)
        skywalker.onFeedSyncFailed.connect(syncToHome)

        const viewMode = userSettings.getFeedViewMode(skywalker.getUserDid(), model.feedUri)

        if (viewMode !== QEnums.CONTENT_MODE_UNSPECIFIED) {
            initialContentMode = viewMode
            changeView(viewMode)
        }
    }
}
