import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyListView {
    required property var skywalker
    readonly property string sideBarTitle: qsTr("Bookmarks")
    readonly property SvgImage sideBarSvg: SvgOutline.bookmark

    signal closed

    id: bookmarksView
    model: skywalker.createBookmarksModel()

    header: Item {
        width: parent.width
        height: portraitHeader.visible ? portraitHeader.height : landscapeHeader.height
        z: guiSettings.headerZLevel

        SimpleHeader {
            id: portraitHeader
            text: sideBarTitle
            visible: !root.showSideBar
            onBack: bookmarksView.closed()
        }
        DeadHeaderMargin {
            id: landscapeHeader
            visible: !portraitHeader.visible
        }
    }
    headerPositioning: ListView.OverlayHeader

    footer: DeadFooterMargin {}
    footerPositioning: ListView.OverlayFooter

    delegate: PostFeedViewDelegate {
        width: bookmarksView.width
    }

    FlickableRefresher {
        inProgress: model.getFeedInProgress
        topOvershootFun: () => skywalker.getBookmarks().getBookmarks()
        bottomOvershootFun: () => skywalker.getBookmarks().getBookmarksNextPage()
        topText: qsTr("Pull down to refresh bookmarks")
    }

    EmptyListIndication {
        y: parent.headerItem ? parent.headerItem.height : 0
        svg: SvgOutline.noPosts
        text: qsTr("No bookmarks")
        list: bookmarksView
    }

    BusyIndicator {
        id: busyIndicator
        anchors.centerIn: parent
        running: model.getFeedInProgress
    }


    Component.onDestruction: {
        skywalker.deleteBookmarksModel()
    }
}
