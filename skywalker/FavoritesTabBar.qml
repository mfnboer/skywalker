import QtQuick
import QtQuick.Controls
import skywalker

SkyTabBar {
    required property var skywalker

    id: tabBar

    AccessibleTabButton {
        text: qsTr("Following", "timeline title")

        onClicked: root.viewHomeFeed()
    }

    Repeater {
        model: skywalker.favoriteFeeds.pinnedFeeds

        FavoriteTabButton {
            required property favoritefeedview modelData

            width: implicitWidth
            favorite: modelData
            onClicked: root.showFavorite(modelData)
        }
    }

    function setCurrent(favorite) {
        skywalker.favoriteFeeds.pinnedFeeds.forEach((pinned, index) => {
            if (favorite.isSame(pinned))
                tabBar.currentIndex = index + 1
        })
    }
}
