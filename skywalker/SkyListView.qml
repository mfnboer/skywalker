import QtQuick
import QtQuick.Controls

ListView {
    property bool enableOnScreenCheck: false
    property var anchorItem // item used to calibrate list position on insert of new posts

    signal covered
    signal uncovered

    spacing: 0
    flickDeceleration: guiSettings.flickDeceleration
    maximumFlickVelocity: guiSettings.maxFlickVelocity
    pixelAligned: guiSettings.flickPixelAligned
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
        property var afterMoveCallback

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
                    afterMoveCallback()
                }
            }
            else {
                afterMoveCallback()
            }
        }

        function go(index, callbackFunc, afterMoveCb = () =>{}) {
            if (!callbackFunc(index)) {
                // HACK: doing it again after a short interval makes the positioning work.
                // After the first time the positioning can be off.
                listIndex = index
                moveAttempt = 2
                callback = callbackFunc
                afterMoveCallback = afterMoveCb
                start()
            }
            else {
                afterMoveCb()
            }
        }
    }

    function moveToIndex(index, callbackFunc, afterMoveCb = () => {}) {
        moveToIndexTimer.go(index, callbackFunc, afterMoveCb)
    }

    // Called when list gets covered by another page
    function cover() {
        covered()

        if (!enableOnScreenCheck)
            return

        for (var i = 0; i < count; ++i) {
            const item = itemAtIndex(i)

            if (item)
                item.cover() // qmllint disable missing-property
        }
    }

    function uncover() {
        uncovered()
    }

    function getFirstVisibleIndex() {
        return indexAt(0, contentY + (headerItem ? headerItem.height : 0))
    }

    function getLastVisibleIndex() {
        return indexAt(0, contentY + height - 1)
    }

    // Calculate offset from bottom of visible section.
    // A positive offset means that the item is partly scrolled down the bottom.
    function calcVisibleOffsetY(index) {
        const item = itemAtIndex(index)

        if (!item)
            return 0

        const hiddenY = contentY + height - item.y
        return item.height - hiddenY
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
}
