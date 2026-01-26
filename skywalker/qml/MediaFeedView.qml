import QtQuick
import QtQuick.Controls
import skywalker

// This is a full screen swipe view
SkyListView {
    property string userDid
    property Skywalker skywalker: root.getSkywalker(userDid)
    property int startIndex: 0
    property int previewIndex: 0
    property var previewImage
    readonly property int closeTransition: StackView.Immediate
    property int headerHeight: guiSettings.getStatusBarSize(QEnums.INSETS_SIDE_TOP)
    property int footerHeight: guiSettings.getNavigationBarSize(QEnums.INSETS_SIDE_BOTTOM)
    property int leftMarginWidth: guiSettings.getNavigationBarSize(QEnums.INSETS_SIDE_LEFT)
    property int rightMarginWidth: guiSettings.getNavigationBarSize(QEnums.INSETS_SIDE_RIGHT)
    readonly property bool noSideBar: true
    readonly property var underlyingModel: model ? model.getUnderlyingModel() : null
    readonly property bool acceptsInteractions: underlyingModel ? underlyingModel.feedAcceptsInteractions : false
    readonly property string feedDid: underlyingModel ? underlyingModel.feedDid : ""
    readonly property string color: currentItem ? currentItem.color : guiSettings.fullScreenColor // for Main right margin (navbar)

    signal closed

    id: postFeedView
    width: parent.width
    boundsBehavior: Flickable.StopAtBounds
    snapMode: ListView.SnapOneItem
    spacing: 2 // to avoid the next video peeping at the bottom of the screen sometimes
    currentIndex: startIndex
    highlightMoveVelocity: 2000
    maximumFlickVelocity: 8000
    interactive: !currentItem.zooming // qmllint disable missing-property

    Accessible.name: postFeedView.model.feedName

    Loader {
        id: previewLoader
        active: Boolean(previewImage) && startIndex === currentIndex

        sourceComponent: Image {
            parent: Overlay.overlay
            x: previewImage.relX
            y: previewImage.relY
            width: previewImage.width
            height: previewImage.height
            fillMode: previewImage.fillMode
            source: previewImage.source
        }
    }

    delegate: MediaFeedViewDelegate {
        width: postFeedView.width
        startImageIndex: index == postFeedView.startIndex ? postFeedView.previewIndex : -1
        startImageWidth: (index == postFeedView.startIndex && postFeedView.previewImage) ? postFeedView.previewImage.width : -1
        footerHeight: postFeedView.footerHeight
        headerHeight: postFeedView.headerHeight
        leftMarginWidth: postFeedView.leftMarginWidth
        rightMarginWidth: postFeedView.rightMarginWidth
        extraFooterHeight: extraHeaderFooterLoader.active && !model.reverseFeed ? extraHeaderFooterLoader.height : 0
        extraHeaderHeight: extraHeaderFooterLoader.active && model.reverseFeed ? extraHeaderFooterLoader.height : 0
        feedAcceptsInteractions: postFeedView.acceptsInteractions
        feedDid: postFeedView.feedDid

        onClosed: postFeedView.closed()

        onImageLoaded: {
            if (index === postFeedView.startIndex)
                postFeedView.previewImage = null
        }

        Loader {
            id: extraHeaderFooterLoader
            y: model.reverseFeed ? 0 : parent.height - height
            active: model && model.isFilterModel() && isLastPost && !endOfFeed

            sourceComponent: FeedViewLoadMore {
                userDid: postFeedView.userDid
                listView: postFeedView
                textColor: "white"
                linkColor: guiSettings.linkColorDarkMode
            }
        }
    }

    onCovered: resetSystemBars()
    onUncovered: setSystemBars()

    onMovementEnded: {
        currentIndex = indexAt(0, contentY)
        console.debug("Move:", postFeedView.model.feedName, "index:", currentIndex, "count:", count)

        if (currentIndex < 0)
            return

        const remaining = model.reverseFeed ? currentIndex : count - currentIndex

        if (remaining < 5) {
            console.debug("Prefetch next page:", postFeedView.model.feedName, "index:", currentIndex, "count:", count)
            model.getFeedNextPage(skywalker)
        }
    }

    FlickableRefresher {
        inProgress: model.getFeedInProgress
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

    function cancel() {
        closed()
    }

    function setSystemBars() {
        displayUtils.setStatusBarTransparentAndMode(true, guiSettings.headerColor, false)
        displayUtils.setNavigationBarColorAndMode("transparent", false)
    }

    function resetSystemBars() {
        displayUtils.setStatusBarTransparent(false, guiSettings.headerColor)
        displayUtils.setNavigationBarColor(guiSettings.backgroundColor)
    }

    function orientationHandler() {
        headerHeight = guiSettings.getStatusBarSize(QEnums.INSETS_SIDE_TOP)
        footerHeight = guiSettings.getNavigationBarSize(QEnums.INSETS_SIDE_BOTTOM)
        leftMarginWidth = guiSettings.getNavigationBarSize(QEnums.INSETS_SIDE_LEFT)
        rightMarginWidth = guiSettings.getNavigationBarSize(QEnums.INSETS_SIDE_RIGHT)
    }

    Component.onDestruction: {
        Screen.onPrimaryOrientationChanged.disconnect(orientationHandler) // qmllint disable missing-property
        resetSystemBars()

        if (model)
            model.clearOverrideLinkColor();
    }

    Component.onCompleted: {
        Screen.onPrimaryOrientationChanged.connect(orientationHandler) // qmllint disable missing-property
        setSystemBars()
        model.setOverrideLinkColor(guiSettings.linkColorDarkMode)
        positionViewAtIndex(currentIndex, ListView.Beginning)
    }
}
