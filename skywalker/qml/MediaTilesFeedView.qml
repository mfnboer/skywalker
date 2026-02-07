import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

GridView {
    property string userDid
    property Skywalker skywalker: root.getSkywalker(userDid)
    property bool acceptsInteractions: false
    property string feedDid: ""
    readonly property int columns: skywalker.TILE_VIEW_ROW_SIZE
    readonly property int spacing: 2
    property bool showAsHome: false
    property var enclosingView // used on AuthorView
    property int unreadPosts: 0
    readonly property bool reverseFeed: model ? model.reverseFeed : false
    property int newFirstVisibleIndex: -1
    readonly property bool feedLoading: model ? model.getFeedInProgress : false

    property int headerHeight: 0
    property int startY: 0
    readonly property int topY: (originY - contentY) - startY + verticalOvershoot
    readonly property int headerY: topY < 0 ? Math.max(topY, -headerHeight) : 0

    property int prevOriginY: 0
    property int virtualFooterHeight: 0
    property int virtualFooterStartY: 0
    readonly property int virtualFooterTopY: (originY - contentY) - virtualFooterStartY + height - virtualFooterHeight + verticalOvershoot
    readonly property int virtualFooterY: virtualFooterTopY < height ? Math.max(virtualFooterTopY, height - virtualFooterHeight) : height

    property int prevContentY: 0
    property bool inSync: true

    signal contentMoved

    onOriginYChanged: {
        virtualFooterStartY += (originY - prevOriginY)
        prevOriginY = originY
    }

    function moveHeader() {
        if (topY < -headerHeight)
            startY = topY + startY + headerHeight
        else if (topY > 0)
            startY = topY + startY
    }

    function moveVirtualFooter() {
        if (virtualFooterTopY < height - virtualFooterHeight) {
            virtualFooterStartY = originY - contentY + verticalOvershoot
        }
        else if (virtualFooterTopY > height) {
            virtualFooterStartY = originY - contentY - virtualFooterHeight + verticalOvershoot
        }
    }

    function resetHeaderPosition() {
        startY = originY - contentY
        virtualFooterStartY = originY - contentY
    }

    id: mediaTilesView
    width: parent.width
    cellWidth: width / columns
    cellHeight: cellWidth
    flickDeceleration: guiSettings.flickDeceleration
    maximumFlickVelocity: guiSettings.maxFlickVelocity
    pixelAligned: guiSettings.flickPixelAligned
    interactive: enclosingView ? !enclosingView.interactive : true
    ScrollIndicator.vertical: ScrollIndicator {}

    onVerticalOvershootChanged: {
        if (enclosingView && verticalOvershoot < 0)
            enclosingView.interactive = true
    }

    header: Rectangle {
        width: parent.width
        height: headerHeight + (endOfReverseFeedLoader.active ? endOfReverseFeedLoader.height : 0)
        color: guiSettings.backgroundColor

        Loader {
            id: endOfReverseFeedLoader
            y: headerHeight
            active: model && model.reverseFeed && count > 0
            sourceComponent: (model && model.endOfFeed) ? endOfFeedComponent : loadMoreComponent
        }
    }

    delegate: MediaTilesFeedViewDelegate {
        width: mediaTilesView.cellWidth
        height: mediaTilesView.cellHeight
        feedAcceptsInteractions: mediaTilesView.acceptsInteractions
        feedDid: mediaTilesView.feedDid

        onActivateSwipe: (imgIndex, previewImg) => {
            let item = mediaTilesView
            root.viewMediaFeed(model, index, imgIndex, previewImg, (newIndex, mediaIndex, closeCb) => {
                if (item) {
                    item.goToIndex(newIndex)
                    item.itemAtIndex(newIndex).closeMedia(mediaIndex, closeCb)
                }
                else {
                    console.warn("NO MEDIA TILES VIEW")
                }
            }, userDid)
        }
    }

    footer: Rectangle {
        width: parent.width
        height: endOfFeedLoader.active ? endOfFeedLoader.height : 0
        color: guiSettings.backgroundColor

        Loader {
            id: endOfFeedLoader
            active: model && (!model.reverseFeed || count === 0)
            sourceComponent: (model && model.endOfFeed) ? endOfFeedComponent : loadMoreComponent
        }
    }

    onCountChanged: {
        if (!inSync) {
            newFirstVisibleIndex = -1
            return
        }

        Qt.callLater(calibrateOnCountChanged)
    }

    function calibrateOnCountChanged() {
        const firstVisibleIndex = getTopLeftVisibleIndex()
        const lastVisibleIndex = getBottomRightVisibleIndex()
        console.debug("Calibration, tiles count changed:", model.feedName, count, "first:", firstVisibleIndex, "last:", lastVisibleIndex, "contentY:", contentY, "originY", originY, "contentHeight", contentHeight)

        updateUnreadPosts()

        if (newFirstVisibleIndex >= 0)
            goToIndex(newFirstVisibleIndex)

        newFirstVisibleIndex = -1
    }

    onMovementEnded: {
        prevContentY = contentY
        moveHeader()

        if (virtualFooterHeight !== 0)
            moveVirtualFooter()

        if (!inSync)
            return

        const firstVisibleIndex = getTopLeftVisibleIndex()
        const lastVisibleIndex = getBottomRightVisibleIndex()

        if (firstVisibleIndex !== -1 && model)
        {
            if (model.feedType === QEnums.FEED_SEARCH)
                skywalker.searchFeedMovementEnded(model.getModelId(), model.contentMode, firstVisibleIndex, 0)
            else
                skywalker.feedMovementEnded(model.getModelId(), model.contentMode, firstVisibleIndex, 0)
        }

        updateOnMovement()
    }

    onContentYChanged: {
        if (Math.abs(contentY - prevContentY) > mediaTilesView.cellHeight) {
            prevContentY = contentY
            contentMoved()
            updateOnMovement()
        }
    }

    function updateOnMovement() {
        if (!inSync)
            return

        if (!model)
            return

        const firstVisibleIndex = getTopLeftVisibleIndex()
        const lastVisibleIndex = getBottomRightVisibleIndex()
        const remaining = model.reverseFeed ? firstVisibleIndex : count - lastVisibleIndex

        if (remaining < skywalker.TIMELINE_NEXT_PAGE_THRESHOLD * 2 && !feedLoading) {
            console.debug("Get next tiles feed page:", model.feedName, "remain:", remaining)
            model.getFeedNextPage(skywalker)
        }

        updateUnreadPosts()
    }

    FlickableRefresher {
        reverseFeed: model.reverseFeed
        inProgress: feedLoading
        verticalOvershoot: mediaTilesView.verticalOvershoot
        topOvershootFun: reverseFeed ? () => model.getFeedNextPage(skywalker) : () => model.getFeed(skywalker)
        bottomOvershootFun: reverseFeed ? () => model.getFeed(skywalker) : () => model.getFeedNextPage(skywalker)
        topText: reverseFeed ? qsTr("Pull up to refresh feed") : qsTr("Pull down to refresh feed")
        enableScrollToTop: !showAsHome
        ignoreFooter: true
    }

    EmptyListIndication {
        id: emptyListIndication
        y: headerHeight
        svg: SvgOutline.noPosts
        text: qsTr("Feed is empty")
        list: mediaTilesView
    }

    BusyIndicator {
        id: busyIndicator
        anchors.centerIn: parent
        running: feedLoading && count > 0 && getBottomLeftVisibleIndex() >= 0
    }

    Rectangle {
        z: parent.z - 1
        anchors.fill: parent
        color: guiSettings.backgroundColor
    }

    Component {
        id: loadMoreComponent

        Rectangle {
            z: guiSettings.footerZLevel
            width: mediaTilesView.width
            height: 150 + footerMargin.height
            color: "transparent"

            Rectangle {
                id: footerMargin
                width: parent.width
                height: emptyListIndication.visible ? emptyListIndication.height : 0
                color: "transparent"
            }

            AccessibleText {
                id: loadMoreText
                width: parent.width
                y: model.reverseFeed ? parent.height - height - 30 : footerMargin.height
                horizontalAlignment: Text.AlignHCenter
                padding: 10
                textFormat: Text.RichText
                wrapMode: Text.Wrap
                text: qsTr(`${guiSettings.getFilteredPostsFooterText(model)}<br><a href="load" style="color: ${guiSettings.linkColorDarkMode}; text-decoration: none">Load more</a>`)
                onLinkActivated: model.getFeedNextPage(skywalker)
            }

            BusyIndicator {
                anchors.top: loadMoreText.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                width: 30
                height: width
                running: feedLoading
            }
        }
    }

    Component {
        id: endOfFeedComponent

        AccessibleText {
            z: guiSettings.footerZLevel
            width: mediaTilesView.width
            horizontalAlignment: Text.AlignHCenter
            topPadding: 10
            bottomPadding: model.reverseFeed ? 10 : 50
            elide: Text.ElideRight
            font.italic: true
            text: qsTr("End of feed")
            visible: mediaTilesView.count > 0
        }
    }

    Timer {
        id: reverseSyncTimer
        interval: 100
        onTriggered: {
            goToIndex(count - 1)
            syncDone()
        }
    }

    MoveToIndexTimer {
        id: moveToIndexTimer
    }

    function moveToIndex(index, callbackFunc, afterMoveCb = () => {}) {
        moveToIndexTimer.go(index, callbackFunc, afterMoveCb)
    }

    function updateUnreadPosts() {
        if (model.reverseFeed) {
            const lastIndex = getBottomRightVisibleIndex()

            if (lastIndex >= 0)
                mediaTilesView.unreadPosts = count - lastIndex - 1

        } else {
            const firstIndex = getTopLeftVisibleIndex()

            if (firstIndex >= 0)
                mediaTilesView.unreadPosts = firstIndex
        }
    }

    function doMoveToPost(index) {
        let firstVisibleIndex = getTopLeftVisibleIndex()
        let lastVisibleIndex = getBottomRightVisibleIndex()
        console.debug("Move to tile:", index, "first:", firstVisibleIndex, "last:", lastVisibleIndex, "count:", count, "contentY:", contentY)
        positionViewAtIndex(Math.max(index, 0), GridView.Beginning)
        updateUnreadPosts()
        resetHeaderPosition()
        return (firstVisibleIndex <= index && index <= lastVisibleIndex)
    }

    function moveToHome() {
        if (model.reverseFeed) {
            goToIndex(count - 1)
        } else {
            positionViewAtBeginning()
            updateUnreadPosts()
        }
    }

    function goToIndex(index) {
        const topLeft = getTopLeftVisibleIndex()
        const bottomRight = getBottomRightVisibleIndex()
        console.debug("Go to tile:", model.feedName, "index:", index, "first:", topLeft, "last:", bottomRight)

        if (topLeft < 0 || bottomRight < 0 || index < topLeft || index > bottomRight)
            moveToIndex(index, doMoveToPost)
    }

    function getTopLeftVisibleIndex() {
        return indexAt(1, contentY)
    }

    function getTopRightVisibleIndex() {
        const index = indexAt(width - 1, contentY)
        return (index < 0 && count > 0) ? count - 1 : index
    }

    function getBottomLeftVisibleIndex() {
        return indexAt(1, contentY + height - 1)
    }

    function getBottomRightVisibleIndex() {
        const index = indexAt(width - 1, contentY + height - 1)
        return (index < 0 && count > 0) ? count - 1 : index
    }

    function getFirstNonNullIndex() {
        for (let index = getTopLeftVisibleIndex(); index >= 0; --index) {
            if (!itemAtIndex(index))
                return index + 1
        }

        return 0
    }

    function calcReverseVisibleIndex(toReverseFeed) {
        const index = getBottomRightVisibleIndex()
        console.debug("Bottom right index:", index)
        const reverseIndex = count - index - 1
        return reverseIndex
    }

    function cover() {
        for (let i = getFirstNonNullIndex(); i < count; ++i) {
            const item = itemAtIndex(i)

            if (!item)
                break

            item.cover()
        }
    }

    function rowsInsertedHandler(parent, start, end) {
        if (!inSync)
            return

        let firstVisibleIndex = getTopLeftVisibleIndex()
        const lastVisibleIndex = getBottomRightVisibleIndex()
        console.debug("Calibration, tiles inserted:", model.feedName, "start:", start, "end:", end, "first:", firstVisibleIndex, "last:", lastVisibleIndex, "count:", count, "contentY:", contentY, "originY", originY, "contentHeight", contentHeight)

        if (start <= newFirstVisibleIndex)
            newFirstVisibleIndex += (end - start + 1)

        updateUnreadPosts()

        if (model.reverseFeed && (end - start + 1 === count || count === 0))
        {
            stopSync()
            reverseSyncTimer.start()
        }
    }

    function rowsAboutToBeInsertedHandler(parent, start, end) {
        if (!inSync)
            return

        let firstVisibleIndex = getTopLeftVisibleIndex()
        const lastVisibleIndex = getBottomRightVisibleIndex()
        console.debug("Calibration, tiles to be inserted:", model.feedName, "start:", start, "end:", end, "first:", firstVisibleIndex, "last:", lastVisibleIndex, "count:", count, "contentY:", contentY, "originY", originY, "contentHeight", contentHeight)

        // When all posts are removed because of a refresh, then count is zero, but
        // first and list visible index are still non-zero
        if (start <= firstVisibleIndex && count > firstVisibleIndex && newFirstVisibleIndex < 0) {
            newFirstVisibleIndex = firstVisibleIndex
            console.debug("New first visible tile index:", newFirstVisibleIndex)
        }
    }

    function rowsRemovedHandler(parent, start, end) {
        if (!inSync)
            return

        updateUnreadPosts()
        let firstVisibleIndex = getTopLeftVisibleIndex()
        const lastVisibleIndex = getBottomRightVisibleIndex()
        console.debug("Calibration, tiles removed:", model.feedName, "start:", start, "end:", end, "first:", firstVisibleIndex, "last:", lastVisibleIndex, "count:", count, "contentY:", contentY, "originY", originY, "contentHeight", contentHeight)

        if (start <= firstVisibleIndex && newFirstVisibleIndex < 0)
            newFirstVisibleIndex -= (end - start + 1)
    }

    function rowsAboutToBeRemovedHandler(parent, start, end) {
        if (!inSync)
            return

        let firstVisibleIndex = getTopLeftVisibleIndex()
        const lastVisibleIndex = getBottomRightVisibleIndex()
        console.debug("Calibration, tiles to be removed:", model.feedName, "start:", start, "end:", end, "first:", firstVisibleIndex, "last:", lastVisibleIndex, "count:", count, "contentY:", contentY, "originY", originY, "contentHeight", contentHeight)

        if (start <= firstVisibleIndex && newFirstVisibleIndex < 0) {
            newFirstVisibleIndex = firstVisibleIndex
            console.debug("New first visible index:", newFirstVisibleIndex, "offsetY:", newLastVisibleOffsetY)
        }
    }

    function setInSync(index) {
        console.debug("Sync tiles:", model.feedName, "index:", index, "count:", count)
        const homeIndex = model.reverseFeed ? count - 1 : 0

        if (index === homeIndex || index < 0)
            moveToHome()
        else
            goToIndex(index)

        syncDone()
    }

    function stopSync() {
        console.debug("Stop sync tiles:", model.feedName)
        inSync = false
    }

    function syncDone() {
        console.debug("Sync done tiles:", model.feedName)
        inSync = true
    }

    function reverseFeedHandler() {
        console.debug("Reverse feed tiles changed:", model.reverseFeed, model.feedName)
        const reverseIndex = calcReverseVisibleIndex(model.reverseFeed)
        console.debug("Reverse index:", reverseIndex)
        goToIndex(reverseIndex)
    }

    Component.onDestruction: {
        model.onRowsInserted.disconnect(rowsInsertedHandler)
        model.onRowsAboutToBeInserted.disconnect(rowsAboutToBeInsertedHandler)
        model.onRowsRemoved.disconnect(rowsRemovedHandler)
        model.onRowsAboutToBeRemoved.disconnect(rowsAboutToBeRemovedHandler)
        model.onReverseFeedChanged.disconnect(reverseFeedHandler)
    }

    Component.onCompleted: {
        prevOriginY = originY
        startY = originY - contentY
        virtualFooterStartY = originY - contentY

        model.onRowsInserted.connect(rowsInsertedHandler)
        model.onRowsAboutToBeInserted.connect(rowsAboutToBeInsertedHandler)
        model.onRowsRemoved.connect(rowsRemovedHandler)
        model.onRowsAboutToBeRemoved.connect(rowsAboutToBeRemovedHandler)
        model.onReverseFeedChanged.connect(reverseFeedHandler)
    }
}
