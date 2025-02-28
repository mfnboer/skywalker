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
            readonly property bool settingsTab: false

            width: implicitWidth
            favorite: favoriteFeed

            onPressAndHold: root.showFavoritesSorter()
        }
    }

    AccessibleTabButton {
        readonly property bool settingsTab: true

        implicitWidth: contentItem.width
        Accessible.name: qsTr("press to change tab order")

        contentItem: Rectangle {
            height: parent.height
            width: parent.height + 4
            color: "transparent"

            SkySvg {
                x: 2
                width: parent.height
                height: parent.height
                color: guiSettings.buttonColor
                svg: SvgFilled.settings
            }

            MouseArea {
                anchors.fill: parent
                onClicked: root.showFavoritesSorter()
            }
        }

        onClicked: root.showFavoritesSorter()
    }

    function setCurrent(favorite) {
        favoriteFeeds.userOrderedPinnedFeeds.forEach((pinned, index) => {
            if (favorite.isSame(pinned))
                tabBar.setCurrentIndex(index + 1)
        })
    }
}
