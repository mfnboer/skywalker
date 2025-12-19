import QtQuick
import QtQuick.Controls
import skywalker

SkyListView {
    required property var skywalker
    property int headerMargin: 0
    property bool inSync: false
    property bool isView: false
    property int unreadPosts: 0
    property int newLastVisibleIndex: -1
    property int newLastVisibleOffsetY: 0
    property var userSettings: skywalker.getUserSettings()
    readonly property int visibleHeaderHeight: headerItem ? Math.max(headerItem.height - headerMargin - (contentY - headerItem.y), 0) : 0
    readonly property int favoritesY : getFavoritesY()

    signal newPosts

    id: timelineView
    width: parent.width
    model: skywalker.timelineModel
    cacheBuffer: Screen.height * 3
    virtualFooterHeight: userSettings.favoritesBarPosition === QEnums.FAVORITES_BAR_POSITION_BOTTOM ? guiSettings.tabBarHeight : 0

    Accessible.name: model ? model.feedName : ""

    header: PostFeedHeader {
        feedName: skywalker.timelineModel.feedName
        showAsHome: true
        isHomeFeed: true
        showMoreOptions: true
        showFavoritesPlaceHolder: root.isFavoritesTabBarVisible() && userSettings.favoritesBarPosition === QEnums.FAVORITES_BAR_POSITION_TOP
        bottomMargin: headerMargin
        visible: !root.showSideBar

        onAddUserView: page.addUserView()
        onAddHashtagView: page.addHashtagView()
        onAddFocusHashtagView: page.addFocusHashtagView()
        onAddMediaView: page.showMediaView()
        onAddVideoView: page.showVideoView()
        onFilterStatistics: root.viewContentFilterStats(skywalker.timelineModel)
    }
    headerPositioning: ListView.PullBackHeader

    footer: AccessibleText {
        width: timelineView.width
        horizontalAlignment: Text.AlignHCenter
        leftPadding: 10
        rightPadding: 10
        topPadding: count > 0 ? 10 : emptyListIndication.height + 10
        bottomPadding: 50
        textFormat: Text.RichText
        wrapMode: Text.Wrap
        text: (isView && model) ? qsTr(`${guiSettings.getFilteredPostsFooterText(model)}<br><a href="load" style="color: ${guiSettings.linkColor}; text-decoration: none">Load more</a>`) : ""
        visible: model ? !model.endOfFeed && !Boolean(model.error) : false
        onLinkActivated: skywalker.getTimelineNextPage()
    }

    delegate: PostFeedViewDelegate {
        width: timelineView.width
        swipeMode: [QEnums.CONTENT_MODE_VIDEO, QEnums.CONTENT_MODE_MEDIA].includes(model.contentMode)

        onUnfoldPosts: model.unfoldPosts(index)
        onActivateSwipe: (imgIndex, previewImg) => root.viewMediaFeed(model, index, imgIndex, previewImg, (newIndex) => { moveToPost(newIndex) })
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

        updateUnreadPosts(firstVisibleIndex)

        if (newLastVisibleIndex >= 0)
            resumeTimeline(newLastVisibleIndex, newLastVisibleOffsetY)

        newLastVisibleIndex = -1
    }

    onMovementEnded: {
        if (!inSync)
            return

        const firstVisibleIndex = getFirstVisibleIndex()
        const lastVisibleIndex = getLastVisibleIndex()

        if (!isView) {
            const lastVisibleOffsetY = calcVisibleOffsetY(lastVisibleIndex)
            skywalker.timelineMovementEnded(firstVisibleIndex, lastVisibleIndex, lastVisibleOffsetY)
        }

        setAnchorItem(firstVisibleIndex, lastVisibleIndex)
        updateOnMovement()
    }

    onContentMoved: updateOnMovement()

    function updateOnMovement() {
        if (!inSync)
            return

        const firstVisibleIndex = getFirstVisibleIndex()
        const lastVisibleIndex = getLastVisibleIndex()

        if (count - lastVisibleIndex < skywalker.TIMELINE_NEXT_PAGE_THRESHOLD && !skywalker.getTimelineInProgress) {
            console.debug("Get next timeline page")
            skywalker.getTimelineNextPage()
        }

        if (firstVisibleIndex >= 0)
            updateUnreadPosts(firstVisibleIndex)
    }

    FlickableRefresher {
        inProgress: skywalker.getTimelineInProgress
        topOvershootFun: () => skywalker.updateTimeline(2, skywalker.TIMELINE_PREPEND_PAGE_SIZE)
        bottomOvershootFun: () => skywalker.getTimelineNextPage()
        scrollToTopFun: () => moveToPost(0)
        enabled: timelineView.inSync
        topText: qsTr("Pull down to refresh timeline")
        enableScrollToTop: false
    }

    EmptyListIndication {
        id: emptyListIndication
        y: parent.headerItem ? parent.headerItem.height : 0
        svg: SvgOutline.noPosts
        text: timelineView.isView ? qsTr("No posts") : qsTr("No posts, follow more people")
        list: timelineView
    }

    BusyIndicator {
        id: busyIndicator
        anchors.centerIn: parent
        running: skywalker.getTimelineInProgress && !skywalker.autoUpdateTimelineInProgress
        Accessible.role: Accessible.ProgressBar
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

    function updateUnreadPosts(firstIndex) {
        timelineView.unreadPosts = Math.max(firstIndex, 0)
    }

    function doMoveToPost(index) {
        const firstVisibleIndex = getFirstVisibleIndex()
        const lastVisibleIndex = getLastVisibleIndex()
        console.debug("Move to:", index, "first:", firstVisibleIndex, "last:", lastVisibleIndex, "count:", count, "contentY:", contentY, "contentHeight", contentHeight)
        positionViewAtIndex(Math.max(index, 0), ListView.End)
        setAnchorItem(firstVisibleIndex, lastVisibleIndex)
        updateUnreadPosts(firstVisibleIndex)
        resetHeaderPosition()
        return (lastVisibleIndex >= index - 1 && lastVisibleIndex <= index + 1)
    }

    function moveToPost(index, afterMoveCb = () => {}) {
        moveToIndex(index, doMoveToPost, afterMoveCb)
    }

    function moveToHome() {
        positionViewAtBeginning()
        setAnchorItem(0, 0)
        updateUnreadPosts(0)

        if (!isView) {
            skywalker.timelineMovementEnded(0, 0, 0)
            skywalker.getTimeline(100)
        }
    }

    function moveToEnd(afterMoveCb = () => {}) {
        console.debug("Move to end:", count - 1)
        moveToPost(count - 1, afterMoveCb)
    }

    function calibrateUnreadPosts() {
        const firstVisibleIndex = getFirstVisibleIndex()
        updateUnreadPosts(firstVisibleIndex)
    }

    function resumeTimeline(index, offsetY = 0) {
        if (!inSync)
            return

        const firstVisibleIndex = getFirstVisibleIndex()
        const lastVisibleIndex = getLastVisibleIndex()
        console.debug("Resume timeline:", index, "offsetY:", offsetY, "first:", firstVisibleIndex, "last:", lastVisibleIndex)

        if (index >= firstVisibleIndex && index <= lastVisibleIndex) {
            console.debug("Index visible:", index)
            const lastOffsetY = calcVisibleOffsetY(index)
            console.debug("lastOffsetY:", lastOffsetY, "offsetY:", offsetY)
            contentY += lastOffsetY - offsetY
            return
        }

        moveToPost(index, () => { contentY -= offsetY; resetHeaderPosition() })
    }

    function resyncTimeline(index, offsetY) {
        console.debug("Resync timeline:", index, "offsetY:", offsetY)
        stopSync()
        setInSync(index, offsetY)
    }

    function rowsInsertedHandler(parent, start, end) {
        let firstVisibleIndex = getFirstVisibleIndex()
        const lastVisibleIndex = getLastVisibleIndex()
        console.debug("Calibration, rows inserted, start:", start, "end:", end, "first:", firstVisibleIndex, "last:", lastVisibleIndex, "count:", count, "contentY:", contentY, "originY", originY, "contentHeight", contentHeight)
        calibrateUnreadPosts()

        if (start === 0)
            newPosts()
    }

    function rowsAboutToBeInsertedHandler(parent, start, end) {
        let firstVisibleIndex = getFirstVisibleIndex()
        const lastVisibleIndex = getLastVisibleIndex()
        console.debug("Calibration, rows to be inserted, start:", start, "end:", end, "first:", firstVisibleIndex, "last:", lastVisibleIndex, "count:", count, "contentY:", contentY, "originY", originY, "contentHeight", contentHeight)

        // When all posts are removed because of a refresh, then count is zero, but
        // first and list visible index are still non-zero
        if (start <= lastVisibleIndex && count > lastVisibleIndex) {
            newLastVisibleIndex = lastVisibleIndex + (end - start + 1)
            newLastVisibleOffsetY = calcVisibleOffsetY(lastVisibleIndex)
            console.debug("New last visible index:", newLastVisibleIndex, "offsetY:", newLastVisibleOffsetY)
        }
        else {
            newLastVisibleIndex = -1
        }
    }

    function rowsRemovedHandler(parent, start, end) {
        calibrateUnreadPosts()
        let firstVisibleIndex = getFirstVisibleIndex()
        const lastVisibleIndex = getLastVisibleIndex()
        console.debug("Calibration, rows removed, start:", start, "end:", end, "first:", firstVisibleIndex, "last:", lastVisibleIndex, "count:", count, "contentY:", contentY, "originY", originY, "contentHeight", contentHeight)
    }

    function rowsAboutToBeRemovedHandler(parent, start, end) {
        let firstVisibleIndex = getFirstVisibleIndex()
        const lastVisibleIndex = getLastVisibleIndex()
        console.debug("Calibration, rows to be removed, start:", start, "end:", end, "first:", firstVisibleIndex, "last:", lastVisibleIndex, "count:", count, "contentY:", contentY, "originY", originY, "contentHeight", contentHeight)

        if (end < lastVisibleIndex) {
            newLastVisibleIndex = lastVisibleIndex - (end - start + 1)
            newLastVisibleOffsetY = calcVisibleOffsetY(lastVisibleIndex)
            console.debug("New last visible index:", newLastVisibleIndex, "offsetY:", newLastVisibleOffsetY)
        }
        else {
            newLastVisibleIndex = -1
        }
    }

    function setInSync(index, offsetY = 0) {
        console.debug("Sync:", model.feedName, "index:", index, "count:", count, "offsetY:", offsetY)

        if (index >= 0)
            moveToPost(index, () => { contentY -= offsetY; resetHeaderPosition(); syncDone() })
        else
            moveToEnd(syncDone)
    }

    function syncDone() {
        console.debug("Sync done")
        inSync = true
        model.onRowsInserted.connect(rowsInsertedHandler)
        model.onRowsAboutToBeInserted.connect(rowsAboutToBeInsertedHandler)
        model.onRowsRemoved.connect(rowsRemovedHandler)
        model.onRowsAboutToBeRemoved.connect(rowsAboutToBeRemovedHandler)
    }

    function stopSync() {
        console.debug("Stop sync")
        inSync = false
        model.onRowsInserted.disconnect(rowsInsertedHandler)
        model.onRowsAboutToBeInserted.disconnect(rowsAboutToBeInsertedHandler)
        model.onRowsRemoved.disconnect(rowsRemovedHandler)
        model.onRowsAboutToBeRemoved.disconnect(rowsAboutToBeRemovedHandler)
    }

    Component.onDestruction: {
        if (model)
            stopSync()
    }

    Component.onCompleted: {
        console.debug("Timeline cacheBuffer:", cacheBuffer)

        if (!isView)
            footer = null
    }
}
