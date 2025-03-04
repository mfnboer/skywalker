import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

GridView {
    required property var skywalker
    readonly property int columns: 3
    readonly property int spacing: 2
    property bool showAsHome: false
    property var enclosingView // used on AuthorView
    property int unreadPosts: 0
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
        height: headerHeight
        color: guiSettings.backgroundColor
    }

    delegate: MediaTilesFeedViewDelegate {
        width: mediaTilesView.cellWidth
        height: mediaTilesView.cellHeight

        onActivateSwipe: {
            let item = mediaTilesView
            root.viewMediaFeed(model, index, (newIndex) => {
                if (item)
                    item.goToIndex(newIndex)
                else
                    console.warn("NO MEDIA TILES VIEW")
            })
        }
    }

    footer: (model && model.endOfFeed) ? endOfFeedComponent : loadMoreComponent

    onCountChanged: {
        updateUnreadPosts()
    }

    onMovementEnded: {
        moveHeader()

        if (virtualFooterHeight !== 0)
            moveVirtualFooter()

        const lastIndex = getBottomRightVisibleIndex()
        console.debug("Move:", mediaTilesView.model.feedName, "index:", lastIndex, "count:", count)

        if (lastIndex >= 0 && count - lastIndex < columns * 6) {
            console.debug("Prefetch next page:", mediaTilesView.model.feedName, "index:", lastIndex, "count:", count)
            model.getFeedNextPage(skywalker)
        }

        updateUnreadPosts()
    }

    FlickableRefresher {
        inProgress: feedLoading
        verticalOvershoot: mediaTilesView.verticalOvershoot
        topOvershootFun: () => model.getFeed(skywalker)
        bottomOvershootFun: () => model.getFeedNextPage(skywalker)
        topText: qsTr("Pull down to refresh feed")
        enableScrollToTop: !showAsHome
        ignoreFooter: true
    }

    EmptyListIndication {
        id: emptyListIndication
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
                anchors.top: footerMargin.bottom
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

        Image {
            z: guiSettings.footerZLevel
            width: mediaTilesView.width
            fillMode: Image.PreserveAspectFit
            source: "/images/thats_all_folks.png"
            visible: mediaTilesView.count > 0
        }
    }

    function updateUnreadPosts() {
        const firstIndex = getTopLeftVisibleIndex()
        mediaTilesView.unreadPosts = Math.max(firstIndex, 0)
    }

    function moveToHome() {
        positionViewAtBeginning()
        updateUnreadPosts()
    }

    function goToIndex(index) {
        const topLeft = getTopLeftVisibleIndex()
        const bottomRight = getBottomRightVisibleIndex()

        if (index < topLeft || index > bottomRight)
            positionViewAtIndex(index, GridView.Beginning)

        updateUnreadPosts()
    }

    function getTopLeftVisibleIndex() {
        return indexAt(1, contentY)
    }

    function getTopRightVisibleIndex() {
        return indexAt(width - 1, contentY)
    }

    function getBottomLeftVisibleIndex() {
        return indexAt(1, contentY + height - 1)
    }

    function getBottomRightVisibleIndex() {
        return indexAt(width - 1, contentY + height - 1)
    }

    Component.onCompleted: {
        prevOriginY = originY
        startY = originY - contentY
        virtualFooterStartY = originY - contentY
    }
}
