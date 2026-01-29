import QtQuick

SkyListView {
    property bool inSync: false
    property int listUnreadPosts: 0
    property int newLastVisibleIndex: -1
    property int newLastVisibleOffsetY: 0
    property var reverseSyncFun: () => {}
    property var resyncFun: () => {}
    property var resetHeaderFun: () => resetHeaderPosition()

    signal newPosts

    Timer {
        id: reverseSyncTimer
        interval: 100
        onTriggered: reverseSyncFun()
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

    function rowsInsertedHandler(parent, start, end) {
        if (!inSync)
            return

        let firstVisibleIndex = getFirstVisibleIndex()
        const lastVisibleIndex = getLastVisibleIndex()
        console.debug("Calibration, rows inserted, start:", start, "end:", end, "first:", firstVisibleIndex, "last:", lastVisibleIndex, "count:", count, "contentY:", contentY, "originY", originY, "contentHeight", contentHeight)

        if (start <= newLastVisibleIndex)
            newLastVisibleIndex += (end - start + 1)

        updateUnreadPosts()

        if (start === 0)
            newPosts()

        if (model.reverseFeed && end - start + 1 === count || count === 0) {
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
        resetHeaderFun()
        return (lastVisibleIndex >= index - 1 && lastVisibleIndex <= index + 1)
    }

    function disonnectModelHandlers() {
        model.onRowsInserted.disconnect(rowsInsertedHandler)
        model.onRowsAboutToBeInserted.disconnect(rowsAboutToBeInsertedHandler)
        model.onRowsRemoved.disconnect(rowsRemovedHandler)
        model.onRowsAboutToBeRemoved.disconnect(rowsAboutToBeRemovedHandler)
    }

    function connectModelHandlers() {
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
