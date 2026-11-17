import QtQuick
import QtQuick.Controls
import skywalker

SkyMultiColumnView {
    required property Skywalker skywalker
    property bool trackLastViewedFeed: false
    property UserSettings userSettings: skywalker.getUserSettings()
    readonly property var currentView: getCurrentView()

    signal unreadPostsChanged(int index, int unread, var view)

    id: view
    spacing: 1
    maxColumns: !root.showSideBar ? 1 : 10
    interactive: userSettings.favoritesBarPosition !== QEnums.FAVORITES_BAR_POSITION_NONE

    StackView {
        readonly property int pageIndex: ObjectModel.index

        id: timelineStackView
        initialItem: TimelinePage {
            readonly property alias pageIndex: timelineStackView.pageIndex
            readonly property bool isVisiblePage: view.isVisibleIndex(pageIndex)
            readonly property bool isFirstVisiblePage: pageIndex === view.firstVisibleIndex
            property bool rewindEnabled: userSettings.getRewindToLastSeenPost(skywalker.getUserDid())

            id: timelinePage
            skywalker: view.skywalker
            feedColumnIndex: pageIndex

            onIsVisiblePageChanged: {
                if (isVisiblePage) {
                    console.debug("Current favorite: timeline")
                    unhide()

                    if (syncWarning) {
                        console.debug("Sync warning: home - ", syncWarning)
                        skywalker.showStatusMessage(syncWarning, QEnums.STATUS_LEVEL_INFO, 10)
                        clearSyncWarning()
                    }


                }
                else {
                    cover()
                    hide()
                }

                resetHeaderPosition()
            }

            onIsFirstVisiblePageChanged: {
                if (isFirstVisiblePage) {
                    if (trackLastViewedFeed)
                        skywalker.saveLastViewedFeed(skywalker.favoriteFeeds.getHomeFeedKey())
                }
            }

            onSyncWarningChanged: {
                if (!syncWarning) {
                    console.debug("Sync warning cleared: home")
                    return
                }

                console.debug("Sync warning: home - ", syncWarning)

                if (SwipeView.isCurrentItem) {
                    skywalker.showStatusMessage(syncWarning, QEnums.STATUS_LEVEL_INFO, 10)
                    Qt.callLater(clearSyncWarning)
                }
            }

            Connections {
                target: userSettings

                function onRewindToLastSeenPostChanged(did) {
                    if (did === skywalker.getUserDid())
                        timelinePage.rewindEnabled = userSettings.getRewindToLastSeenPost(did)
                }
            }
        }
    }

    Repeater {
        model: skywalker.favoriteFeeds.userOrderedPinnedFeedsInitialized ?
                   skywalker.favoriteFeeds.userOrderedPinnedFeeds : []

        StackView {
            required property var modelData
            readonly property int pageIndex: ObjectModel.index

            id: viewStack
            initialItem: Loader {
                property alias modelData: viewStack.modelData
                readonly property alias pageIndex: viewStack.pageIndex
                readonly property bool isVisiblePage: view.isVisibleIndex(pageIndex)
                readonly property bool isFirstVisiblePage: pageIndex === view.firstVisibleIndex
                property int unreadPosts: 0
                property bool rewindEnabled: mustSyncFeed(modelData)

                id: viewLoader
                sourceComponent: getComponent(modelData)
                active: mustSyncFeed(modelData) // activate at startup if sync is enabled

                onIsVisiblePageChanged: {
                    if (isVisiblePage) {
                        console.debug("Current favorite:", modelData.name)

                        if (active) {
                            if (item) {
                                item.uncover()

                                if (item.syncWarning) {
                                    skywalker.showStatusMessage(item.syncWarning, QEnums.STATUS_LEVEL_INFO, 10)
                                    item.clearSyncWarning()
                                }
                            }
                        }
                        else {
                            active = true
                        }

                        if (item) {
                            item.unhide()
                            item.forceActiveFocus()
                        }
                    }
                    else {
                        if (item) {
                            item.cover()
                            item.hide()
                        }
                    }

                    if (item)
                        item.resetHeaderPosition()
                }

                onIsFirstVisiblePageChanged: {
                    if (isFirstVisiblePage) {
                        if (trackLastViewedFeed)
                            saveLastViewedFeed(modelData)
                    }
                }

                function getHeaderHeight() {
                    if (item && typeof item.getHeaderHeight == 'function')
                        return item.getHeaderHeight()
                }

                Timer {
                    id: updateTimer
                    repeat: true
                    running: viewLoader.rewindEnabled && viewLoader.active
                    interval: 285000 + getRandomDelayMs()

                    onTriggered: {
                        console.debug("Update feed:", modelData.name)

                        if (viewLoader.item)
                            viewLoader.item.updateFeed()
                    }

                    function getRandomDelayMs() {
                        const d = Math.floor(Math.random() * 300)
                        return d * 100
                    }
                }

                Connections {
                    target: viewLoader.item
                    ignoreUnknownSignals: true

                    function onSyncWarningChanged() {
                        if (!viewLoader.item.syncWarning) {
                            console.debug("Sync warning cleared:", modelData.name)
                            return
                        }

                        console.debug("Sync warning:", modelData.name, " - ", viewLoader.item.syncWarning)

                        if (viewLoader.isVisiblePage) {
                            skywalker.showStatusMessage(viewLoader.item.syncWarning, QEnums.STATUS_LEVEL_INFO, 10)
                            Qt.callLater(() => viewLoader.item.clearSyncWarning())
                        }
                    }

                    function onUnreadPostsChanged() {
                        viewLoader.unreadPosts = viewLoader.item.unreadPosts
                    }
                }

                Connections {
                    target: userSettings

                    function onSyncFeedChanged(did, feedUri) {
                        if (did === skywalker.getUserDid() && feedUri === modelData.uri)
                            viewLoader.rewindEnabled = mustSyncFeed(modelData)
                    }

                    function onSyncSearchFeedChanged(did, searchKey) {
                        if (did === skywalker.getUserDid() && modelData.type === QEnums.FAVORITE_SEARCH &&
                                searchKey === modelData.searchFeed.key)
                            viewLoader.rewindEnabled = mustSyncFeed(modelData)
                    }
                }
            }
        }
    }

    Component {
        id: favoriteFeedComp

        PostFeedView {
            id: postFeedView
            skywalker: view.skywalker
            modelId: skywalker.createPostFeedModel(modelData.generatorView)
            feedColumnIndex: pageIndex
            showAsHome: true
            showFavorites: true
            footer: null

            function refreshFeed() {
                skywalker.syncFeed(modelId)
            }

            function updateFeed() {
                model.updateFeed(skywalker)
            }

            Component.onCompleted: refreshFeed()
        }
    }

    Component {
        id: favoriteListComp

        PostFeedView {
            id: postFeedView
            skywalker: view.skywalker
            modelId: skywalker.createPostFeedModel(modelData.listView)
            feedColumnIndex: pageIndex
            showAsHome: true
            showFavorites: true
            footer: null

            function refreshFeed() {
                skywalker.syncListFeed(modelId)
            }

            function updateFeed() {
                model.updateFeed(skywalker)
            }

            Component.onCompleted: refreshFeed()
        }
    }

    Component {
        id: favoriteSearchComp

        SearchFeedView {
            id: searchFeedView
            feedColumnIndex: pageIndex
            searchFeed: modelData.searchFeed
            showAsHome: true

            function refreshFeed() {
                syncSearch()
            }

            function updateFeed() {
                if (unreadPosts == 0)
                    syncSearch()
                else
                    console.debug("There are unread posts:", unreadPosts, searchFeed.name)
            }
        }
    }

    function mustSyncFeed(favorite) {
        switch (favorite.type) {
        case QEnums.FAVORITE_FEED:
        case QEnums.FAVORITE_LIST:
            return userSettings.mustSyncFeed(skywalker.getUserDid(), favorite.uri)
        case QEnums.FAVORITE_SEARCH:
            return userSettings.mustSyncSearchFeed(skywalker.getUserDid(), favorite.key)
        }

        console.warn("Unknown favorite type:", favorite.name)
        return false
    }

    function saveLastViewedFeed(favorite) {
        switch (favorite.type) {
        case QEnums.FAVORITE_FEED:
            return skywalker.saveLastViewedFeed(favorite.generatorView.uri)
        case QEnums.FAVORITE_LIST:
            return skywalker.saveLastViewedFeed(favorite.listView.uri)
        case QEnums.FAVORITE_SEARCH:
            return skywalker.saveLastViewedFeed(favorite.searchFeed.key)
        }

        console.warn("Unknown favorite type:", favorite.name)
    }

    function getComponent(favorite) {
        switch (favorite.type) {
        case QEnums.FAVORITE_FEED:
            return favoriteFeedComp
        case QEnums.FAVORITE_LIST:
            return favoriteListComp
        case QEnums.FAVORITE_SEARCH:
            return favoriteSearchComp
        }

        console.warn("Unknown favorite type:", favorite.name)
        return undefined
    }

    function viewAt(index) {
        return itemAt(index).get(0)
    }

    function getCurrentView() {
        if (currentItem && currentItem.depth > 1)
            return null

        if (currentIndex === 0)
            return timelinePage // currentItem.get(0) // TimelinePage

        if (currentItem)
            return currentItem.get(0).item

        return null
    }

    function popStack(index, operation = StackView.PopTransition) {
        let stack = itemAt(index)

        let item = stack.pop(operation)
        item.destroy()

        let topItem = stack.get(stack.depth - 1)

        if (topItem) {
            if (typeof topItem.uncover === 'function')
                topItem.uncover()

            topItem.forceActiveFocus()
        }
    }

    function pushStack(index, item, operation = StackView.PushTransition) {
        let stack = itemAt(index)
        let topItem = stack.get(stack.depth - 1)

        if (topItem && typeof topItem.cover === 'function')
            topItem.cover()

        stack.pushItem(item, {}, operation)
        item.forceActiveFocus()
    }

    function cover() {
        let view = getCurrentView()

        if (view)
            view.cover()
    }

    function reset() {
        trackLastViewedFeed = false

        for (let i = 1; i < count; ++i) {
            let loader = itemAt(i).get(0)
            loader.active = false
        }
    }

    function getHeaderHeight() {
        if (currentItem && typeof currentItem.getHeaderHeight == 'function')
            return currentItem.getHeaderHeight()

        return 0
    }

    function getFooterHeight() {
        return root.footer.visible ? root.footer.height : 0
    }

    Component.onCompleted: {
        contentItem.highlightMoveDuration = 200
    }
}
