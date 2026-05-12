import QtQuick
import QtQuick.Controls
import skywalker

SkyTabBar {
    required property var favoriteFeeds
    property var favoritesSwipeView // FavoritesSwipeView or NULL
    property favoritefeedview homeFeed
    readonly property int pinnedFeedsCount: favoriteFeeds.userOrderedPinnedFeeds.length
    property Skywalker skywalker: root.getSkywalker()
    property UserSettings userSettings: skywalker.getUserSettings()

    id: tabBar

    Repeater {
        model: pinnedFeedsCount + 1

        FavoriteTabButton {
            required property int index
            readonly property favoritefeedview favoriteFeed: index > 0 ? favoriteFeeds.userOrderedPinnedFeeds[index - 1] : homeFeed

            width: implicitWidth
            favorite: favoriteFeed
            counter: index === 0 && favoritesSwipeView ? favoritesSwipeView.itemAt(0).unreadPosts : 0

            onPressAndHold: {
                if (pinnedFeedsCount > 1)
                    root.showFavoritesSorter()
            }
        }
    }

    SkySettingsTabButton {
        visible: pinnedFeedsCount > 1
        onActivated: root.showFavoritesSorter()
    }

    Connections {
        target: favoritesSwipeView
        ignoreUnknownSignals: true

        function onUnreadPostsChanged(index, unread, view) {
            let showCounter = false

            if (view instanceof PostFeedView)
                showCounter = userSettings.mustSyncFeed(skywalker.getUserDid(), view.feedUri)
            else if (view instanceof SearchFeedView)
                showCounter = userSettings.mustSyncSearchFeed(skywalker.getUserDid(), view.searchFeed.searchQuery)

            if (showCounter)
                tabBar.itemAt(index + 1).counter = unread
        }
    }

    function setCurrent(favorite) {
        favoriteFeeds.userOrderedPinnedFeeds.forEach((pinned, index) => {
            if (favorite.isSame(pinned))
                tabBar.setCurrentIndex(index + 1)
        })
    }
}
