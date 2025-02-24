import QtQuick
import QtQuick.Controls
import skywalker

SkyTabBar {
    required property var skywalker

    id: tabBar

    Repeater {
        model: skywalker.favoriteFeeds.pinnedFeeds.length + 1

        FavoriteTabButton {
            required property int index
            property favoritefeedview homeFeed
            readonly property favoritefeedview favoriteFeed: index > 0 ? skywalker.favoriteFeeds.pinnedFeeds[index - 1] : homeFeed

            width: implicitWidth
            favorite: favoriteFeed
            onClicked: {
                if (index === 0)
                    root.viewHomeFeed()
                else
                    root.showFavorite(favoriteFeed)
            }
        }
    }

    function setCurrent(favorite) {
        skywalker.favoriteFeeds.pinnedFeeds.forEach((pinned, index) => {
            if (favorite.isSame(pinned))
                tabBar.setCurrentIndex(index + 1)
        })
    }
}
