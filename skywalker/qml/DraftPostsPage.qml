import QtQuick
import QtQuick.Controls
import skywalker

SkyPage {
    property var localDraftsModel
    property var blueskyDraftsModel
    readonly property string sideBarTitle: qsTr("Drafts")
    readonly property SvgImage sideBarSvg: SvgOutline.chat

    signal closed
    signal localSelected(int index)
    signal localDeleted(int index)
    signal blueskySelected(int index)
    signal blueskyDeleted(int index)

    id: page

    header: SimpleHeader {
        text: sideBarTitle
        visible: !root.showSideBar
        onBack: page.closed()
    }

    SkyTabBar {
        id: tabBar
        width: parent.width
        clip: true

        AccessibleTabButton {
            text: qsTr("On device")
        }
        AccessibleTabButton {
            text: qsTr("Bluesky")
        }
    }

    SwipeView {
        id: swipeView
        anchors.top: tabBar.bottom
        anchors.bottom: parent.bottom
        width: parent.width
        currentIndex: tabBar.currentIndex

        onCurrentIndexChanged: tabBar.setCurrentIndex(currentIndex)

        DraftPostsView {
            model: localDraftsModel

            onSelected: localSelected
            onDeleted: localDeleted
        }

        DraftPostsView {
            model: blueskyDraftsModel
            boundsBehavior: Flickable.DragAndOvershootBounds

            onSelected: blueskySelected
            onDeleted: blueskyDeleted
        }
    }
}
