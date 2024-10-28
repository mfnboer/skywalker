import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyListView {
    required property var skywalker

    signal closed

    id: bookmarksView
    model: skywalker.createBookmarksModel()

    header: SimpleHeader {
        text: qsTr("Bookmarks") + ` (${skywalker.bookmarks.size} / ${skywalker.bookmarks.maxSize})`
        onBack: bookmarksView.closed()
    }
    headerPositioning: ListView.OverlayHeader

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

    GuiSettings {
        id: guiSettings
    }

    Component.onDestruction: {
        skywalker.deleteBookmarksModel()
    }
}
