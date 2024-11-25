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
            return
        }

        let firstVisibleIndex = getFirstVisibleIndex()
        let lastVisibleIndex = getLastVisibleIndex()

        const index = getLastVisibleIndex()
        console.debug("Calibration, count changed:", model.feedName, count, "first:", firstVisibleIndex, "last:", lastVisibleIndex)

        // Adding/removing content changes the indices.
        if (!isView)
            skywalker.timelineMovementEnded(firstVisibleIndex, lastVisibleIndex)

        updateUnreadPosts(firstVisibleIndex)

        if (count > prevCount)
            newPosts()

        prevCount = count
    }

    onMovementEnded: {
        if (!inSync)
            return

        let firstVisibleIndex = getFirstVisibleIndex()
        let lastVisibleIndex = getLastVisibleIndex()

        if (!isView)
            skywalker.timelineMovementEnded(firstVisibleIndex, lastVisibleIndex)

        setAnchorItem(lastVisibleIndex + 1)
        updateUnreadPosts(firstVisibleIndex)
    }

    FlickableRefresher {
        inProgress: skywalker.getTimelineInProgress
        topOvershootFun: () => skywalker.updateTimeline(2, skywalker.TIMELINE_PREPEND_PAGE_SIZE) // was: skywalker.getTimeline(50)
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
        console.debug("Calibration, calibrationDy:", calibrationDy)
        timelineView.contentY += calibrationDy
        calibrationDy = 0
        calibrateUnreadPosts()
    }

    function updateUnreadPosts(firstIndex) {
        timelineView.unreadPosts = Math.max(firstIndex, 0)
    }

    function setAnchorItem(index) {
        if (anchorItem)
            anchorItem.isAnchorItem = false

        if (index < 0)
            anchorItem = null
        else
            anchorItem = itemAtIndex(index)

        if (anchorItem)
            anchorItem.isAnchorItem = true
    }

    function doMoveToPost(index) {
        const firstVisibleIndex = getFirstVisibleIndex()
        const lastVisibleIndex = getLastVisibleIndex()
        console.debug("Move to:", index, "first:", firstVisibleIndex, "last:", lastVisibleIndex, "count:", count)
        positionViewAtIndex(Math.max(index, 0), ListView.End)
        setAnchorItem(lastVisibleIndex + 1)
        updateUnreadPosts(firstVisibleIndex)
        return (lastVisibleIndex >= index - 1 && lastVisibleIndex <= index + 1)
    }

    function moveToPost(index) {
        moveToIndex(index, doMoveToPost)
    }

    function moveToHome() {
        positionViewAtBeginning()
        setAnchorItem(0)
        updateUnreadPosts(0)
    }

    function moveToEnd() {
        console.debug("Move to end:", count - 1)
        moveToPost(count - 1)
    }

    function calibrateUnreadPosts() {
        const firstVisibleIndex = getFirstVisibleIndex()
        updateUnreadPosts(firstVisibleIndex)
    }

    function rowsInsertedHandler(parent, start, end) {
        let firstVisibleIndex = getFirstVisibleIndex()
        const index = getLastVisibleIndex()
        console.debug("Calibration, rows inserted, start:", start, "end:", end, "first:", firstVisibleIndex, "last:", index)
        calibrateUnreadPosts()
    }

    function rowsAboutToBeInsertedHandler(parent, start, end) {
        let firstVisibleIndex = getFirstVisibleIndex()
        const index = getLastVisibleIndex()
        console.debug("Calibration, rows to be inserted, start:", start, "end:", end, "first:", firstVisibleIndex, "last:", index)
        setAnchorItem(index + 1)
    }

    function setInSync(index) {
        console.debug("Sync:", model.feedName, "index:", index, "count:", count)

        if (index >= 0)
            moveToPost(index)
        else
            moveToEnd()

        inSync = true
        model.onRowsInserted.connect(rowsInsertedHandler)
        model.onRowsAboutToBeInserted.connect(rowsAboutToBeInsertedHandler)
    }

    function stopSync() {
        inSync = false
        model.onRowsInserted.disconnect(rowsInsertedHandler)
        model.onRowsAboutToBeInserted.disconnect(rowsAboutToBeInsertedHandler)
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
