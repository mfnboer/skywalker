import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import skywalker

SkyPage {
    required property var favoriteFeeds

    signal closed

    id: page
    width: parent.width
    height: parent.height

    header: SimpleDescriptionHeader {
        title: qsTr("Sort favorites")
        description: qsTr("To change the order, keep a favorite pushed till its background changes color, then drag it to the desired position.")
        onClosed: page.closed()

        SvgPlainButton {
            anchors.rightMargin: 10
            anchors.right: parent.right
            y: (guiSettings.headerHeight - height) / 2
            svg: SvgOutline.moreVert
            accessibleName: qsTr("Favorites options")
            onClicked: moreMenu.open()

            Menu {
                id: moreMenu
                modal: true

                CloseMenuItem {
                    text: qsTr("<b>Favorites</b>")
                    Accessible.name: qsTr("close more options menu")
                }

                AccessibleMenuItem {
                    text: qsTr("Sort alphabetically")
                    onTriggered: alphaSort()
                    MenuItemSvg { svg: SvgOutline.sortByAlpha }
                }
            }
        }
    }

    SkyListView {
        id: listView
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        spacing: 0
        boundsBehavior: Flickable.StopAtBounds

        model: DelegateModel {
            id: visualModel

            model: favoriteFeeds.userOrderedPinnedFeeds

            delegate: DragDropRectangle {
                id: rect
                width: listView.width
                height: row.height + 10
                color: guiSettings.backgroundColor

                Row {
                    id: row

                    anchors.centerIn: parent
                    width: parent.width - 20
                    spacing: 5

                    FeedAvatar {
                        id: avatar
                        anchors.verticalCenter: parent.verticalCenter
                        width: visible ? parent.height : 0
                        avatarUrl: modelData.avatarThumb
                        unknownSvg: guiSettings.favoriteDefaultAvatar(modelData)
                        contentMode: modelData.contentMode
                    }

                    AccessibleText {
                        anchors.verticalCenter: parent.verticalCenter
                        color: guiSettings.textColor
                        elide: Text.ElideRight
                        text: modelData.name
                    }
                }
            }
        }
    }

    function alphaSort() {
        favoriteFeeds.clearUserOrderedPinnedFeed()

        // Re-assign model to reset any dragging changes
        visualModel.model = favoriteFeeds.userOrderedPinnedFeeds
    }

    function setUserOrderedFavorites() {
        let favorites = []

        for (let i = 0; i < visualModel.items.count; ++i) {
            const item = visualModel.items.get(i)
            favorites.push(item.model.modelData)
        }

        favoriteFeeds.userOrderedPinnedFeeds = favorites
    }

    Component.onDestruction: {
        setUserOrderedFavorites()
    }
}
