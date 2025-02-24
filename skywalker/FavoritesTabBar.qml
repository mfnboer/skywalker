import QtQuick
import QtQuick.Controls
import skywalker

SkyTabBar {
    required property var favoriteFeeds
    property favoritefeedview homeFeed

    id: tabBar

    Repeater {
        model: favoriteFeeds.pinnedFeeds.length + 1

        FavoriteTabButton {
            required property int index
            readonly property favoritefeedview favoriteFeed: index > 0 ? favoriteFeeds.pinnedFeeds[index - 1] : homeFeed

            width: implicitWidth
            favorite: favoriteFeed
            // onClicked: {
            //     if (index === 0)
            //         root.viewHomeFeed()
            //     else
            //         root.showFavorite(favoriteFeed)
            // }
        }
    }

    function setCurrent(favorite) {
        favoriteFeeds.pinnedFeeds.forEach((pinned, index) => {
            if (favorite.isSame(pinned))
                tabBar.setCurrentIndex(index + 1)
        })
    }
}
