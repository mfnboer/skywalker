import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyPage {
    property Skywalker skywalker: root.getSkywalker()
    required property int modelId
    readonly property string sideBarTitle: qsTr("User Lists & Feeds")
    readonly property SvgImage sideBarSvg: SvgOutline.list

    id: page

    signal closed

    Accessible.role: Accessible.Pane

    header: SimpleHeader {
        text: sideBarTitle
        visible: !root.showSideBar
        onBack: page.closed()
    }

    SkyTabBar {
        id: listsBar
        width: parent.width

        AccessibleTabButton {
            text: qsTr("Your lists")
        }
        AccessibleTabButton {
            text: qsTr("Saved lists")
        }
        AccessibleTabButton {
            text: qsTr("Saved feeds")
        }
    }

    SwipeView {
        anchors.top: listsBar.bottom
        anchors.bottom: parent.bottom
        width: parent.width
        currentIndex: listsBar.currentIndex

        onCurrentIndexChanged: listsBar.setCurrentIndex(currentIndex)

        ListListView {
            id: yourLists
            modelId: page.modelId
            ownLists: true
            description: qsTr("Public, shareable lists of users which can be used as feeds or reply restrictions.")
        }

        SkyListView {
            id: savedListsView
            spacing: 0
            clip: true
            model: skywalker.favoriteFeeds.getSavedListsModel()
            boundsBehavior: Flickable.StopAtBounds

            header: Rectangle {
                width: parent.width
                height: headerText.height
                z: guiSettings.headerZLevel
                color: guiSettings.backgroundColor

                AccessibleText {
                    id: headerText
                    width: parent.width
                    padding: 10
                    wrapMode: Text.Wrap
                    elide: Text.ElideRight
                    text: qsTr("Saved lists from other users")
                }
            }
            headerPositioning: ListView.OverlayHeader

            delegate: ListViewDelegate {
                width: page.width

                onHideReplies: (list, hide) => graphUtils.hideReplies(list.uri, hide)
                onHideFollowing: (list, hide) => graphUtils.hideFollowing(list.uri, hide)
                onSyncList: (list, sync) => graphUtils.syncList(list.uri, sync)
            }

            FlickableRefresher {}

            EmptyListIndication {
                y: parent.headerItem ? parent.headerItem.height : 0
                svg: SvgOutline.noPosts
                text: qsTr("No saved lists")
                list: savedListsView
            }

            BusyIndicator {
                anchors.centerIn: parent
                running: skywalker.favoriteFeeds.updateSavedListsModelInProgress
            }
        }

        SkyListView {
            id: savedFeedsView
            spacing: 0
            clip: true
            model: skywalker.favoriteFeeds.getSavedFeedsModel()
            boundsBehavior: Flickable.StopAtBounds

            header: Rectangle {
                width: parent.width
                height: savedFeedsheaderText.height
                z: guiSettings.headerZLevel
                color: guiSettings.backgroundColor

                AccessibleText {
                    id: savedFeedsheaderText
                    width: parent.width
                    padding: 10
                    wrapMode: Text.Wrap
                    elide: Text.ElideRight
                    text: qsTr("You can find feeds on the search page")
                }
            }
            headerPositioning: ListView.OverlayHeader

            delegate: GeneratorViewDelegate {
                width: page.width

                onHideFollowing: (feed, hide) => feedUtils.hideFollowing(feed.uri, hide)
                onSyncFeed: (feed, sync) => feedUtils.syncFeed(feed.uri, sync)
            }

            FlickableRefresher {}

            EmptyListIndication {
                svg: SvgOutline.noPosts
                text: qsTr("No saved feeds")
                list: savedFeedsView
            }

            BusyIndicator {
                anchors.centerIn: parent
                running: skywalker.favoriteFeeds.updateSavedFeedsModelInProgress
            }
        }
    }

    GraphUtils {
        id: graphUtils
        skywalker: page.skywalker // qmllint disable missing-type
    }

    FeedUtils {
        id: feedUtils
        skywalker: page.skywalker // qmllint disable missing-type
    }

    Component.onDestruction: {
        skywalker.favoriteFeeds.removeSavedListsModel()
        skywalker.favoriteFeeds.removeSavedFeedsModel()
    }
}
