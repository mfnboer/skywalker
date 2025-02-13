import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

GridView {
    required property var skywalker
    readonly property int columns: 3
    readonly property int spacing: 3
    property bool showAsHome: false

    id: postFeedView
    width: parent.width
    cellWidth: width / columns
    cellHeight: cellWidth
    flickDeceleration: guiSettings.flickDeceleration
    maximumFlickVelocity: guiSettings.maxFlickVelocity
    pixelAligned: guiSettings.flickPixelAligned
    cacheBuffer: 5000
    ScrollIndicator.vertical: ScrollIndicator {}

    Accessible.name: postFeedView.model.feedName

    delegate: MediaTilesFeedViewDelegate {
        width: postFeedView.cellWidth
        height: postFeedView.cellHeight
        Layout.alignment: Qt.AlignRight

        onActivateSwipe: {
            root.viewMediaFeed(model, index, (newIndex) => { postFeedView.positionViewAtIndex(newIndex, ListView.Beginning) })
        }

        // Loader {
        //     id: extraFooterLoader
        //     anchors.bottom: parent.bottom

        //     active: model.isFilterModel() && index == count - 1 && !endOfFeed
        //     sourceComponent: extraFooterComponent
        // }
    }

    onMovementEnded: {
        const lastIndex = getBottomRightVisibleIndex()
        console.debug("Move:", postFeedView.model.feedName, "index:", lastIndex, "count:", count)

        if (lastIndex >= 0 && count - lastIndex < columns * 5) {
            console.debug("Prefetch next page:", postFeedView.model.feedName, "index:", lastIndex, "count:", count)
            model.getFeedNextPage(skywalker)
        }
    }

    FlickableRefresher {
        inProgress: model.feedType === QEnums.FEED_AUTHOR ? skywalker.getAuthorFeedInProgress : skywalker.getFeedInProgress
        verticalOvershoot: postFeedView.verticalOvershoot
        topOvershootFun: () => model.getFeed(skywalker)
        bottomOvershootFun: () => model.getFeedNextPage(skywalker)
        enableScrollToTop: !showAsHome
    }

    EmptyListIndication {
        y: parent.headerItem ? parent.headerItem.height : 0
        svg: SvgOutline.noPosts
        text: qsTr("Feed is empty")
        list: postFeedView
    }

    Rectangle {
        z: parent.z - 1
        anchors.fill: parent
        color: guiSettings.fullScreenColor
    }

    Component {
        id: extraFooterComponent

        Rectangle {
            width: postFeedView.width
            height: 150
            color: "transparent"

            AccessibleText {
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                padding: 10
                textFormat: Text.RichText
                wrapMode: Text.Wrap
                color: "white"
                text: qsTr(`${guiSettings.getFilteredPostsFooterText(model)}<br><a href="load" style="color: ${guiSettings.linkColorDarkMode}; text-decoration: none">Load more</a>`)
                onLinkActivated: model.getFeedNextPage(skywalker)
            }
        }
    }

    function getTopRightVisibleIndex() {
        return indexAt(width - 1, contentY)
    }

    function getBottomRightVisibleIndex() {
        return indexAt(width - 1, contentY + height - 1)
    }
}
