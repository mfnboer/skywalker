import QtQuick
import QtQuick.Controls
import skywalker

SkyTabBar {
    required property var favoriteFeeds
    property favoritefeedview homeFeed
    readonly property int pinnedFeedsCount: favoriteFeeds.userOrderedPinnedFeeds.length

    id: tabBar

    Repeater {
        model: pinnedFeedsCount + 1

        FavoriteTabButton {
            required property int index
            readonly property favoritefeedview favoriteFeed: index > 0 ? favoriteFeeds.userOrderedPinnedFeeds[index - 1] : homeFeed

            width: implicitWidth
            favorite: favoriteFeed

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

    function setCurrent(favorite) {
        favoriteFeeds.userOrderedPinnedFeeds.forEach((pinned, index) => {
            if (favorite.isSame(pinned))
                tabBar.setCurrentIndex(index + 1)
        })
    }
}
