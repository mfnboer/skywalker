import QtQuick
import QtQuick.Controls
import skywalker

SkyListView {
    required property var skywalker
    required property int modelId
    property int footerHeight: guiSettings.getNavigationBarHeight()

    signal closed

    id: postFeedView
    width: parent.width
    model: skywalker.getPostFeedModel(modelId)
    snapMode: ListView.SnapOneItem
    spacing: 2 // to avoid the next video peeping at the bottom of the screen sometimes
    currentIndex: 0
    highlightMoveVelocity: 1000

    Accessible.name: postFeedView.model.feedName

    delegate: VideoFeedViewDelegate {
        width: postFeedView.width
        footerHeight: postFeedView.footerHeight
    }

    onCovered: resetSystemBars()
    onUncovered: setSystemBars()

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

    Rectangle {
        z: parent.z - 1
        anchors.fill: parent
        color: guiSettings.fullScreenColor
    }

    function cancel() {
        closed()
    }

    function setSystemBars() {
        skywalker.setStatusBarTransparent(true)
        skywalker.setNavigationBarColorAndMode(guiSettings.fullScreenColor, false)
    }

    function resetSystemBars() {
        skywalker.setStatusBarTransparent(false)
        skywalker.setNavigationBarColor(guiSettings.backgroundColor)
    }

    function setFooterHeight() {
        footerHeight = guiSettings.getNavigationBarHeight()
    }

    Component.onDestruction: {
        resetSystemBars()
        model.clearOverrideLinkColor();
    }

    Component.onCompleted: {
        Screen.onOrientationChanged.connect(setFooterHeight)
        setSystemBars()
        model.setOverrideLinkColor(guiSettings.linkColorDarkMode)
    }
}
