import QtQuick
import QtQuick.Controls
import skywalker

SkyListView {
    required property var skywalker
    required property int modelId

    signal closed

    id: postFeedView
    width: parent.width
    model: skywalker.getPostFeedModel(modelId)
    snapMode: ListView.SnapOneItem
    currentIndex: 0

    Accessible.name: postFeedView.model.feedName

    delegate: VideoFeedViewDelegate {
        width: postFeedView.width
    }

    onMovementEnded: {
        currentIndex = indexAt(0, contentY)
        console.debug("Move:", postFeedView.model.feedName, "index:", currentIndex, "count:", count)

        if (count - currentIndex < 15) {
            console.debug("Prefetch next page:", postFeedView.model.feedName, "index:", currentIndex, "count:", count)
            skywalker.getFeedNextPage(modelId)
        }
    }

    FlickableRefresher {
        inProgress: skywalker.getFeedInProgress
        verticalOvershoot: postFeedView.verticalOvershoot
        topOvershootFun: () => skywalker.getFeed(modelId)
        bottomOvershootFun: () => skywalker.getFeedNextPage(modelId)
        topText: qsTr("Pull down to refresh feed")
        enableScrollToTop: false
    }

    EmptyListIndication {
        y: parent.headerItem ? parent.headerItem.height : 0
        svg: SvgOutline.noPosts
        text: qsTr("Feed is empty")
        list: postFeedView
    }

    BusyIndicator {
        id: busyIndicator
        anchors.centerIn: parent
        running: skywalker.getFeedInProgress
    }

    function cancel() {
        closed()
    }
}
