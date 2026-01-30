import QtQuick
import skywalker

SkyListView {
    property string userDid
    property Skywalker skywalker: root.getSkywalker(userDid)
    property var userSettings: skywalker.getUserSettings()
    property bool inSync: false
    property int listUnreadPosts: 0
    property int newLastVisibleIndex: -1
    property int newLastVisibleOffsetY: 0
    property bool showAsHome: false
    property bool acceptsInteractions: false
    property string feedDid
    property var reverseSyncFun: () => {}
    property var resyncFun: () => {}
    property var syncFun: (index, offset) => {}
    readonly property bool reverseFeed: model ? model.reverseFeed : false
    readonly property var underlyingModel: model ? model.getUnderlyingModel() : null
    property int initialContentMode: underlyingModel ? underlyingModel.contentMode : QEnums.CONTENT_MODE_UNSPECIFIED
    readonly property var mediaTilesLoader: mediaTilesViewLoader
    readonly property int favoritesY: getFavoritesY()

    signal newPosts

    id: postListView
    cacheBuffer: Screen.height * 3
    virtualFooterHeight: userSettings.favoritesBarPosition === QEnums.FAVORITES_BAR_POSITION_BOTTOM ? guiSettings.tabBarHeight : 0

    Timer {
        id: reverseSyncTimer
        interval: 100
        onTriggered: reverseSyncFun()
    }

    Loader {
        id: mediaTilesViewLoader
        active: false

        sourceComponent: MediaTilesFeedView {
            property int favoritesY: getFavoritesY()

            clip: true
            width: postListView.width
            height: postListView.height - (postListView.footerItem && postListView.footerItem.visible ? postListView.footerItem.height : 0)
            headerHeight: postListView.headerItem ? postListView.headerItem.height : 0
            userDid: postListView.userDid
            acceptsInteractions: postListView.acceptsInteractions
            feedDid: postListView.feedDid
            showAsHome: postListView.showAsHome
            model: postListView.model
            virtualFooterHeight: postListView.virtualFooterHeight

            // HACK: grid view does not have a pullback header
            Loader {
                id: headerLoader
                y: headerY
                width: parent.width
                sourceComponent: postListView.header
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

    onContentMoved: updateOnMovement()

    onCovered: {
        if (mediaTilesViewLoader.item)
            mediaTilesViewLoader.item.cover()
    }

    onCountChanged: {
        console.debug((model ? model.feedName : "no feed yet"), count)

        if (!inSync) {
            newLastVisibleIndex = -1
            return
        }

        // Calling later allows the new list elements to render (if they are visible)
        Qt.callLater(calibrateOnCountChanged)
    }

    function calibrateOnCountChanged() {
        const firstVisibleIndex = getFirstVisibleIndex()
        const lastVisibleIndex = getLastVisibleIndex()
        console.debug("Calibration, count changed:", model.feedName, count, "first:", firstVisibleIndex, "last:", lastVisibleIndex, "contentY:", contentY, "originY", originY, "contentHeight", contentHeight)

        updateUnreadPosts()

        if (newLastVisibleIndex >= 0)
            resyncFun()

        newLastVisibleIndex = -1
    }

    function stopSync() {
        console.debug("Stop sync:", model.feedName)
        inSync = false
    }

    function syncDone() {
        console.debug("Sync done:", model.feedName)
        inSync = true
    }

    function updateUnreadPosts(unread = -1) {
        if (unread < 0) {
            if (model.reverseFeed) {
                const lastIndex = getLastVisibleIndex()

                if (lastIndex >= 0)
                    unread = count - lastIndex - 1

            } else {
                const firstIndex = getFirstVisibleIndex()

                if (firstIndex >= 0)
                    unread = firstIndex
            }
        }

        listUnreadPosts = Math.max(unread, 0)
    }

    function updateOnMovement() {
        if (!inSync)
            return

        if (!model)
            return

        const firstVisibleIndex = getFirstVisibleIndex()
        const lastVisibleIndex = getLastVisibleIndex()

        if (firstVisibleIndex < 0 || lastVisibleIndex < 0)
            return

        const remaining = model.reverseFeed ? firstVisibleIndex : count - lastVisibleIndex

        if (remaining < skywalker.TIMELINE_NEXT_PAGE_THRESHOLD && !model.getFeedInProgress) {
            console.debug("Get next feed page:", model.feedName, "first:", firstVisibleIndex, "last:", lastVisibleIndex, "count:", count, "remain:", remaining)
            model.getFeedNextPage(skywalker)
        }

        updateUnreadPosts()
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

    function resetHeaderPosition() {
        if (mediaTilesLoader.item)
            mediaTilesLoader.item.resetHeaderPosition()
        else
            privateResetHeaderPosition()
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
            setModel(model.getUnderlyingModel())
            break
        case QEnums.CONTENT_MODE_VIDEO:
        case QEnums.CONTENT_MODE_VIDEO_TILES:
            setModel(model.getUnderlyingModel().addVideoFilter())
            break
        case QEnums.CONTENT_MODE_MEDIA:
        case QEnums.CONTENT_MODE_MEDIA_TILES:
            setModel(model.getUnderlyingModel().addMediaFilter())
            break
        default:
            console.warn("Unknown content mode:", contentMode)
            return
        }

        if (oldModel.isFilterModel())
            oldModel.getUnderlyingModel().deleteFilteredPostFeedModel(oldModel)

        switch (underlyingModel.feedType) {
            case QEnums.FEED_GENERATOR:
            case QEnums.FEED_LIST:
                if (skywalker.favoriteFeeds.isPinnedFeed(underlyingModel.feedUri))
                    userSettings.setFeedViewMode(skywalker.getUserDid(), underlyingModel.feedUri, contentMode)

                break
            case QEnums.FEED_SEARCH:
                if (skywalker.favoriteFeeds.isPinnedSearch(searchFeed.name))
                    userSettings.setSearchFeedViewMode(skywalker.getUserDid(), underlyingModel.feedName, contentMode)

                break
        }

        mediaTilesLoader.active = [QEnums.CONTENT_MODE_MEDIA_TILES, QEnums.CONTENT_MODE_VIDEO_TILES].includes(contentMode)

        if (lastVisibleIndex > -1) {
            // TODO: what if feed is not chronological
            const newIndex = model.findTimestamp(timestamp, cid)
            syncFun(newIndex, lastVisibleOffsetY)

            if (mediaTilesLoader.item) {
                mediaTilesLoader.item.goToIndex(newIndex)
            }
        }
    }

    function setModel(newModel) {
        // Resetting the model before changing is needed since Qt6.10.1
        // Without resetting, the code will crash
        if (model)
            model.reset()

        disonnectModelHandlers()
        model = newModel
        connectModelHandlers()
    }

    function atStart() {
        if (mediaTilesLoader.item)
            return mediaTilesLoader.item.atYBeginning
        else
            return atYBeginning
    }

    function rowsInsertedHandler(parent, start, end) {
        if (!inSync)
            return

        let firstVisibleIndex = getFirstVisibleIndex()
        const lastVisibleIndex = getLastVisibleIndex()
        console.debug("Calibration, rows inserted, start:", start, "end:", end, "first:", firstVisibleIndex, "last:", lastVisibleIndex, "count:", count, "reversed:", model.reverseFeed)

        if (start <= newLastVisibleIndex)
            newLastVisibleIndex += (end - start + 1)

        updateUnreadPosts()

        if (start === 0)
            newPosts()

        if (model.reverseFeed && (end - start + 1 === count || count === 0)) {
            stopSync()

            // Delay the move to give the ListView time to stabilize
            reverseSyncTimer.start()
        }
    }

    function rowsAboutToBeInsertedHandler(parent, start, end) {
        if (!inSync)
            return

        let firstVisibleIndex = getFirstVisibleIndex()
        const lastVisibleIndex = getLastVisibleIndex()
        console.debug("Calibration, rows to be inserted, start:", start, "end:", end, "first:", firstVisibleIndex, "last:", lastVisibleIndex, "count:", count, "contentY:", contentY, "originY", originY, "contentHeight", contentHeight)

        // When all posts are removed because of a refresh, then count is zero, but
        // first and list visible index are still non-zero
        if (start <= lastVisibleIndex && count > lastVisibleIndex && newLastVisibleIndex < 0) {
            newLastVisibleIndex = lastVisibleIndex
            newLastVisibleOffsetY = calcVisibleOffsetY(lastVisibleIndex)
            console.debug("New last visible index:", newLastVisibleIndex, "offsetY:", newLastVisibleOffsetY)
        }
    }

    function rowsRemovedHandler(parent, start, end) {
        if (!inSync)
            return

        updateUnreadPosts()
        let firstVisibleIndex = getFirstVisibleIndex()
        const lastVisibleIndex = getLastVisibleIndex()
        console.debug("Calibration, rows removed, start:", start, "end:", end, "first:", firstVisibleIndex, "last:", lastVisibleIndex, "count:", count, "contentY:", contentY, "originY", originY, "contentHeight", contentHeight)

        if (end < newLastVisibleIndex)
            newLastVisibleIndex -= (end - start + 1)
    }

    function rowsAboutToBeRemovedHandler(parent, start, end) {
        if (!inSync)
            return

        let firstVisibleIndex = getFirstVisibleIndex()
        const lastVisibleIndex = getLastVisibleIndex()
        console.debug("Calibration, rows to be removed, start:", start, "end:", end, "first:", firstVisibleIndex, "last:", lastVisibleIndex, "count:", count, "contentY:", contentY, "originY", originY, "contentHeight", contentHeight)

        if (end < lastVisibleIndex && newLastVisibleIndex < 0) {
            newLastVisibleIndex = lastVisibleIndex
            newLastVisibleOffsetY = calcVisibleOffsetY(lastVisibleIndex)
            console.debug("New last visible index:", newLastVisibleIndex, "offsetY:", newLastVisibleOffsetY)
        }
    }

    function doMoveToPost(index) {
        let firstVisibleIndex = getFirstVisibleIndex()
        let lastVisibleIndex = getLastVisibleIndex()
        console.debug("Move to:", index, "first:", firstVisibleIndex, "last:", lastVisibleIndex, "count:", count, "contentY:", contentY, "contentHeight", contentHeight)
        positionViewAtIndex(Math.max(index, 0), ListView.End)
        setAnchorItem(firstVisibleIndex, lastVisibleIndex)
        updateUnreadPosts()
        resetHeaderPosition()
        return (lastVisibleIndex >= index - 1 && lastVisibleIndex <= index + 1)
    }

    function disonnectModelHandlers() {
        if (!model)
            return

        model.onRowsInserted.disconnect(rowsInsertedHandler)
        model.onRowsAboutToBeInserted.disconnect(rowsAboutToBeInsertedHandler)
        model.onRowsRemoved.disconnect(rowsRemovedHandler)
        model.onRowsAboutToBeRemoved.disconnect(rowsAboutToBeRemovedHandler)
    }

    function connectModelHandlers() {
        if (!model)
            return

        model.onRowsInserted.connect(rowsInsertedHandler)
        model.onRowsAboutToBeInserted.connect(rowsAboutToBeInsertedHandler)
        model.onRowsRemoved.connect(rowsRemovedHandler)
        model.onRowsAboutToBeRemoved.connect(rowsAboutToBeRemovedHandler)
    }

    Component.onDestruction: {
        disonnectModelHandlers()
    }

    Component.onCompleted: {
        connectModelHandlers()
    }
}
