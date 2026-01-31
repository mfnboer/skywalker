import QtQuick
import QtQuick.Controls
import skywalker

SwipeView {
    required property var skywalker
    property bool trackLastViewedFeed: false
    property var userSettings: skywalker.getUserSettings()
    readonly property var currentView: getCurrentView()

    id: view
    spacing: 1
    interactive: userSettings.favoritesBarPosition !== QEnums.FAVORITES_BAR_POSITION_NONE

    TimelinePage {
        skywalker: view.skywalker

        SwipeView.onIsCurrentItemChanged: {
            if (SwipeView.isCurrentItem) {

                if (trackLastViewedFeed)
                    skywalker.saveLastViewedFeed("home")
            }
            else {
                cover()
            }

            resetHeaderPosition()
        }
    }

    Repeater {
        model: skywalker.favoriteFeeds.userOrderedPinnedFeeds

        Loader {
            required property var modelData

            sourceComponent: getComponent(modelData)
            active: false

            SwipeView.onIsCurrentItemChanged: {
                if (SwipeView.isCurrentItem) {
                    console.debug("Current favorite:", modelData.name)

                    if (active) {
                        if (item) {
                            item.uncover()

                            if (item.atStart()) {
                                console.debug("Reload feed:", modelData.name)
                                item.refreshFeed()
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
        }
    }

    Component {
        id: favoriteFeedComp

        PostFeedView {
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

            Component.onCompleted: refreshFeed()
        }
    }

    Component {
        id: favoriteListComp

        PostFeedView {
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

            Component.onCompleted: refreshFeed()
        }
    }

    Component {
        id: favoriteSearchComp

        SearchFeedView {
            searchFeed: modelData.searchFeed
            showAsHome: true

            function saveAsLastViewedFeed() {
                skywalker.saveLastViewedFeed(modelData.searchFeed.name)
            }

            function refreshFeed() {
                search()
            }
        }
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
}
