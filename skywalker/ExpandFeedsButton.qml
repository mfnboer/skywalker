import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SvgButton {
    required property var skywalker

    iconColor: guiSettings.headerTextColor
    Material.background: "transparent"
    svg: svgOutline.expandMore
    accessibleName: qsTr("select other feed")
    onClicked: feedsMenu.open()

    Accessible.role: Accessible.ButtonDropDown

    Menu {
        id: feedsMenu
        modal: true

        MenuItem {
            contentItem: Text {
                verticalAlignment: Text.AlignVCenter
                rightPadding: homeAvatar.width + 5
                color: guiSettings.textColor
                elide: Text.ElideRight
                text: qsTr("Home feed", "timeline title")
            }

            FeedAvatar {
                id: homeAvatar
                y: 5
                anchors.rightMargin: 10
                anchors.right: parent.right
                width: height
                height: parent.height - 10
                unknownSvg: svgFilled.home
                onClicked: parent.triggered()
            }

            onTriggered: root.viewTimeline()

            Accessible.role: Accessible.MenuItem
            Accessible.name: contentItem.text
            Accessible.description: Accessible.name
            Accessible.onPressAction: triggered()
        }

        Instantiator {
            id: menuInstantiator
            model: []
            delegate: MenuItem {
                contentItem: SkyCleanedText {
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
                    avatarUrl: modelData.avatar
                    unknownSvg: modelData.isGeneratorView ? svgFilled.feed : svgFilled.list
                    onClicked: parent.triggered()
                }

                onTriggered: viewFeed()

                Accessible.role: Accessible.MenuItem
                Accessible.name: contentItem.text
                Accessible.description: Accessible.name
                Accessible.onPressAction: triggered()

                function viewFeed() {
                    if (modelData.isGeneratorView)
                        root.viewFeed(modelData.generatorView)
                    else
                        root.viewListFeed(modelData.listView)
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

        onAboutToShow: {
            let favorites = skywalker.favoriteFeeds.getPinnedFeeds()

            if (!compareFavorites(favorites))
                menuInstantiator.model = favorites
        }

        function compareFavorites(favorites) {
            if (favorites.length !== menuInstantiator.model.length)
                return false

            for (let i = 0; i < favorites.length; ++i) {
                const newFav = favorites[i]
                const oldFav = menuInstantiator.model[i]

                if (newFav.uri !== oldFav.uri ||
                        newFav.name !== oldFav.name ||
                        newFav.avatar !== oldFav.avatar)
                {
                    return false
                }
            }

            return true
        }
    }

    GuiSettings {
        id: guiSettings
    }
}
