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
    readonly property string currentKey: (currentItem && currentItem.favoriteFeed) ? currentItem.favoriteFeed.key : ""

    id: tabBar

    Repeater {
        model: pinnedFeedsCount + 1

        FavoriteTabButton {
            required property int index
            readonly property favoritefeedview favoriteFeed: index > 0 ? favoriteFeeds.userOrderedPinnedFeeds[index - 1] : homeFeed
            readonly property var feedView: favoritesSwipeView && index >= 0 && index < favoritesSwipeView.count ? favoritesSwipeView.itemAt(index) : null

            width: implicitWidth
            favorite: favoriteFeed
            counter: (feedView && feedView.rewindEnabled) ? feedView.unreadPosts : 0

            onPressAndHold: {
                if (pinnedFeedsCount > 1)
                    root.showFavoritesSorter()
            }

            function getCounter() {
                if (!favoritesSwipeView)
                    return 0

                if (index < 0 || index >= favoritesSwipeView.count)
                    return 0

                const view = favoritesSwipeView.itemAt(index)
                return view.rewindEnabled ? view.unreadPosts : 0
            }
        }
    }

    SkySettingsTabButton {
        visible: pinnedFeedsCount > 1
        onActivated: root.showFavoritesSorter()
    }

    function setCurrent(favorite) {
        favoriteFeeds.userOrderedPinnedFeeds.forEach((pinned, index) => {
            if (favorite.isSame(pinned))
                tabBar.setCurrentIndex(index + 1)
        })
    }

    function setCurrentKey(key) {
        if (key === skywalker.favoriteFeeds.getHomeFeedKey()) {
            tabBar.setCurrentIndex(0)
            return
        }

        favoriteFeeds.userOrderedPinnedFeeds.forEach((pinned, index) => {
            if (key === pinned.key)
                tabBar.setCurrentIndex(index + 1)
        })
    }
}
