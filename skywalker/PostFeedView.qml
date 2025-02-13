import QtQuick
import QtQuick.Controls
import skywalker

SkyListView {
    required property var skywalker
    required property int modelId
    property bool showAsHome: false
    property int unreadPosts: 0
    property int calibrationDy: 0
    property bool inSync: true
    readonly property var underlyingModel: model ? model.getUnderlyingModel() : null
    property int initialContentMode: underlyingModel ? underlyingModel.contentMode : QEnums.CONTENT_MODE_UNSPECIFIED

    signal closed

    id: postFeedView
    width: parent.width
    model: skywalker.getPostFeedModel(modelId)

    Accessible.name: underlyingModel ? underlyingModel.feedName : ""

    header: PostFeedHeader {
        skywalker: postFeedView.skywalker
        feedName: underlyingModel ? underlyingModel.feedName : ""
        feedAvatar: getFeedAvatar()
        defaultSvg: getFeedDefaultAvatar()
        contentMode: initialContentMode
        showAsHome: postFeedView.showAsHome
        showLanguageFilter: underlyingModel ? underlyingModel.languageFilterConfigured : false
        filteredLanguages: underlyingModel ? underlyingModel.filteredLanguages : []
        showPostWithMissingLanguage: underlyingModel ? underlyingModel.showPostWithMissingLanguage :true
        showViewOptions: underlyingModel ? underlyingModel.contentMode === QEnums.CONTENT_MODE_UNSPECIFIED : false

        onClosed: postFeedView.closed()
        onFeedAvatarClicked: showFeed()
        onViewChanged: (contentMode) => changeView(contentMode)
    }
    headerPositioning: ListView.OverlayHeader

    footer: SkyFooter {
        visible: showAsHome
        timeline: postFeedView
        skywalker: postFeedView.skywalker
        homeActive: true
        showHomeFeedBadge: true
        onHomeClicked: moveToHome()
        onNotificationsClicked: root.viewNotifications()
        onSearchClicked: root.viewSearchView()
        onFeedsClicked: root.viewFeedsView()
        onMessagesClicked: root.viewChat()
    }
    footerPositioning: ListView.OverlayFooter

    delegate: PostFeedViewDelegate {
        required property int index

        width: postFeedView.width
        swipeMode: [QEnums.CONTENT_MODE_VIDEO, QEnums.CONTENT_MODE_MEDIA].includes(model.contentMode)
        extraFooterHeight: extraFooterLoader.active ? extraFooterLoader.height : 0

        onCalibratedPosition: (dy) => {
            calibrationDy += dy
            Qt.callLater(calibratePosition)
        }

        onActivateSwipe: {
            root.viewMediaFeed(model, index, (newIndex) => { postFeedView.positionViewAtIndex(newIndex, ListView.Beginning) })
        }

        Loader {
            id: extraFooterLoader
            anchors.bottom: parent.bottom

            active: model.isFilterModel() && index == count - 1 && !endOfFeed
            sourceComponent: extraFooterComponent
        }
    }

    onMovementEnded: {
        if (!inSync)
            return

        const firstVisibleIndex = getFirstVisibleIndex()
        const lastVisibleIndex = getLastVisibleIndex()

        if (lastVisibleIndex !== -1 && modelId !== -1) {
            const lastVisibleOffsetY = calcVisibleOffsetY(lastVisibleIndex)
            skywalker.feedMovementEnded(modelId, lastVisibleIndex, lastVisibleOffsetY)
        }

        setAnchorItem(firstVisibleIndex, lastVisibleIndex)
    }

    FlickableRefresher {
        inProgress: skywalker.getFeedInProgress
        verticalOvershoot: postFeedView.verticalOvershoot
        topOvershootFun: () => model.getFeed(skywalker)
        bottomOvershootFun: () => model.getFeedNextPage(skywalker)
        topText: qsTr("Pull down to refresh feed")
        enableScrollToTop: !showAsHome
    }

    EmptyListIndication {
        id: emptyListIndication
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

    Loader {
        id: mediaTilesLoader
        active: false

        sourceComponent: MediaTilesFeedView {
            clip: true
            y: postFeedView.headerItem ? postFeedView.headerItem.height : 0
            width: postFeedView.width
            height: postFeedView.height - (postFeedView.headerItem ? postFeedView.headerItem.height : 0) - (postFeedView.footerItem && postFeedView.footerItem.isVisible ? postFeedView.footerItem.height : 0)
            skywalker: postFeedView.skywalker
            showAsHome: postFeedView.showAsHome
            model: postFeedView.model
        }
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
                text: qsTr(`${guiSettings.getFilteredPostsFooterText(model)}<br><a href="load" style="color: ${guiSettings.linkColor}; text-decoration: none">Load more</a>`)
                onLinkActivated: model.getFeedNextPage(skywalker)
            }
        }
    }

    Loader {
        anchors.top: emptyListIndication.bottom
        active: Boolean(model) && model.isFilterModel() && count === 0 && !model.endOfFeed
        sourceComponent: extraFooterComponent
    }

    Rectangle {
        y: headerItem ? headerItem.height : 0
        width: parent.width
        height: parent.height - (headerItem ? headerItem.height : 0) - (footerItem && footerItem.visible ? footerItem.height : 0)
        color: guiSettings.backgroundColor
        visible: !inSync && (rewindStatus.rewindPagesLoaded > 0 || rewindStatus.isFirstRewind)

        Column {
            width: parent.width - 20
            anchors.centerIn: parent

            AccessibleText {
                anchors.horizontalCenter: parent.horizontalCenter
                font.pointSize: guiSettings.scaledFont(2)
                text: qsTr("Rewinding feed")
            }

            RewindStatus {
                property bool isFirstRewind: true

                id: rewindStatus
                width: parent.width
            }
        }
    }

    function getFeedDefaultAvatar() {
        if (!underlyingModel)
            return SvgFilled.feed

        switch (underlyingModel.feedType) {
        case QEnums.FEED_GENERATOR:
            return guiSettings.feedDefaultAvatar(underlyingModel.getGeneratorView())
        case QEnums.FEED_LIST:
            return SvgFilled.list
        default:
            console.warn("Unexpected feed type:", underlyingModel.feedType)
            return SvgFilled.feed
        }
    }

    function getFeedAvatar() {
        if (!underlyingModel)
            return ""

        switch (underlyingModel.feedType) {
        case QEnums.FEED_GENERATOR:
            return guiSettings.feedContentVisible(underlyingModel.getGeneratorView()) ?
                underlyingModel.getGeneratorView().avatarThumb : ""
        case QEnums.FEED_LIST:
            return guiSettings.feedContentVisible(underlyingModel.getListView()) ?
                underlyingModel.getListView().avatarThumb : ""
        default:
            console.warn("Unexpected feed type:", underlyingModel.feedType)
            return ""
        }
    }

    function showFeed() {
        if (!underlyingModel)
            return

        switch (underlyingModel.feedType) {
        case QEnums.FEED_GENERATOR:
            skywalker.getFeedGenerator(underlyingModel.getGeneratorView().uri)
            break
        case QEnums.FEED_LIST:
            root.viewListByUri(underlyingModel.getListView().uri, false)
            break
        default:
            console.warn("Unexpected feed type:", underlyingModel.feedType)
            break
        }
    }

    function changeView(contentMode) {
        let oldModel = model
        const lastVisibleIndex = mediaTilesLoader.item ? mediaTilesLoader.item.getTopRightVisibleIndex() : getLastVisibleIndex()
        const timestamp = model.getPostTimelineTimestamp(lastVisibleIndex)
        const cid = model.getPostCid(lastVisibleIndex)
        const lastVisibleOffsetY = mediaTilesLoader.item ? 0 : calcVisibleOffsetY(lastVisibleIndex)

        switch (contentMode) {
        case QEnums.CONTENT_MODE_UNSPECIFIED:
            model = model.getUnderlyingModel()
            break
        case QEnums.CONTENT_MODE_VIDEO:
            model = model.getUnderlyingModel().addVideoFilter()
            break
        case QEnums.CONTENT_MODE_MEDIA:
        case QEnums.CONTENT_MODE_MEDIA_TILES:
            model = model.getUnderlyingModel().addMediaFilter()
            break
        default:
            console.warn("Unknown content mode:", contentMode)
            return
        }

        if (oldModel.isFilterModel())
            oldModel.getUnderlyingModel().deleteFilteredPostFeedModel(oldModel)

        if (skywalker.favoriteFeeds.isPinnedFeed(underlyingModel.feedUri)) {
            const userSettings = skywalker.getUserSettings()
            userSettings.setFeedViewMode(skywalker.getUserDid(), underlyingModel.feedUri, contentMode)
        }

        mediaTilesLoader.active = (contentMode === QEnums.CONTENT_MODE_MEDIA_TILES)

        if (lastVisibleIndex > -1) {
            const newIndex = model.findTimestamp(timestamp, cid)
            setInSync(modelId, newIndex, lastVisibleOffsetY)

            if (mediaTilesLoader.item)
                mediaTilesLoader.item.positionViewAtIndex(newIndex, GridView.Beginning)
        }
    }

    function calibratePosition() {
        if (calibrationDy === 0)
            return

        console.debug("Calibration, calibrationDy:", calibrationDy)
        contentY += calibrationDy
        calibrationDy = 0
    }

    function moveToHome() {
        positionViewAtBeginning()
        setAnchorItem(0, 0)

        if (mediaTilesLoader.item)
            mediaTilesLoader.item.positionViewAtBeginning()

        if (modelId != -1)
            skywalker.feedMovementEnded(modelId, 0, 0)
    }

    function doMoveToPost(index) {
        const firstVisibleIndex = getFirstVisibleIndex()
        const lastVisibleIndex = getLastVisibleIndex()
        console.debug("Move to:", model.feedName, "index:", index, "first:", firstVisibleIndex, "last:", lastVisibleIndex, "count:", count)
        positionViewAtIndex(Math.max(index, 0), ListView.End)
        setAnchorItem(firstVisibleIndex, lastVisibleIndex)
        return (lastVisibleIndex >= index - 1 && lastVisibleIndex <= index + 1)
    }

    function finishSync() {
        inSync = true
        rewindStatus.isFirstRewind = false
    }

    function setInSync(id, index, offsetY = 0) {
        if (id !== modelId)
            return

        console.debug("Sync:", model.feedName, "index:", index, "count:", count, "offsetY:", offsetY)

        if (index === 0 && offsetY === 0) {
            moveToHome()
            finishSync()
        }
        else if (index >= 0) {
            moveToIndex(index, doMoveToPost, () => { contentY -= offsetY; finishSync() })
        }
        else {
            moveToEnd()
            finishSync()
        }
    }

    function syncToHome(id) {
        if (id !== modelId)
            return

        finishSync()
        moveToHome()
    }

    function handleSyncStart(id, maxPages, timestamp) {
        if (id !== modelId)
            return

        console.debug("Sync start:", model.feedName, "maxPages:", maxPages, "timestamp:", timestamp)
        rewindStatus.startRewind(maxPages, timestamp)
        inSync = false
    }

    function handleSyncProgress(id, pages, timestamp) {
        if (id !== modelId)
            return

        console.debug("Sync proress:", model.feedName, "pages:", pages, "timestamp:", timestamp)
        rewindStatus.updateRewindProgress(pages, timestamp)
    }

    function activate() {
        for (var i = 0; i < count; ++i) {
            const item = itemAtIndex(i)

            if (item)
                item.activate() // qmllint disable missing-property
        }
    }

    function deactivate() {
        for (var i = 0; i < count; ++i) {
            const item = itemAtIndex(i)

            if (item)
                item.deactivate() // qmllint disable missing-property
        }
    }

    function forceDestroy() {
        if (modelId !== -1) {
            postFeedView.model = null
            skywalker.removePostFeedModel(modelId)
            modelId = -1
            destroy()
        }
    }

    Component.onDestruction: {
        skywalker.onFeedSyncStart.disconnect(handleSyncStart)
        skywalker.onFeedSyncProgress.disconnect(handleSyncProgress)
        skywalker.onFeedSyncOk.disconnect(setInSync)
        skywalker.onFeedSyncFailed.disconnect(syncToHome)

        if (modelId !== -1)
            skywalker.removePostFeedModel(modelId)
    }

    Component.onCompleted: {
        skywalker.onFeedSyncStart.connect(handleSyncStart)
        skywalker.onFeedSyncProgress.connect(handleSyncProgress)
        skywalker.onFeedSyncOk.connect(setInSync)
        skywalker.onFeedSyncFailed.connect(syncToHome)

        const userSettings = skywalker.getUserSettings()
        const viewMode = userSettings.getFeedViewMode(skywalker.getUserDid(), model.feedUri)

        if (viewMode !== QEnums.CONTENT_MODE_UNSPECIFIED) {
            initialContentMode = viewMode
            changeView(viewMode)
        }
    }
}
