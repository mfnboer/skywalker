import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

GridView {
    required property var skywalker
    readonly property int columns: 3
    readonly property int spacing: 2
    property bool showAsHome: false

    id: postFeedView
    width: parent.width
    cellWidth: width / columns
    cellHeight: cellWidth
    flickDeceleration: guiSettings.flickDeceleration
    maximumFlickVelocity: guiSettings.maxFlickVelocity
    pixelAligned: guiSettings.flickPixelAligned
    ScrollIndicator.vertical: ScrollIndicator {}

    Accessible.name: postFeedView.model.feedName

    delegate: MediaTilesFeedViewDelegate {
        width: postFeedView.cellWidth
        height: postFeedView.cellHeight

        onActivateSwipe: {
            root.viewMediaFeed(model, index, (newIndex) => { postFeedView.positionViewAtIndex(newIndex, ListView.Beginning) })
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
        inProgress: model.feedType === QEnums.FEED_AUTHOR ? skywalker.getAuthorFeedInProgress : skywalker.getFeedInProgress
        verticalOvershoot: postFeedView.verticalOvershoot
        topOvershootFun: () => model.getFeed(skywalker)
        bottomOvershootFun: () => model.getFeedNextPage(skywalker)
        topText: qsTr("Pull down to refresh feed")
        enableScrollToTop: !showAsHome
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
            height: 150
            color: "transparent"

            AccessibleText {
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                padding: 10
                textFormat: Text.RichText
                wrapMode: Text.Wrap
                text: qsTr(`${guiSettings.getFilteredPostsFooterText(model)}<br><a href="load" style="color: ${guiSettings.linkColorDarkMode}; text-decoration: none">Load more</a>`)
                onLinkActivated: model.getFeedNextPage(skywalker)
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
        }
    }

    function getTopRightVisibleIndex() {
        return indexAt(width - 1, contentY)
    }

    function getBottomRightVisibleIndex() {
        return indexAt(width - 1, contentY + height - 1)
    }
}
