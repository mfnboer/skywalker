import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

GridView {
    property string userDid
    property Skywalker skywalker: root.getSkywalker(userDid)
    property bool acceptsInteractions: false
    property string feedDid: ""
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

    property int prevContentY: 0

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
        height: headerHeight
        color: guiSettings.backgroundColor
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

    footer: (model && model.endOfFeed) ? endOfFeedComponent : loadMoreComponent

    onCountChanged: {
        updateUnreadPosts()
    }

    onMovementEnded: {
        prevContentY = contentY
        moveHeader()

        if (virtualFooterHeight !== 0)
            moveVirtualFooter()

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
        const lastVisibleIndex = getBottomRightVisibleIndex()

        if (count - lastVisibleIndex < skywalker.TIMELINE_NEXT_PAGE_THRESHOLD * 2 && Boolean(model) && !feedLoading) {
            console.debug("Get next tiles feed page")
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
            asynchronous: true
            visible: mediaTilesView.count > 0
        }
    }

    function updateUnreadPosts() {
        const firstIndex = getTopLeftVisibleIndex()

        if (firstIndex >= 0)
            mediaTilesView.unreadPosts = firstIndex
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

    function getFirstNonNullIndex() {
        for (let index = getTopLeftVisibleIndex(); index >= 0; --index) {
            if (!itemAtIndex(index))
                return index + 1
        }

        return 0
    }

    function cover() {
        for (let i = getFirstNonNullIndex(); i < count; ++i) {
            const item = itemAtIndex(i)

            if (!item)
                break

            item.cover()
        }
    }

    Component.onCompleted: {
        prevOriginY = originY
        startY = originY - contentY
        virtualFooterStartY = originY - contentY
    }
}
