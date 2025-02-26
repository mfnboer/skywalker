import QtQuick
import skywalker

SkyPage {
    required property var favoriteFeeds

    signal closed

    id: page
    width: parent.width
    height: parent.height

    header: SimpleHeader {
        text: qsTr("Sort favorites")
        onBack: page.closed()
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

            model: favoriteFeeds.pinnedFeeds

            delegate: Rectangle {
                required property var modelData

                property int visualIndex: DelegateModel.itemsIndex

                id: rect
                width: listView.width
                height: row.height + 10

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
                        unknownSvg: page.getDefaultAvatar(modelData)
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

                DropArea {
                    anchors.fill: parent

                    onEntered: (drag) => {
                        console.debug("ENTERED:", parent.modelData.name, "src:", drag.source.modelData.name)
                        drag.source.getDragHandler().start = Qt.point(parent.x, parent.y)
                        drag.source.getDragHandler().startZ = parent.z
                        visualModel.items.move(drag.source.visualIndex, parent.visualIndex)
                    }
                }

                function getDragHandler(item) {
                    return mouseArea
                }

                MouseArea {
                    property point start
                    property int startZ

                    id: mouseArea
                    anchors.fill: parent

                    drag.axis: Drag.YAxis

                    onPressAndHold: (mouse) => {
                        start = Qt.point(parent.x, parent.y)
                        startZ = parent.z
                        parent.z = z + listView.count
                        parent.color = "yellow"
                        drag.target = rect
                        mouse.accepted = false
                    }

                    onReleased: {
                        if (drag.target) {
                            parent.x = start.x
                            parent.y = start.y
                            parent.z = startZ
                            parent.color = guiSettings.backgroundColor
                            drag.target = undefined
                        }
                    }
                }
            }
        }
    }

    // TODO: duplicate from tab button
    function getDefaultAvatar(favorite) {
        switch (favorite.type) {
        case QEnums.FAVORITE_FEED:
            return guiSettings.feedDefaultAvatar(favorite.generatorView)
        case QEnums.FAVORITE_LIST:
            return SvgFilled.list
        case QEnums.FAVORITE_SEARCH:
            return favorite.searchFeed.isHashtag() ? SvgOutline.hashtag : SvgOutline.search
        }

        return SvgOutline.feed
    }
}
