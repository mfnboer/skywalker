import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

ListView {
    required property var skywalker
    property bool inSync: false
    property int unreadPosts: 0
    property int topIndexBeforeInsert: -1

    id: timelineView
    spacing: 0
    model: skywalker.timelineModel
    clip: true
    flickDeceleration: guiSettings.flickDeceleration
    maximumFlickVelocity: guiSettings.maxFlickVelocity
    pixelAligned: guiSettings.flickPixelAligned
    ScrollIndicator.vertical: ScrollIndicator {}

    Accessible.role: Accessible.List
    Accessible.name: qsTr("Home feed")

    header: PostFeedHeader {
        skywalker: timelineView.skywalker
        feedName: qsTr("Home feed")
        showAsHome: true
        isHomeFeed: true
    }
    headerPositioning: ListView.OverlayHeader

    footer: SkyFooter {
        timeline: timelineView
        skywalker: timelineView.skywalker
        homeActive: true
        onHomeClicked: moveToPost(0)
        onNotificationsClicked: root.viewNotifications()
        onSearchClicked: root.viewSearchView()
        onFeedsClicked: root.viewFeedsView()
        onMessagesClicked: root.viewChat()
    }
    footerPositioning: ListView.OverlayFooter

    delegate: PostFeedViewDelegate {
        width: timelineView.width
    }

    onCountChanged: {
        if (!inSync)
            return

        let firstVisibleIndex = getFirstVisibleIndex()
        let lastVisibleIndex = getLastVisibleIndex()
        // Adding/removing content changes the indices.
        skywalker.timelineMovementEnded(firstVisibleIndex, lastVisibleIndex)
        updateUnreadPosts(firstVisibleIndex)
    }

    onMovementEnded: {
        if (!inSync)
            return

        let firstVisibleIndex = getFirstVisibleIndex()
        let lastVisibleIndex = getLastVisibleIndex()
        skywalker.timelineMovementEnded(firstVisibleIndex, lastVisibleIndex)
        updateUnreadPosts(firstVisibleIndex)
    }

    FlickableRefresher {
        inProgress: skywalker.getTimelineInProgress
        topOvershootFun: () => skywalker.getTimeline(50)
        bottomOvershootFun: () => skywalker.getTimelineNextPage()
        scrollToTopFun: () => moveToPost(0)
        enabled: timelineView.inSync
        topText: qsTr("Pull down to refresh timeline")
        enableScrollToTop: false
    }

    EmptyListIndication {
        y: parent.headerItem ? parent.headerItem.height : 0
        svg: svgOutline.noPosts
        text: qsTr("No posts, follow more people")
        list: timelineView
    }

    BusyIndicator {
        id: busyIndicator
        anchors.centerIn: parent
        running: skywalker.getTimelineInProgress && !skywalker.autoUpdateTimelineInProgress
        Accessible.role: Accessible.ProgressBar
    }

    GuiSettings {
        id: guiSettings
    }

    function getFirstVisibleIndex() {
        let firstVisibleIndex = indexAt(0, contentY)

        if (firstVisibleIndex < 0 && count > 0)
            return 0

        return firstVisibleIndex
    }

    function getLastVisibleIndex() {
        let lastVisibleIndex = indexAt(0, contentY + height - 1)

        if (lastVisibleIndex < 0 && count > 0)
            return count

        return lastVisibleIndex
    }

    function updateUnreadPosts(firstIndex) {
        timelineView.unreadPosts = Math.max(firstIndex, 0)
    }

    function moveToPost(index) {
        positionViewAtIndex(Math.max(index, 0), ListView.Beginning)
        updateUnreadPosts(index)
    }

    function rowsInsertedHandler(parent, start, end) {
        console.debug("ROWS INSERTED:", start, end, "Count:", count, "First visible:", getFirstVisibleIndex())

        if (topIndexBeforeInsert === 0 && start === 0) {
            if (count > end + 1) {
                // Stay at the current item instead of scrolling to the new top
                console.debug("Position at:", end + 1)
                positionViewAtIndex(end + 1, ListView.Beginning)
            }
        }

        const firstVisibleIndex = getFirstVisibleIndex()
        updateUnreadPosts(firstVisibleIndex)
    }

    function rowsAboutToBeInsertedHandler(parent, start, end) {
        topIndexBeforeInsert = getFirstVisibleIndex()
        console.debug("Top index before insert:", topIndexBeforeInsert, "Count:", count)
    }

    function setInSync(index) {
        moveToPost(index)
        inSync = true
        model.onRowsInserted.connect(rowsInsertedHandler)
        model.onRowsAboutToBeInserted.connect(rowsAboutToBeInsertedHandler)
    }

    function stopSync() {
        inSync = false
        model.onRowsInserted.disconnect(rowsInsertedHandler)
        model.onRowsAboutToBeInserted.disconnect(rowsAboutToBeInsertedHandler)
    }
}
