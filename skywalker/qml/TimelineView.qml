import QtQuick
import QtQuick.Controls
import skywalker

PostListView {
    required property var skywalker
    property int headerMargin: 0
    property bool isView: false
    readonly property int unreadPosts: listUnreadPosts
    readonly property bool reverseFeed: skywalker.timelineModel.reverseFeed
    property var userSettings: skywalker.getUserSettings()
    readonly property int visibleHeaderHeight: headerItem ? Math.max(headerItem.height - headerMargin - (contentY - headerItem.y), 0) : 0
    readonly property int favoritesY : getFavoritesY()

    id: timelineView
    width: parent.width
    reverseSyncFun: () => setInSync(count - 1)
    resyncFun: () => resumeTimeline(newLastVisibleIndex, newLastVisibleOffsetY)
    model: skywalker.timelineModel
    cacheBuffer: Screen.height * 3
    virtualFooterHeight: userSettings.favoritesBarPosition === QEnums.FAVORITES_BAR_POSITION_BOTTOM ? guiSettings.tabBarHeight : 0

    Accessible.name: model ? model.feedName : ""

    header: PostFeedHeader {
        reverseFeed: skywalker.timelineModel.reverseFeed
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

        onNewReverseFeed: (reverse) => {
            userSettings.setReverseTimeline(skywalker.getUserDid(), reverse)

            const [reverseIndex, offsetY] = calcReverseVisibleIndexAndOffsetY(reverse)
            skywalker.timelineModel.reverseFeed = reverse
            moveToPost(reverseIndex, () => { contentY -= offsetY; resetHeaderPosition() })
        }
    }
    headerPositioning: ListView.PullBackHeader

    delegate: PostFeedViewDelegate {
        width: timelineView.width
        swipeMode: [QEnums.CONTENT_MODE_VIDEO, QEnums.CONTENT_MODE_MEDIA].includes(model.contentMode)
        extraFooterHeight: extraHeaderFooterLoader.active && !model.reverseFeed ? extraHeaderFooterLoader.height : 0
        extraHeaderHeight: extraHeaderFooterLoader.active && model.reverseFeed ? extraHeaderFooterLoader.height : 0

        onUnfoldPosts: model.unfoldPosts(index)
        onActivateSwipe: (imgIndex, previewImg) => {
            let view = timelineView
            root.viewMediaFeed(model, index, imgIndex, previewImg,
                (newIndex, mediaIndex, closeCb) => {
                    view.moveToPost(newIndex)
                    view.itemAtIndex(newIndex).closeMedia(mediaIndex, closeCb)
                })
        }

        Loader {
            id: extraHeaderFooterLoader
            y: model.reverseFeed ? 0 : parent.height - height
            active: isView && model && model.isFilterModel() && isLastPost && !endOfFeed

            sourceComponent: FeedViewLoadMore {
                listView: timelineView
            }
        }
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
        const remaining = model.reverseFeed ? firstVisibleIndex : count - lastVisibleIndex

        if (remaining < skywalker.TIMELINE_NEXT_PAGE_THRESHOLD && !skywalker.getTimelineInProgress) {
            console.debug("Get next timeline page")
            skywalker.getTimelineNextPage()
        }

        if (firstVisibleIndex >= 0)
            updateUnreadPosts()
    }

    FlickableRefresher {
        reverseFeed: model.reverseFeed
        inProgress: skywalker.getTimelineInProgress
        topOvershootFun: reverseFeed ?
                             () => skywalker.getTimelineNextPage() :
                             () => skywalker.updateTimeline(2, skywalker.TIMELINE_PREPEND_PAGE_SIZE)
        bottomOvershootFun: reverseFeed ?
                                () => skywalker.updateTimeline(2, skywalker.TIMELINE_PREPEND_PAGE_SIZE) :
                                () => skywalker.getTimelineNextPage()
        scrollToTopFun: reverseFeed ? () => moveToPost(0) : () => moveToEnd()
        enabled: timelineView.inSync
        topText: reverseFeed ? qsTr("Pull up to refresh timeline") : qsTr("Pull down to refresh timeline")
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

    Loader {
        anchors.top: emptyListIndication.bottom
        active: isView && model && count === 0 && !model.endOfFeed && !Boolean(model.error)

        sourceComponent: FeedViewLoadMore {
            listView: timelineView
        }
    }

    Timer {
        id: reverseSyncTimer
        interval: 100
        onTriggered: setInSync(count - 1)
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

    function moveToPost(index, afterMoveCb = () => {}) {
        moveToIndex(index, doMoveToPost, afterMoveCb)
    }

    function moveToHome() {
        if (model.reverseFeed)
            positionViewAtEnd()
        else
            positionViewAtBeginning()

        const homeIndex = model.reverseFeed ? count - 1 : 0
        setAnchorItem(homeIndex, homeIndex)
        updateUnreadPosts(0)

        if (!isView) {
            skywalker.timelineMovementEnded(homeIndex, homeIndex, 0)
            skywalker.getTimeline(100)
        }
    }

    function moveToEnd(afterMoveCb = () => {}) {
        console.debug("Move to end:", count - 1)
        moveToPost(count - 1, afterMoveCb)
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

    function setInSync(index, offsetY = 0) {
        console.debug("Sync:", model.feedName, "index:", index, "count:", count, "offsetY:", offsetY)

        if (index >= 0)
            moveToPost(index, () => { contentY -= offsetY; resetHeaderPosition(); syncDone() })
        else
            moveToEnd(syncDone)
    }
}
