import QtQuick
import QtQuick.Controls

// SwipeView where the tabs are all SkyListView with PlaceHolderHeader.
// This will keep the the header scrolling aligned.
SwipeView {
    required property int headerHeight
    required property int headerScrollHeight
    readonly property int headerTopY: currentItem.headerTopY
    readonly property int headerTopMinY: calcHeaderMinTopY()
    readonly property int headerTopMaxY: calcHeaderMaxTopY()

    id: swipeListView

    onHeaderTopYChanged: {
        if (headerTopMaxY > -headerScrollHeight)
            Qt.callLater(moveHeaderTopUp)
    }

    onHeaderTopMaxYChanged: {
        if (headerTopMaxY > -headerScrollHeight)
            Qt.callLater(alignHeaderTop)
    }

    function calcHeaderMinTopY() {
        let minTopY = itemAt(0).headerTopY

        for (let i = 1; i < count; ++i)
            minTopY = Math.min(minTopY, itemAt(i).headerTopY)

        return minTopY
    }

    function calcHeaderMaxTopY() {
        let maxTopY = itemAt(0).headerTopY

        for (let i = 1; i < count; ++i)
            maxTopY = Math.max(maxTopY, itemAt(i).headerTopY)

        return maxTopY
    }

    function moveHeaderTopUp() {
        for (let i = 0; i < count; ++i) {
            if (i === currentIndex)
                continue

            let item = itemAt(i)

            if (item.headerTopY <= -swipeListView.headerScrollHeight)
                continue

            const dh = Math.min(item.headerTopY - swipeListView.headerTopMinY, swipeListView.headerScrollHeight)

            if (dh > 0)
                item.contentY += dh
        }
    }

    function alignHeaderTop() {
        for (let i = 0; i < count; ++i) {
            if (i === currentIndex)
                continue

            let item = itemAt(i)
            item.contentY = item.originY - swipeListView.headerTopMaxY
        }
    }
}
