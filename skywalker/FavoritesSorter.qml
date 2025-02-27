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

        SvgButton {
            anchors.rightMargin: 10
            anchors.right: parent.right
            y: (guiSettings.headerHeight - height) / 2
            iconColor: guiSettings.headerTextColor
            Material.background: "transparent"
            svg: SvgOutline.moreVert
            accessibleName: qsTr("Sort favorites options")
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

            delegate: Rectangle {
                required property var modelData
                property int visualIndex: DelegateModel.itemsIndex

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
                        text: modelData.name
                    }
                }

                Drag.active: mouseArea.drag.active
                Drag.source: rect
                Drag.hotSpot.y: height / 2

                DropArea {
                    anchors.fill: parent

                    onEntered: (drag) => {
                        console.debug("Entered:", parent.modelData.name, "src:", drag.source.modelData.name)
                        drag.source.setDragStartingPoint(parent)
                        visualModel.items.move(drag.source.visualIndex, parent.visualIndex)
                    }
                }

                function setDragStartingPoint(item) {
                    mouseArea.start = Qt.point(item.x, item.y)
                    mouseArea.startZ = item.z
                }

                function restoreDragStartingPoint() {
                    x = mouseArea.start.x
                    y = mouseArea.start.y
                    z = mouseArea.startZ
                }

                MouseArea {
                    property point start
                    property int startZ

                    id: mouseArea
                    anchors.fill: parent

                    drag.axis: Drag.YAxis

                    onPressAndHold: (mouse) => {
                        parent.setDragStartingPoint(parent)
                        parent.z = z + listView.count
                        parent.color = guiSettings.dragHighLightColor
                        drag.target = rect
                        mouse.accepted = false
                    }

                    onReleased: {
                        if (drag.target) {
                            parent.restoreDragStartingPoint()
                            parent.color = guiSettings.backgroundColor
                            drag.target = undefined
                        }
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
