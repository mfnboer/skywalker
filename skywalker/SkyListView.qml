import QtQuick
import QtQuick.Controls

ListView {
    property bool enableOnScreenCheck: false

    spacing: 0
    flickDeceleration: guiSettings.flickDeceleration
    maximumFlickVelocity: guiSettings.maxFlickVelocity
    pixelAligned: guiSettings.flickPixelAligned
    cacheBuffer: 15000
    ScrollIndicator.vertical: ScrollIndicator {}

    Accessible.role: Accessible.List

    onMovementEnded: {
        if (!enableOnScreenCheck)
            return

        for (var i = 0; i < count; ++i) {
            const item = itemAtIndex(i)

            if (item)
                item.checkOnScreen() // qmllint disable missing-property
        }
    }

    Timer {
        property int listIndex
        property int moveAttempt
        property var callback

        id: moveToIndexTimer
        interval: 200
        onTriggered: {
            if (!callback(listIndex)) { // qmllint disable use-proper-function
                if (moveAttempt < 5) {
                    moveAttempt = moveAttempt + 1
                    start()
                }
                else {
                    console.debug("No exact move, no more attempts")
                }
            }
        }

        function go(index, callbackFunc) {
            if (!callbackFunc(index)) {
                // HACK: doing it again after a short interval makes the positioning work.
                // After the first time the positioning can be off.
                listIndex = index
                moveAttempt = 2
                callback = callbackFunc
                start()
            }
        }
    }

    function moveToIndex(index, callbackFunc) {
        moveToIndexTimer.go(index, callbackFunc)
    }

    // Called when list gets covered by another page
    function cover() {
        if (!enableOnScreenCheck)
            return

        for (var i = 0; i < count; ++i) {
            const item = itemAtIndex(i)

            if (item)
                item.cover() // qmllint disable missing-property
        }
    }

    function getFirstVisibleIndex() {
        let firstVisibleIndex = indexAt(0, contentY + headerItem.height)

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
}
