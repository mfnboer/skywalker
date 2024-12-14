import QtQuick
import QtQuick.Controls
import skywalker

SkyListView {
    required property var skywalker
    property bool inSync: false
    property bool isView: false
    property int unreadPosts: 0
    property var anchorItem // item used to calibrate list position on insert of new posts
    property int calibrationDy: 0
    property int prevCount: 0
    property int newLastVisibleIndex: -1
    property int newLastVisibleOffsetY: 0

    signal newPosts

    id: timelineView
    width: parent.width
    model: skywalker.timelineModel

    Accessible.name: model ? model.feedName : ""

    footer: AccessibleText {
        width: timelineView.width
        horizontalAlignment: Text.AlignHCenter
        leftPadding: 10
        rightPadding: 10
        topPadding: count > 0 ? 10 : emptyListIndication.height + 10
        bottomPadding: 50
        textFormat: Text.RichText
        wrapMode: Text.Wrap
        text: (isView && model) ? qsTr(`${timelineView.getViewFooterText()}<br><a href="load" style="color: ${guiSettings.linkColor}; text-decoration: none">Load more</a>`) : ""
        onLinkActivated: skywalker.getTimelineNextPage()
    }

    delegate: PostFeedViewDelegate {
        required property int index

        width: timelineView.width

        onCalibratedPosition: (dy) => {
            calibrationDy += dy
            Qt.callLater(calibratePosition)
        }

        onUnfoldPosts: model.unfoldPosts(index)
    }

    onCountChanged: {
        console.debug((model ? model.feedName : "no feed yet"), count)

        if (!inSync) {
            prevCount = count
            newLastVisibleIndex = -1
            return
        }

        const firstVisibleIndex = getFirstVisibleIndex()
        const lastVisibleIndex = getLastVisibleIndex()
        console.debug("Calibration, count changed:", model.feedName, count, "first:", firstVisibleIndex, "last:", lastVisibleIndex, "contentY:", contentY, "originY", originY, "contentHeight", contentHeight)

        updateUnreadPosts(firstVisibleIndex)

        if (count > prevCount)
            newPosts()

        prevCount = count

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

    function calibratePosition() {
        if (calibrationDy === 0)
            return

        console.debug("Calibration, calibrationDy:", calibrationDy)
        timelineView.contentY += calibrationDy
        calibrationDy = 0
        calibrateUnreadPosts()
    }

    function updateUnreadPosts(firstIndex) {
        timelineView.unreadPosts = Math.max(firstIndex, 0)
    }

    function setAnchorItem(firstIndex, lastIndex) {
        const index = firstIndex >= 0 ? firstIndex : lastIndex

        if (index < 0)
            return

        if (anchorItem)
            anchorItem.isAnchorItem = false

        anchorItem = itemAtIndex(index)

        if (anchorItem)
            anchorItem.isAnchorItem = true
    }

    function doMoveToPost(index) {
        const firstVisibleIndex = getFirstVisibleIndex()
        const lastVisibleIndex = getLastVisibleIndex()
        console.debug("Move to:", index, "first:", firstVisibleIndex, "last:", lastVisibleIndex, "count:", count)
        positionViewAtIndex(Math.max(index, 0), ListView.End)
        setAnchorItem(firstVisibleIndex, lastVisibleIndex)
        updateUnreadPosts(firstVisibleIndex)
        return (lastVisibleIndex >= index - 1 && lastVisibleIndex <= index + 1)
    }

    function moveToPost(index, afterMoveCb = () => {}) {
        moveToIndex(index, doMoveToPost, afterMoveCb)
    }

    function moveToHome() {
        positionViewAtBeginning()
        setAnchorItem(0, 0)
        updateUnreadPosts(0)

        if (!isView)
            skywalker.getTimeline(100)
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
        const firstVisibleIndex = getFirstVisibleIndex()
        const lastVisibleIndex = getLastVisibleIndex()
        console.debug("Resume timeline:", index, "first:", firstVisibleIndex, "last:", lastVisibleIndex)

        if (index >= firstVisibleIndex && index <= lastVisibleIndex) {
            console.debug("Index visible:", index)
            return
        }

        moveToPost(index, () => { contentY -= offsetY })
    }

    function rowsInsertedHandler(parent, start, end) {
        let firstVisibleIndex = getFirstVisibleIndex()
        const lastVisibleIndex = getLastVisibleIndex()
        console.debug("Calibration, rows inserted, start:", start, "end:", end, "first:", firstVisibleIndex, "last:", lastVisibleIndex, "contentY:", contentY, "originY", originY, "contentHeight", contentHeight)
        calibrateUnreadPosts()
    }

    function rowsAboutToBeInsertedHandler(parent, start, end) {
        // The Qt.callLater may still be pending when this code executes.
        // Incoming network events have higher prio than callLater.
        calibratePosition()

        let firstVisibleIndex = getFirstVisibleIndex()
        const lastVisibleIndex = getLastVisibleIndex()
        console.debug("Calibration, rows to be inserted, start:", start, "end:", end, "first:", firstVisibleIndex, "last:", lastVisibleIndex, "contentY:", contentY, "originY", originY, "contentHeight", contentHeight)

        if (start <= lastVisibleIndex) {
            newLastVisibleIndex = lastVisibleIndex + (end - start + 1)
            newLastVisibleOffsetY = calcVisibleOffsetY(lastVisibleIndex)
        }
        else {
            newLastVisibleIndex = -1
        }
    }

    function rowsRemovedHandler(parent, start, end) {
        calibrateUnreadPosts()
        let firstVisibleIndex = getFirstVisibleIndex()
        const lastVisibleIndex = getLastVisibleIndex()

        if (start <= lastVisibleIndex) {
            newLastVisibleIndex = lastVisibleIndex - (end - start + 1)
            newLastVisibleOffsetY = calcVisibleOffsetY(lastVisibleIndex)
        }
        else {
            newLastVisibleIndex = -1
        }
    }

    function rowsAboutToBeRemovedHandler(parent, start, end) {
        calibratePosition()
    }

    function setInSync(index, offsetY = 0) {
        console.debug("Sync:", model.feedName, "index:", index, "count:", count, "offsetY:", offsetY)

        if (index >= 0)
            moveToPost(index, () => { contentY -= offsetY })
        else
            moveToEnd()

        inSync = true
        model.onRowsInserted.connect(rowsInsertedHandler)
        model.onRowsAboutToBeInserted.connect(rowsAboutToBeInsertedHandler)
        model.onRowsRemoved.connect(rowsRemovedHandler)
        model.onRowsAboutToBeRemoved.connect(rowsAboutToBeRemovedHandler)
    }

    function stopSync() {
        inSync = false
        model.onRowsInserted.disconnect(rowsInsertedHandler)
        model.onRowsAboutToBeInserted.disconnect(rowsAboutToBeInsertedHandler)
        model.onRowsRemoved.disconnect(rowsRemovedHandler)
        model.onRowsAboutToBeRemoved.disconnect(rowsAboutToBeRemovedHandler)
    }

    function getViewFooterText() {
        if (model.numPostsChecked === 0)
            return qsTr(`No more posts till ${model.checkedTillTimestamp.toLocaleString(Qt.locale(), Locale.ShortFormat)}`)

        return qsTr(`No more posts in ${model.numPostsChecked} timeline posts till ${model.checkedTillTimestamp.toLocaleString(Qt.locale(), Locale.ShortFormat)}`)
    }

    Component.onCompleted: {
        if (!isView)
            footer = null
    }
}
