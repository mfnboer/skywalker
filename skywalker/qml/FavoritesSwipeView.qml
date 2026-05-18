import QtQuick
import QtQuick.Controls
import skywalker

SwipeView {
    required property Skywalker skywalker
    property bool trackLastViewedFeed: false
    property UserSettings userSettings: skywalker.getUserSettings()
    readonly property var currentView: getCurrentView()

    signal unreadPostsChanged(int index, int unread, var view)

    id: view
    spacing: 1
    interactive: userSettings.favoritesBarPosition !== QEnums.FAVORITES_BAR_POSITION_NONE

    TimelinePage {
        property bool rewindEnabled: userSettings.getRewindToLastSeenPost(skywalker.getUserDid())

        id: timelinePage
        skywalker: view.skywalker

        SwipeView.onIsCurrentItemChanged: {
            if (SwipeView.isCurrentItem) {
                if (syncWarning) {
                    console.debug("Sync warning: home - ", syncWarning)
                    skywalker.showStatusMessage(syncWarning, QEnums.STATUS_LEVEL_INFO, 10)
                    clearSyncWarning()
                }

                if (trackLastViewedFeed)
                    skywalker.saveLastViewedFeed("home")
            }
            else {
                cover()
            }

            resetHeaderPosition()
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

    Repeater {
        model: skywalker.favoriteFeeds.userOrderedPinnedFeedsInitialized ?
                   skywalker.favoriteFeeds.userOrderedPinnedFeeds : []

        Loader {
            required property int index
            required property var modelData
            property int unreadPosts: 0
            property bool rewindEnabled: mustSyncFeed(modelData)

            id: viewLoader
            sourceComponent: getComponent(modelData)
            active: mustSyncFeed(modelData) // activate at startup if sync is enabled

            SwipeView.onIsCurrentItemChanged: {
                if (SwipeView.isCurrentItem) {
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

                    if (item && trackLastViewedFeed)
                        item.saveAsLastViewedFeed()
                }
                else {
                    if (item)
                        item.cover()
                }

                if (item)
                    item.resetHeaderPosition()
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

                    if (viewLoader.SwipeView.isCurrentItem) {
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

                function onSyncSearchFeedChanged(did, searchQuery) {
                    if (did === skywalker.getUserDid() && modelData.type === QEnums.FAVORITE_SEARCH &&
                            searchQuery === modelData.searchFeed.searchQuery)
                        viewLoader.rewindEnabled = mustSyncFeed(modelData)
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
            showAsHome: true
            showFavorites: true
            footer: null

            function saveAsLastViewedFeed() {
                skywalker.saveLastViewedFeed(modelData.generatorView.uri)
            }

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
            showAsHome: true
            showFavorites: true
            footer: null

            function saveAsLastViewedFeed() {
                skywalker.saveLastViewedFeed(modelData.listView.uri)
            }

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
            searchFeed: modelData.searchFeed
            showAsHome: true

            function saveAsLastViewedFeed() {
                skywalker.saveLastViewedFeed(modelData.searchFeed.name)
            }

            function refreshFeed() {
                syncSearch()
            }

            function updateFeed() {
                if (unreadPosts == 0)
                    syncSearch()
                else
                    console.debug("There are unread posts:", unreadPosts, modelData.searchFeed.name)
            }
        }
    }

    function mustSyncFeed(favorite) {
        switch (favorite.type) {
        case QEnums.FAVORITE_FEED:
        case QEnums.FAVORITE_LIST:
            return userSettings.mustSyncFeed(skywalker.getUserDid(), favorite.uri)
        case QEnums.FAVORITE_SEARCH:
            return userSettings.mustSyncSearchFeed(skywalker.getUserDid(), favorite.searchFeed.searchQuery)
        }

        console.warn("Unknown favorite type:", favorite.name)
        return false
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

    function getCurrentView() {
        if (currentIndex === 0)
            return currentItem // TimelinePage

        if (currentItem)
            return currentItem.item

        return null
    }

    function cover() {
        let view = getCurrentView()

        if (view)
            view.cover()
    }

    function reset() {
        trackLastViewedFeed = false

        for (let i = 1; i < count; ++i) {
            let loader = itemAt(i)
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
