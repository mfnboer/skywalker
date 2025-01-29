import QtQuick
import QtQuick.Controls
import skywalker

SkyListView {
    required property var skywalker
    property int headerHeight: guiSettings.getStatusBarSize(QEnums.INSETS_SIDE_TOP)
    property int footerHeight: guiSettings.getNavigationBarSize(QEnums.INSETS_SIDE_BOTTOM)
    property int leftMarginWidth: guiSettings.getNavigationBarSize(QEnums.INSETS_SIDE_LEFT)

    signal closed

    id: postFeedView
    width: parent.width
    boundsBehavior: Flickable.StopAtBounds
    snapMode: ListView.SnapOneItem
    spacing: 2 // to avoid the next video peeping at the bottom of the screen sometimes
    currentIndex: 0
    highlightMoveVelocity: 1000

    Accessible.name: postFeedView.model.feedName

    delegate: VideoFeedViewDelegate {
        width: postFeedView.width
        footerHeight: postFeedView.footerHeight
        headerHeight: postFeedView.headerHeight
        leftMarginWidth: postFeedView.leftMarginWidth
        extraFooterHeight: extraFooterLoader.active ? extraFooterLoader.height : 0

        onClosed: postFeedView.closed()

        Loader {
            id: extraFooterLoader
            anchors.bottom: parent.bottom

            active: model.isFilterModel() && index == count - 1 && !endOfFeed
            sourceComponent: extraFooterComponent
        }
    }

    onCovered: resetSystemBars()
    onUncovered: setSystemBars()

    onMovementEnded: {
        currentIndex = indexAt(0, contentY)
        console.debug("Move:", postFeedView.model.feedName, "index:", currentIndex, "count:", count)

        if (currentIndex >= 0 && count - currentIndex < 5) {
            console.debug("Prefetch next page:", postFeedView.model.feedName, "index:", currentIndex, "count:", count)
            model.getFeedNextPage(skywalker)
        }
    }

    FlickableRefresher {
        inProgress: model.feedType === QEnums.FEED_AUTHOR ? skywalker.getAuthorFeedInProgress : skywalker.getFeedInProgress
        verticalOvershoot: postFeedView.verticalOvershoot
        bottomOvershootFun: () => model.getFeedNextPage(skywalker)
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

    function orientationHandler() {
        headerHeight = guiSettings.getStatusBarSize(QEnums.INSETS_SIDE_TOP)
        footerHeight = guiSettings.getNavigationBarSize(QEnums.INSETS_SIDE_BOTTOM)
        leftMarginWidth = guiSettings.getNavigationBarSize(QEnums.INSETS_SIDE_LEFT)
        positionViewAtIndex(currentIndex, ListView.Beginning)
    }

    Component.onDestruction: {
        Screen.onOrientationChanged.disconnect(orientationHandler)
        resetSystemBars()

        if (model)
            model.clearOverrideLinkColor();
    }

    Component.onCompleted: {
        Screen.onOrientationChanged.connect(orientationHandler)
        setSystemBars()
        model.setOverrideLinkColor(guiSettings.linkColorDarkMode)
        positionViewAtIndex(currentIndex, ListView.Beginning)
    }
}
