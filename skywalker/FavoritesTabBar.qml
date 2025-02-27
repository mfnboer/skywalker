import QtQuick
import QtQuick.Controls
import skywalker

SkyTabBar {
    required property var favoriteFeeds
    property favoritefeedview homeFeed

    id: tabBar

    Repeater {
        model: favoriteFeeds.userOrderedPinnedFeeds.length + 1

        FavoriteTabButton {
            required property int index
            readonly property favoritefeedview favoriteFeed: index > 0 ? favoriteFeeds.userOrderedPinnedFeeds[index - 1] : homeFeed

            width: implicitWidth
            favorite: favoriteFeed

            onPressAndHold: showSorter()
        }
    }

    function setCurrent(favorite) {
        favoriteFeeds.userOrderedPinnedFeeds.forEach((pinned, index) => {
            if (favorite.isSame(pinned))
                tabBar.setCurrentIndex(index + 1)
        })
    }

    function showSorter() {
        let component = guiSettings.createComponent("FavoritesSorter.qml")
        let page = component.createObject(root, { favoriteFeeds: favoriteFeeds })
        page.onClosed.connect(() => { root.popStack() })
        root.pushStack(page)
    }
}
