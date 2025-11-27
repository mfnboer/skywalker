import QtQuick
import QtQuick.Controls

ListView {
    property bool enableOnScreenCheck: false
    property var anchorItem // item used to calibrate list position on insert of new posts
    readonly property string error: (model && typeof model.error != 'undefined') ? model.error : ""

    readonly property int headerTopY: headerItem ? headerItem.y - contentY : 0

    property int prevOriginY: 0
    property int virtualFooterHeight: 0
    property int virtualFooterStartY: 0
    readonly property int virtualFooterTopY: (originY - contentY) - virtualFooterStartY + height - virtualFooterHeight + verticalOvershoot
    readonly property int virtualFooterY: virtualFooterTopY < height ? Math.max(virtualFooterTopY, height - virtualFooterHeight) : height
    property int prevContentY: 0

    signal contentMoved()

    onOriginYChanged: {
        virtualFooterStartY += (originY - prevOriginY)
        prevOriginY = originY
    }

    function moveVirtualFooter() {
        if (virtualFooterTopY < height - virtualFooterHeight) {
            virtualFooterStartY = originY - contentY + verticalOvershoot
        }
        else if (virtualFooterTopY > height) {
            virtualFooterStartY = originY - contentY - virtualFooterHeight + verticalOvershoot
        }
    }

    signal covered
    signal uncovered

    spacing: 0
    flickDeceleration: guiSettings.flickDeceleration
    maximumFlickVelocity: guiSettings.maxFlickVelocity
    pixelAligned: guiSettings.flickPixelAligned
    ScrollIndicator.vertical: ScrollIndicator {}

    Accessible.role: Accessible.List

    onMovementEnded: {
        prevContentY = contentY

        if (virtualFooterHeight !== 0)
            moveVirtualFooter()

        if (!enableOnScreenCheck)
            return



        for (let index = getFirstNonNullIndex(); index < count; ++index) {
            const item = itemAtIndex(index)

            if (!item)
                break

            item.checkOnScreen() // qmllint disable missing-property
        }
    }

    onContentYChanged: {
        // Throttle the callbacks in order not to do something on every
        // contentY change.
        if (Math.abs(contentY - prevContentY) > 250) {
            prevContentY = contentY
            contentMoved()
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

        for (let i = getFirstNonNullIndex(); i < count; ++i) {
            const item = itemAtIndex(i)

            if (!item)
                break

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

    function getFirstNonNullIndex() {
        for (let index = getFirstVisibleIndex(); index >= 0; --index) {
            if (!itemAtIndex(index))
                return index + 1
        }

        return 0
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

    function getAnchorIndex() {
        if (!anchorItem)
            return -1

        return anchorItem.index
    }

    function privateResetHeaderPosition() {
        if (headerItem)
            headerItem.y = contentY

        virtualFooterStartY = originY - contentY
    }

    function resetHeaderPosition() {
        privateResetHeaderPosition()
    }

    function getHeaderHeight() {
        return (headerItem ? headerItem.height + headerTopY : 0) + guiSettings.headerMargin
    }

    function getFooterHeight() {
        return (footerItem ? footerItem.height : 0) + guiSettings.footerMargin
    }

    Component.onCompleted: {
        prevOriginY = originY
        virtualFooterStartY = originY - contentY
    }
}
