import QtQuick
import QtQuick.Controls
import skywalker

SkyListView {
    required property var skywalker
    required property int modelId
    property bool showAsHome: false
    property bool showFavorites: false
    property int unreadPosts: mediaTilesLoader.item ? mediaTilesLoader.item.unreadPosts : feedUnreadPosts
    property int feedUnreadPosts: 0
    property bool inSync: true
    readonly property var underlyingModel: model ? model.getUnderlyingModel() : null
    property int initialContentMode: underlyingModel ? underlyingModel.contentMode : QEnums.CONTENT_MODE_UNSPECIFIED
    property var userSettings: skywalker.getUserSettings()
    readonly property int favoritesY: getFavoritesY()
    readonly property int extraFooterMargin: 0
    readonly property string feedName: underlyingModel ? underlyingModel.feedName : ""
    readonly property bool acceptsInteractions: underlyingModel ? underlyingModel.feedAcceptsInteractions : false
    readonly property string feedDid: underlyingModel ? underlyingModel.feedDid : ""


    signal closed

    id: postFeedView
    width: parent.width
    model: skywalker.getPostFeedModel(modelId)
    cacheBuffer: 2000
    virtualFooterHeight: userSettings.favoritesBarPosition === QEnums.FAVORITES_BAR_POSITION_BOTTOM ? guiSettings.tabBarHeight : 0

    Accessible.name: feedName

    header: PostFeedHeader {
        skywalker: postFeedView.skywalker
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
        onFeedAvatarClicked: showFeed()
        onViewChanged: (contentMode) => changeView(contentMode)
    }
    headerPositioning: ListView.PullBackHeader

    delegate: PostFeedViewDelegate {
        width: postFeedView.width
        feedAcceptsInteractions: postFeedView.acceptsInteractions
        feedDid: postFeedView.feedDid
        swipeMode: [QEnums.CONTENT_MODE_VIDEO, QEnums.CONTENT_MODE_MEDIA].includes(model.contentMode)
        extraFooterHeight: extraFooterLoader.active ? extraFooterLoader.height : 0

        onActivateSwipe: {
            let view = postFeedView
            root.viewMediaFeed(model, index, (newIndex) => { view.positionViewAtIndex(newIndex, ListView.Beginning) })
        }

        Loader {
            id: extraFooterLoader
            anchors.bottom: parent.bottom

            active: model && model.isFilterModel() && index === count - 1 && !endOfFeed
            sourceComponent: extraFooterComponent
        }
    }

    onCountChanged: {
        updateFeedUnreadPosts()
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
        updateFeedUnreadPosts()
    }

    FlickableRefresher {
        inProgress: Boolean(model) && model.getFeedInProgress
        verticalOvershoot: postFeedView.verticalOvershoot
        topOvershootFun: () => model.getFeed(skywalker)
        bottomOvershootFun: () => model.getFeedNextPage(skywalker)
        topText: qsTr("Pull down to refresh feed")
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

    Component {
        id: extraFooterComponent

        Rectangle {
            width: postFeedView.width
            height: 150
            color: "transparent"

            AccessibleText {
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                padding: 10
                textFormat: Text.RichText
                wrapMode: Text.Wrap
                text: qsTr(`${guiSettings.getFilteredPostsFooterText(model)}<br><a href="load" style="color: ${guiSettings.linkColor}; text-decoration: none">Load more</a>`)
                onLinkActivated: model.getFeedNextPage(skywalker)
            }
        }
    }

    Loader {
        anchors.top: emptyListIndication.bottom
        active: Boolean(model) && model.isFilterModel() && count === 0 && !model.endOfFeed && !Boolean(model.error)
        sourceComponent: extraFooterComponent
    }

    Loader {
        id: mediaTilesLoader
        active: false

        sourceComponent: MediaTilesFeedView {
            property int favoritesY: getFavoritesY()

            clip: true
            width: postFeedView.width
            height: postFeedView.height - (postFeedView.footerItem && postFeedView.footerItem.visible ? postFeedView.footerItem.height : 0)
            headerHeight: postFeedView.headerItem ? postFeedView.headerItem.height : 0
            skywalker: postFeedView.skywalker
            acceptsInteractions: postFeedView.acceptsInteractions
            feedDid: postFeedView.feedDid
            showAsHome: postFeedView.showAsHome
            model: postFeedView.model
            virtualFooterHeight: postFeedView.virtualFooterHeight

            // HACK: grid view does not have a pullback header
            Loader {
                id: headerLoader
                y: headerY
                width: parent.width
                sourceComponent: postFeedView.header
            }

            function getFavoritesY() {
                switch (userSettings.favoritesBarPosition) {
                case QEnums.FAVORITES_BAR_POSITION_TOP:
                    return headerLoader.item ? headerLoader.item.favoritesY + headerY : headerY
                case QEnums.FAVORITES_BAR_POSITION_BOTTOM:
                    return virtualFooterY
                }

                return 0
            }
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

    function getFavoritesY() {
        if (mediaTilesLoader.item)
            return mediaTilesLoader.item.favoritesY

        switch (userSettings.favoritesBarPosition) {
        case QEnums.FAVORITES_BAR_POSITION_TOP:
            return headerItem ? headerItem.favoritesY - (contentY - headerItem.y) : 0
        case QEnums.FAVORITES_BAR_POSITION_BOTTOM:
            return virtualFooterY
        }

        return 0
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
            return guiSettings.feedContentVisible(underlyingModel.getGeneratorView()) ?
                underlyingModel.getGeneratorView().avatarThumb : ""
        case QEnums.FEED_LIST:
            return guiSettings.feedContentVisible(underlyingModel.getListView()) ?
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

    function changeView(contentMode) {
        let oldModel = model
        const lastVisibleIndex = mediaTilesLoader.item ? mediaTilesLoader.item.getTopRightVisibleIndex() : getLastVisibleIndex()
        const timestamp = model.getPostTimelineTimestamp(lastVisibleIndex)
        const cid = model.getPostCid(lastVisibleIndex)
        const lastVisibleOffsetY = mediaTilesLoader.item ? 0 : calcVisibleOffsetY(lastVisibleIndex)

        // When a tiles view is shown the header gets duplicated. Make sure the content values
        // between these headers is synced.
        headerItem.contentMode = contentMode
        initialContentMode = contentMode

        switch (contentMode) {
        case QEnums.CONTENT_MODE_UNSPECIFIED:
            model = model.getUnderlyingModel()
            break
        case QEnums.CONTENT_MODE_VIDEO:
            case QEnums.CONTENT_MODE_VIDEO_TILES:
            model = model.getUnderlyingModel().addVideoFilter()
            break
        case QEnums.CONTENT_MODE_MEDIA:
        case QEnums.CONTENT_MODE_MEDIA_TILES:
            model = model.getUnderlyingModel().addMediaFilter()
            break
        default:
            console.warn("Unknown content mode:", contentMode)
            return
        }

        if (oldModel.isFilterModel())
            oldModel.getUnderlyingModel().deleteFilteredPostFeedModel(oldModel)

        if (skywalker.favoriteFeeds.isPinnedFeed(underlyingModel.feedUri)) {
            userSettings.setFeedViewMode(skywalker.getUserDid(), underlyingModel.feedUri, contentMode)
        }

        mediaTilesLoader.active = [QEnums.CONTENT_MODE_MEDIA_TILES, QEnums.CONTENT_MODE_VIDEO_TILES].includes(contentMode)

        if (lastVisibleIndex > -1) {
            const newIndex = model.findTimestamp(timestamp, cid)
            setInSync(modelId, newIndex, lastVisibleOffsetY)

            if (mediaTilesLoader.item) {
                mediaTilesLoader.item.goToIndex(newIndex)
            }
        }
    }

    function updateFeedUnreadPosts() {
        const firstIndex = getFirstVisibleIndex()
        postFeedView.feedUnreadPosts = Math.max(firstIndex, 0)
    }

    function moveToHome() {
        positionViewAtBeginning()
        setAnchorItem(0, 0)

        if (mediaTilesLoader.item)
            mediaTilesLoader.item.moveToHome()

        if (modelId != -1)
            skywalker.feedMovementEnded(modelId, 0, 0)

        updateFeedUnreadPosts()
    }

    function atStart() {
        if (mediaTilesLoader.item)
            return mediaTilesLoader.item.atYBeginning
        else
            return atYBeginning
    }

    function resetHeaderPosition() {
        if (mediaTilesLoader.item)
            mediaTilesLoader.item.resetHeaderPosition()
        else
            privateResetHeaderPosition()
    }

    function doMoveToPost(index) {
        const firstVisibleIndex = getFirstVisibleIndex()
        const lastVisibleIndex = getLastVisibleIndex()
        console.debug("Move to:", model.feedName, "index:", index, "first:", firstVisibleIndex, "last:", lastVisibleIndex, "count:", count)
        positionViewAtIndex(Math.max(index, 0), ListView.End)
        setAnchorItem(firstVisibleIndex, lastVisibleIndex)
        updateFeedUnreadPosts()
        resetHeaderPosition()
        return (lastVisibleIndex >= index - 1 && lastVisibleIndex <= index + 1)
    }

    function finishSync() {
        inSync = true
        rewindStatus.isFirstRewind = false
        updateFeedUnreadPosts()
        resetHeaderPosition()
    }

    function setInSync(id, index, offsetY = 0) {
        if (id !== modelId)
            return

        console.debug("Sync:", model.feedName, "index:", index, "count:", count, "offsetY:", offsetY)

        if (index === 0 && offsetY === 0) {
            moveToHome()
            finishSync()
        }
        else if (index >= 0) {
            moveToIndex(index, doMoveToPost, () => { contentY -= offsetY; finishSync() })
        }
        else {
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
            postFeedView.model = null
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

        if (modelId !== -1) {
            model = null
            skywalker.removePostFeedModel(modelId)
        }
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
