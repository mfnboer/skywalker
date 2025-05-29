import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyListView {
    required property var skywalker
    readonly property string sideBarTitle: qsTr("Bookmarks")
    readonly property string sideBarSubTitle: `${skywalker.bookmarks.size} / ${skywalker.bookmarks.maxSize}`
    readonly property SvgImage sideBarSvg: SvgOutline.bookmark

    signal closed

    id: bookmarksView
    model: skywalker.createBookmarksModel()

    header: SimpleHeader {
        text: sideBarTitle
        subTitle: sideBarSubTitle
        visible: !root.showSideBar
        onBack: bookmarksView.closed()
    }
    headerPositioning: ListView.OverlayHeader

    footer: DeadFooterMargin {}
    footerPositioning: ListView.OverlayFooter

    delegate: PostFeedViewDelegate {
        width: bookmarksView.width
    }

    FlickableRefresher {
        inProgress: model.inProgress
        topOvershootFun: () => skywalker.getBookmarksPage(true)
        bottomOvershootFun: () => skywalker.getBookmarksPage()
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
        running: model.inProgress
    }


    Component.onDestruction: {
        skywalker.deleteBookmarksModel()
    }
}
