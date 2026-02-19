import QtQuick
import QtQuick.Controls
import skywalker

SkyPage {
    required property DraftPosts localDraftPosts
    required property DraftPosts blueskyDraftPosts
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
        clip: true

        onCurrentIndexChanged: tabBar.setCurrentIndex(currentIndex)

        DraftPostsView {
            id: localDraftsView
            draftPosts: localDraftPosts

            onSelected: (index) => localSelected(index)
            onDeleted: (index) => localDeleted(index)
        }

        DraftPostsView {
            id: blueskyDraftsView
            draftPosts: blueskyDraftPosts
            boundsBehavior: Flickable.DragAndOvershootBounds

            onSelected: (index) => blueskySelected(index)
            onDeleted: (index) => blueskyDeleted(index)
        }
    }

    Component.onCompleted: {
        if (localDraftsView.count === 0 && blueskyDraftsView.count > 0) {
            tabBar.setCurrentIndex(1)
            swipeView.setCurrentIndex(1)
        }
    }
}
