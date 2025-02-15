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
    readonly property bool feedLoading: model.feedType === QEnums.FEED_AUTHOR ? skywalker.getAuthorFeedInProgress : skywalker.getFeedInProgress

    id: postFeedView
    width: parent.width
    cellWidth: width / columns
    cellHeight: cellWidth
    flickDeceleration: guiSettings.flickDeceleration
    maximumFlickVelocity: guiSettings.maxFlickVelocity
    pixelAligned: guiSettings.flickPixelAligned
    interactive: enclosingView ? !enclosingView.interactive : true
    ScrollIndicator.vertical: ScrollIndicator {}

    Accessible.name: postFeedView.model.feedName

    onVerticalOvershootChanged: {
        if (enclosingView && verticalOvershoot < 0)
            enclosingView.interactive = true
    }

    delegate: MediaTilesFeedViewDelegate {
        width: postFeedView.cellWidth
        height: postFeedView.cellHeight

        onActivateSwipe: {
            root.viewMediaFeed(model, index, (newIndex) => { postFeedView.goToIndex(newIndex) })
        }
    }

    footer: model.endOfFeed ? endOfFeedComponent : loadMoreComponent

    onMovementEnded: {
        const lastIndex = getBottomRightVisibleIndex()
        console.debug("Move:", postFeedView.model.feedName, "index:", lastIndex, "count:", count)

        if (lastIndex >= 0 && count - lastIndex < columns * 6) {
            console.debug("Prefetch next page:", postFeedView.model.feedName, "index:", lastIndex, "count:", count)
            model.getFeedNextPage(skywalker)
        }
    }

    FlickableRefresher {
        inProgress: feedLoading
        verticalOvershoot: postFeedView.verticalOvershoot
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
        list: postFeedView
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
            width: postFeedView.width
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
            width: postFeedView.width
            fillMode: Image.PreserveAspectFit
            source: "/images/thats_all_folks.png"
            visible: postFeedView.count > 0
        }
    }

    function goToIndex(index) {
        const topLeft = getTopLeftVisibleIndex()
        const bottomRight = getBottomRightVisibleIndex()

        if (index < topLeft || index > bottomRight)
            positionViewAtIndex(index, GridView.Beginning)
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
}
