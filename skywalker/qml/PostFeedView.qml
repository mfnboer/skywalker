import QtQuick
import QtQuick.Controls
import skywalker

PostListView {
    required property int modelId
    property bool showFavorites: false
    readonly property int unreadPosts: mediaTilesLoader.item ? mediaTilesLoader.item.unreadPosts : listUnreadPosts
    readonly property bool reverseFeed: model ? model.reverseFeed : false
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
        reverseFeed: model.reverseFeed
        inProgress: Boolean(model) && model.getFeedInProgress
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
        running: Boolean(model) && model.getFeedInProgress
    }

    Loader {
        anchors.top: emptyListIndication.bottom
        active: Boolean(model) && model.isFilterModel() && count === 0 && !model.endOfFeed && !Boolean(model.error)
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

    SkyMenu {
        property var feed: underlyingModel?.getGeneratorView()
        property bool feedHideFollowing: false

        id: feedOptionsMenu

        CloseMenuItem {
            text: qsTr("<b>Feed</b>")
            Accessible.name: qsTr("close more options menu")
        }

        AccessibleMenuItem {
            text: qsTr("Feed profile")
            svg: SvgOutline.feed
            onTriggered: showFeed()
        }

        AccessibleMenuItem {
            text: qsTr("Remove favorite")
            svg: SvgFilled.star
            svgColor: guiSettings.favoriteColor
            onTriggered: {
                skywalker.favoriteFeeds.pinFeed(feedOptionsMenu.feed, false)
                skywalker.saveFavoriteFeeds()
            }
        }

        AccessibleMenuItem {
            text: qsTr("Share")
            svg: SvgOutline.share
            onTriggered: skywalker.shareFeed(feedOptionsMenu.feed)
        }

        AccessibleMenuItem {
            text: qsTr("Filtered posts")
            svg: SvgOutline.hideVisibility
            onTriggered: root.viewContentFilterStats(underlyingModel)
        }

        AccessibleMenuItem {
            text: qsTr("Show following")
            checkable: true
            checked: !feedOptionsMenu.feedHideFollowing
            onToggled: {
                const fu = root.getFeedUtils(userDid)
                fu.hideFollowing(feedOptionsMenu.feed.uri, !checked)
                feedOptionsMenu.feedHideFollowing = !checked
            }
        }

        function show() {
            feedHideFollowing = skywalker.getUserSettings().getFeedHideFollowing(skywalker.getUserDid(), feed.uri)
            open()
        }
    }

    SkyMenu {
        property var list: underlyingModel?.getListView()
        property bool listHideFromTimeline: false
        property bool listHideReplies: false
        property bool listHideFollowing: false
        property bool listSync: false

        id: listFeedOptionsMenu
        menuWidth: 250

        CloseMenuItem {
            text: qsTr("<b>List</b>")
            Accessible.name: qsTr("close more options menu")
        }

        AccessibleMenuItem {
            text: qsTr("List profile")
            svg: SvgOutline.list
            onTriggered: showFeed()
        }

        AccessibleMenuItem {
            text: qsTr("Remove favorite")
            svg: SvgFilled.star
            svgColor: guiSettings.favoriteColor
            onTriggered: {
                const favorite = skywalker.favoriteFeeds.getPinnedFeed(listFeedOptionsMenu.list.uri)

                if (favorite.isNull()) {
                    console.warn("List is not a favorite:" << listFeedOptionsMenu.list.uri)
                    return
                }

                if (listFeedOptionsMenu.isOwnList())
                    skywalker.favoriteFeeds.removeList(favorite.listView) // We never show own lists as saved
                else
                    skywalker.favoriteFeeds.pinList(favorite.listView, false)

                skywalker.saveFavoriteFeeds()
            }
        }

        AccessibleMenuItem {
            id: hideListMenuItem
            visible: listFeedOptionsMenu.list?.purpose === QEnums.LIST_PURPOSE_CURATE && listFeedOptionsMenu.isOwnList()
            text: listFeedOptionsMenu.listHideFromTimeline ? qsTr("Unhide list from timeline") : qsTr("Hide list from timeline")
            svg: listFeedOptionsMenu.listHideFromTimeline ? SvgOutline.unmute : SvgOutline.mute
            onTriggered: {
                const gu = root.getGraphUtils(userDid)

                if (listFeedOptionsMenu.listHideFromTimeline) {
                    gu.unhideList(listFeedOptionsMenu.list.uri)
                    listFeedOptionsMenu.listHideFromTimeline = false
                }
                else {
                    gu.hideList(listFeedOptionsMenu.list.uri)
                }
            }
        }

        AccessibleMenuItem {
            text: qsTr("Share")
            svg: SvgOutline.share
            onTriggered: {
                const favorite = skywalker.favoriteFeeds.getPinnedFeed(listFeedOptionsMenu.list.uri)

                if (favorite.isNull()) {
                    console.warn("List is not a favorite:" << listFeedOptionsMenu.list.uri)
                    return
                }

                skywalker.shareList(favorite.listView)
            }
        }

        AccessibleMenuItem {
            text: qsTr("Filtered posts")
            svg: SvgOutline.hideVisibility
            onTriggered: root.viewContentFilterStats(underlyingModel)
        }

        PostsOrderMenu {
            reverseFeed: model.reverseFeed

            onNewReverseFeed: (reverse) => {
                userSettings.setFeedReverse(skywalker.getUserDid(), underlyingModel.feedUri, reverse)

                console.debug("Reverse feed changed:", reverse, feedName)
                const [reverseIndex, offsetY] = calcReverseVisibleIndexAndOffsetY(reverse)
                underlyingModel.reverseFeed = reverse
                setInSync(modelId, reverseIndex, offsetY)
            }
        }

        AccessibleMenuItem {
            text: qsTr("Show replies")
            checkable: true
            checked: !listFeedOptionsMenu.listHideReplies
            onToggled: {
                const gu = root.getGraphUtils(userDid)
                gu.hideReplies(listFeedOptionsMenu.list.uri, !checked)
                listFeedOptionsMenu.listHideReplies = !checked
            }
        }
        AccessibleMenuItem {
            text: qsTr("Show following")
            checkable: true
            checked: !listFeedOptionsMenu.listHideFollowing
            onToggled: {
                const gu = root.getGraphUtils(userDid)
                gu.hideFollowing(listFeedOptionsMenu.list.uri, !checked)
                listFeedOptionsMenu.listHideFollowing = !checked
            }
        }
        AccessibleMenuItem {
            text: qsTr("Rewind on startup")
            checkable: true
            checked: listFeedOptionsMenu.listSync
            onToggled: {
                const gu = root.getGraphUtils(userDid)
                gu.syncList(listFeedOptionsMenu.list.uri, checked)
                listFeedOptionsMenu.listSync = checked
            }
        }

        function isOwnList() {
            if (!list)
                return false

            const pu = root.getPostUtils(userDid)
            const listCreatorDid = pu.extractDidFromUri(list.uri)
            return skywalker.getUserDid() === listCreatorDid
        }

        function show() {
            listHideFromTimeline = skywalker.getTimelineHide().hasList(list.uri)
            listHideReplies = userSettings.getFeedHideReplies(skywalker.getUserDid(), list.uri)
            listHideFollowing = userSettings.getFeedHideFollowing(skywalker.getUserDid(), list.uri)
            listSync = userSettings.mustSyncFeed(skywalker.getUserDid(), list.uri)
            open()
        }
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
            console.debug("Media tiles loader active, don't sync:", model.feedName)
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
    }

    function handleSyncProgress(id, pages, timestamp) {
        if (id !== modelId)
            return

        console.debug("Sync proress:", model.feedName, "pages:", pages, "timestamp:", timestamp)
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
