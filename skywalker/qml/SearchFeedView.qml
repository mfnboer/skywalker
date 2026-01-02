import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker
import atproto.lib

// When you use moveToIndex(), you must set enough cacheBuffer space. Position a list
// to an index, may it to position a few items before the index the first time (size mismatch).
// Therefore positioning is called a few times. If there is enough buffer then the item to
// position to will be already created if we are close, and the next positioning call will be
// accurate. If not, then exisiting items will be destroyed. Positioning will be off again,
// and so on.
SkyListView {
    required property var skywalker
    required property searchfeed searchFeed
    property bool showAsHome: false
    property int unreadPosts: 0
    property var userSettings: skywalker.getUserSettings()
    readonly property int favoritesY: getFavoritesY()
    readonly property int extraFooterMargin: 0

    readonly property var underlyingModel: model ? model.getUnderlyingModel() : null
    property int initialContentMode: QEnums.CONTENT_MODE_UNSPECIFIED

    signal closed

    id: feedView
    width: parent.width
    model: searchUtils.getSearchPostFeedModel(SearchSortOrder.LATEST, searchFeed.name)
    cacheBuffer: Screen.height * 3
    virtualFooterHeight: userSettings.favoritesBarPosition === QEnums.FAVORITES_BAR_POSITION_BOTTOM ? guiSettings.tabBarHeight : 0

    Accessible.name: searchFeed.name

    header: PostFeedHeader {
        feedName: searchFeed.name
        defaultSvg: searchFeed.isHashtag() ? SvgOutline.hashtag : SvgOutline.search
        feedAvatar: ""
        contentMode: initialContentMode
        showAsHome: feedView.showAsHome
        showLanguageFilter: searchFeed.languageList.length > 0
        filteredLanguages: searchFeed.languageList
        showPostWithMissingLanguage: false
        showViewOptions: true
        showFavoritesPlaceHolder: userSettings.favoritesBarPosition === QEnums.FAVORITES_BAR_POSITION_TOP
        visible: !root.showSideBar

        onClosed: feedView.closed()
        onFeedAvatarClicked: showOptionsMenu()
        onViewChanged: (contentMode) => changeView(contentMode)
    }
    headerPositioning: ListView.PullBackHeader

    delegate: PostFeedViewDelegate {
        width: feedView.width
        swipeMode: [QEnums.CONTENT_MODE_VIDEO, QEnums.CONTENT_MODE_MEDIA].includes(model.contentMode)
        extraFooterHeight: extraFooterLoader.active ? extraFooterLoader.height : 0

        onActivateSwipe: (imgIndex, previewImg) => {
            let view = feedView
            root.viewMediaFeed(model, index, imgIndex, previewImg,
                               (newIndex, mediaIndex, closeCb) => {
                                   view.positionViewAtIndex(newIndex, ListView.Beginning)
                                   view.itemAtIndex(newIndex).closeMedia(mediaIndex, closeCb)
                               })
        }

        Loader {
            id: extraFooterLoader
            anchors.bottom: parent.bottom

            active: model && model.isFilterModel() && index === count - 1 && !endOfFeed
            sourceComponent: extraFooterComponent
        }
    }

    onCountChanged: {
        updateUnreadPosts()
    }

    onMovementEnded: updateOnMovement()
    onContentMoved: updateOnMovement()

    function updateOnMovement() {
        const lastVisibleIndex = getLastVisibleIndex()

        if (count - lastVisibleIndex < skywalker.TIMELINE_NEXT_PAGE_THRESHOLD && Boolean(model) && !model.getFeedInProgress) {
            console.debug("Get next feed page")
            model.getFeedNextPage(skywalker)
        }

        updateUnreadPosts()
    }

    FlickableRefresher {
        inProgress: feedView.model.getFeedInProgress
        topOvershootFun: () => feedView.search()
        bottomOvershootFun: () => feedView.getNextPage()
        topText: qsTr("Pull down to refresh feed")
        enableScrollToTop: !showAsHome
    }

    EmptyListIndication {
        y: parent.headerItem ? parent.headerItem.height : 0
        svg: SvgOutline.noPosts
        text: qsTr("Feed is empty")
        list: feedView
    }

    BusyIndicator {
        id: busyIndicator
        anchors.centerIn: parent
        running: feedView.model.getFeedInProgress
    }

    Component {
        id: extraFooterComponent

        Rectangle {
            width: feedView.width
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
        id: mediaTilesLoader
        active: false

        sourceComponent: MediaTilesFeedView {
            property int favoritesY: getFavoritesY()

            clip: true
            width: feedView.width
            height: feedView.height - (feedView.footerItem && feedView.footerItem.visible ? feedView.footerItem.height : 0)
            headerHeight: feedView.headerItem ? feedView.headerItem.height : 0
            skywalker: feedView.skywalker
            showAsHome: feedView.showAsHome
            model: feedView.model
            virtualFooterHeight: feedView.virtualFooterHeight

            // HACK: grid view does not have a pullback header
            Loader {
                id: headerLoader
                y: headerY
                width: parent.width
                sourceComponent: feedView.header
            }

            function getFavoritesY() {
                switch (userSettings.favoritesBarPosition) {
                case QEnums.FAVORITES_BAR_POSITION_TOP:
                    return headerLoader.item ? headerLoader.item.favoritesY + headerY : headerY
                case QEnums.FAVORITES_BAR_POSITION_BOTTOM:
                    return virtualFooterY
                }

                return 0
            }
        }
    }

    SearchUtils {
        id: searchUtils
        skywalker: feedView.skywalker

        Component.onDestruction: {
            // The destuctor of SearchUtils is called too late by the QML engine
            // Remove models now before the Skywalker object is destroyed.
            searchUtils.removeModels()
        }
    }

    SkyMenu {
        id: optionsMenu

        CloseMenuItem {
            text: qsTr("<b>Hashtag</b>")
            Accessible.name: qsTr("close hashtag options menu")
        }

        AccessibleMenuItem {
            text: qsTr("Search")
            svg: SvgOutline.search
            onTriggered: root.viewSearchViewFeed(searchFeed)
        }

        AccessibleMenuItem {
            text: qsTr("Remove favorite")
            svg: SvgFilled.star
            svgColor: guiSettings.favoriteColor
            onTriggered: {
                skywalker.favoriteFeeds.pinSearch(searchFeed, false)
                skywalker.saveFavoriteFeeds()
            }
        }

        AccessibleMenuItem {
            text: qsTr("Filtered posts")
            svg: SvgOutline.hideVisibility
            onTriggered: root.viewContentFilterStats(underlyingModel)
        }
    }

    function showOptionsMenu() {
        optionsMenu.open()
    }

    function getFavoritesY() {
        if (mediaTilesLoader.item)
            return mediaTilesLoader.item.favoritesY

        switch (userSettings.favoritesBarPosition) {
        case QEnums.FAVORITES_BAR_POSITION_TOP:
            return headerItem ? headerItem.favoritesY - (contentY - headerItem.y) : 0
        case QEnums.FAVORITES_BAR_POSITION_BOTTOM:
            return virtualFooterY
        }

        return 0
    }

    function changeView(contentMode) {
        let oldModel = model
        const lastVisibleIndex = mediaTilesLoader.item ? mediaTilesLoader.item.getTopRightVisibleIndex() : getLastVisibleIndex()
        const timestamp = model.getPostTimelineTimestamp(lastVisibleIndex)
        const cid = model.getPostCid(lastVisibleIndex)
        const lastVisibleOffsetY = mediaTilesLoader.item ? 0 : calcVisibleOffsetY(lastVisibleIndex)

        // When a tiles view is shown the header gets duplicated. Make sure the content values
        // between these headers is synced.
        headerItem.contentMode = contentMode
        initialContentMode = contentMode

        switch (contentMode) {
        case QEnums.CONTENT_MODE_UNSPECIFIED:
            setModel(model.getUnderlyingModel())
            break
        case QEnums.CONTENT_MODE_VIDEO:
        case QEnums.CONTENT_MODE_VIDEO_TILES:
            setModel(model.getUnderlyingModel().addVideoFilter())
            break
        case QEnums.CONTENT_MODE_MEDIA:
        case QEnums.CONTENT_MODE_MEDIA_TILES:
            setModel(model.getUnderlyingModel().addMediaFilter())
            break
        default:
            console.warn("Unknown content mode:", contentMode)
            return
        }

        if (oldModel.isFilterModel())
            oldModel.getUnderlyingModel().deleteFilteredPostFeedModel(oldModel)

        if (skywalker.favoriteFeeds.isPinnedSearch(searchFeed.name)) {
            userSettings.setSearchFeedViewMode(skywalker.getUserDid(), searchFeed.name, contentMode)
        }

        mediaTilesLoader.active = [QEnums.CONTENT_MODE_MEDIA_TILES, QEnums.CONTENT_MODE_VIDEO_TILES].includes(contentMode)

        if (lastVisibleIndex > -1) {
            const newIndex = model.findTimestamp(timestamp, cid)
            setInSync(newIndex, lastVisibleOffsetY)

            if (mediaTilesLoader.item) {
                mediaTilesLoader.item.goToIndex(newIndex)
            }
        }
    }

    function setModel(newModel) {
        // Resetting the model before changing is needed since Qt6.10.1
        // Without resetting, the code will crash
        if (model)
            model.reset()

        model = newModel
    }

    function updateUnreadPosts() {
        const firstIndex = getFirstVisibleIndex()
        feedView.unreadPosts = Math.max(firstIndex, 0)
    }

    function atStart() {
        return atYBeginning
    }

    function moveToHome() {
        positionViewAtBeginning()

        if (mediaTilesLoader.item)
            mediaTilesLoader.item.moveToHome()

        updateUnreadPosts()
    }

    function resetHeaderPosition() {
        if (mediaTilesLoader.item)
            mediaTilesLoader.item.resetHeaderPosition()
        else
            privateResetHeaderPosition()
    }

    function doMoveToPost(index) {
        const firstVisibleIndex = getFirstVisibleIndex()
        const lastVisibleIndex = getLastVisibleIndex()
        console.debug("Move to:", model.feedName, "index:", index, "first:", firstVisibleIndex, "last:", lastVisibleIndex, "count:", count)
        positionViewAtIndex(Math.max(index, 0), ListView.End)
        updateUnreadPosts()
        resetHeaderPosition()
        return (lastVisibleIndex >= index - 1 && lastVisibleIndex <= index + 1)
    }

    function finishSync() {
        updateUnreadPosts()
        resetHeaderPosition()
    }

    function setInSync(index, offsetY = 0) {
        console.debug("Sync:", model.feedName, "index:", index, "count:", count, "offsetY:", offsetY)

        if (index === 0 && offsetY === 0) {
            moveToHome()
            finishSync()
        }
        else if (index >= 0) {
            moveToIndex(index, doMoveToPost, () => { contentY -= offsetY; finishSync() })
        }
        else {
            positionViewAtEnd()
            finishSync()
        }
    }

    function syncToHome() {
        finishSync()
        moveToHome()
    }

    function search() {
        searchUtils.searchPosts(searchFeed.searchQuery, SearchSortOrder.LATEST,
                                searchFeed.authorHandle, searchFeed.mentionHandle,
                                searchFeed.since, !isNaN(searchFeed.since.getTime()),
                                searchFeed.until, !isNaN(searchFeed.until.getTime()),
                                searchFeed.language)
    }

    function getNextPage() {
        searchUtils.getNextPageSearchPosts(searchFeed.searchQuery, SearchSortOrder.LATEST,
                                searchFeed.authorHandle, searchFeed.mentionHandle,
                                searchFeed.since, !isNaN(searchFeed.since.getTime()),
                                searchFeed.until, !isNaN(searchFeed.until.getTime()),
                                searchFeed.language)
    }

    function forceDestroy() {
        searchUtils.clearAllSearchResults()
        searchUtils.removeModels()
        destroy()
    }

    Component.onCompleted: {
        let m = searchUtils.getSearchPostFeedModel(SearchSortOrder.LATEST, searchFeed.name)
        m.onFirstPage.connect(() => { search() })
        m.onNextPage.connect(() => { getNextPage() })

        const viewMode = userSettings.getSearchFeedViewMode(skywalker.getUserDid(), searchFeed.name)

        if (viewMode !== QEnums.CONTENT_MODE_UNSPECIFIED) {
            initialContentMode = viewMode
            changeView(viewMode)
        }

        search()
    }
}
