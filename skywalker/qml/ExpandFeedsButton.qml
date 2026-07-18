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
    onClicked: feedsMenuLoader.open()

    Accessible.role: Accessible.ButtonDropDown

    Loader {
        id: feedsMenuLoader
        active: false

        function open() {
            active = true
        }

        onStatusChanged: {
            if (status == Loader.Ready)
                item.open() // qmllint disable missing-property
        }

        sourceComponent: SkyMenu {
            id: feedsMenu
            menuWidth: 250
            currentIndex: 0

            MenuItem {
                contentItem: Row {
                    AccessibleText {
                        id: homeText
                        width: Math.min(parent.width - homeBadge.width - homeAvatar.width - 5, implicitWidth)
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                        text: qsTr("Following", "timeline title")
                    }

                    BadgeCounter {
                        id: homeBadge
                        counter: root.getFavoritesTabBar().itemAt(0).counter
                    }
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
                Accessible.name: homeText.text
                Accessible.description: Accessible.name
                Accessible.onPressAction: triggered()
            }

            Instantiator {
                id: menuInstantiator
                model: []
                delegate: MenuItem {
                    required property int index
                    required property favoritefeedview modelData

                    contentItem: Row {
                        Column {
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.verticalCenterOffset: modelData.subTitle ? -subTitleText.implicitHeight / 2 : 0
                            width: parent.width - badge.width - feedAvatar.width - 5

                            height: nameText.implicitHeight

                            AccessibleText {
                                id: nameText
                                width: parent.width
                                elide: Text.ElideRight
                                text: modelData.name
                            }
                            Text {
                                id: subTitleText
                                width: parent.width
                                elide: Text.ElideRight
                                font.pointSize: guiSettings.scaledFont(5/8)
                                text: modelData.subTitle
                                visible: modelData.subTitle
                            }
                        }

                        BadgeCounter {
                            id: badge
                            counter: root.getFavoritesTabBar().itemAt(index + 1).counter
                        }
                    }

                    FeedAvatar {
                        id: feedAvatar
                        y: 5
                        anchors.rightMargin: 10
                        anchors.right: parent.right
                        width: height
                        height: parent.height - 10
                        avatarUrl: modelData.avatarThumb
                        unknownSvg: guiSettings.favoriteDefaultAvatar(modelData)
                        contentMode: modelData.contentMode
                        onClicked: parent.triggered()
                    }

                    onTriggered: {
                        highlighted = false;
                        root.showFavorite(modelData)
                    }

                    Accessible.role: Accessible.MenuItem
                    Accessible.name: modelData.name
                    Accessible.description: Accessible.name
                    Accessible.onPressAction: triggered()
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

            MenuItem {
                contentItem: AccessibleText {
                    verticalAlignment: Text.AlignVCenter
                    rightPadding: settingsIcon.width + 5
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

            onClosed: {
                feedsMenuLoader.active = false
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
}
