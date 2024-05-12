import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

ListView {
    required property var skywalker

    signal closed

    id: bookmarksView
    spacing: 0
    model: skywalker.createBookmarksModel()
    clip: true
    flickDeceleration: guiSettings.flickDeceleration
    ScrollIndicator.vertical: ScrollIndicator {}

    Accessible.role: Accessible.List

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
        svg: svgOutline.noPosts
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
