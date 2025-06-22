import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQml.Models
import skywalker

SvgButton {
    required property var skywalker
    property list<favoritefeedview> favorites: skywalker.favoriteFeeds.userOrderedPinnedFeeds

    iconColor: guiSettings.headerTextColor
    Material.background: "transparent"
    svg: SvgOutline.expandMore
    accessibleName: qsTr("select other feed")
    onClicked: feedsMenu.open()

    Accessible.role: Accessible.ButtonDropDown

    Menu {
        id: feedsMenu
        modal: true
        topPadding: guiSettings.headerMargin
        bottomPadding: guiSettings.footerMargin

        MenuItem {
            contentItem: Text {
                verticalAlignment: Text.AlignVCenter
                rightPadding: homeAvatar.width + 5
                color: guiSettings.textColor
                elide: Text.ElideRight
                text: qsTr("Following", "timeline title")
            }

            FeedAvatar {
                id: homeAvatar
                y: 5
                anchors.rightMargin: 10
                anchors.right: parent.right
                width: height
                height: parent.height - 10
                unknownSvg: SvgFilled.home
                onClicked: parent.triggered()
            }

            onTriggered: { highlighted = false; root.viewHomeFeed() }

            Accessible.role: Accessible.MenuItem
            Accessible.name: contentItem.text
            Accessible.description: Accessible.name
            Accessible.onPressAction: triggered()
        }

        Instantiator {
            id: menuInstantiator
            model: []
            delegate: MenuItem {
                required property favoritefeedview modelData

                contentItem: SkyCleanedTextLine {
                    verticalAlignment: Text.AlignVCenter
                    rightPadding: feedAvatar.width + 5
                    color: guiSettings.textColor
                    elide: Text.ElideRight
                    plainText: modelData.name

                    Accessible.ignored: true
                }

                FeedAvatar {
                    id: feedAvatar
                    y: 5
                    anchors.rightMargin: 10
                    anchors.right: parent.right
                    width: height
                    height: parent.height - 10
                    avatarUrl: modelData.avatarThumb
                    unknownSvg: getDefaultAvatar()
                    contentMode: modelData.contentMode
                    onClicked: parent.triggered()
                }

                onTriggered: {
                    highlighted = false;
                    root.showFavorite(modelData)
                }

                Accessible.role: Accessible.MenuItem
                Accessible.name: contentItem.text
                Accessible.description: Accessible.name
                Accessible.onPressAction: triggered()

                function getDefaultAvatar() {
                    switch (modelData.type) {
                    case QEnums.FAVORITE_FEED:
                        return guiSettings.feedDefaultAvatar(modelData.generatorView)
                    case QEnums.FAVORITE_LIST:
                        return SvgFilled.list
                    case QEnums.FAVORITE_SEARCH:
                        return modelData.searchFeed.isHashtag() ? SvgOutline.hashtag : SvgOutline.search
                    }

                    return SvgOutline.feed
                }
            }

            onObjectAdded: (index, object) => {
                console.debug("Add feed menu item:", object.text, "index:", index)
                feedsMenu.insertItem(index + 1, object)
            }
            onObjectRemoved: (index, object) => {
                console.debug("Remove feed menu item:", object.text, "index:", index)
                feedsMenu.removeItem(object)
            }
        }

        AccessibleMenuItem {
            contentItem: Text {
                verticalAlignment: Text.AlignVCenter
                rightPadding: settingsIcon.width + 5
                color: guiSettings.textColor
                elide: Text.ElideRight
                text: qsTr("Sort favorites")
            }

            SkySvg {
                id: settingsIcon
                y: height + 5
                anchors.rightMargin: 10
                anchors.right: parent.right
                width: height
                height: parent.height - 10
                color: guiSettings.buttonColor
                svg: SvgFilled.settings
            }

            visible: favorites.length > 1
            onTriggered: { highlighted = false; root.showFavoritesSorter() }

            Accessible.name: contentItem.text
        }

        onAboutToShow: {
            if (!compareFavorites(favorites))
                menuInstantiator.model = favorites
        }

        function compareFavorites(favorites) {
            if (favorites.length !== menuInstantiator.model.length)
                return false

            for (let i = 0; i < favorites.length; ++i) {
                const newFav = favorites[i]
                const oldFav = menuInstantiator.model[i]

                if (!newFav.isSame(oldFav))
                    return false
            }

            return true
        }
    }
}
