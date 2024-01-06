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
    flickDeceleration: guiSettings.flickDeceleration
    ScrollIndicator.vertical: ScrollIndicator {}

    header: SimpleHeader {
        text: qsTr("Bookmarks") + ` (${skywalker.bookmarks.size} / ${skywalker.bookmarks.maxSize})`
        onBack: bookmarksView.closed()
    }
    headerPositioning: ListView.OverlayHeader

    delegate: PostFeedViewDelegate {
        viewWidth: bookmarksView.width
    }

    FlickableRefresher {
        inProgress: model.inProgress
        verticalOvershoot: bookmarksView.verticalOvershoot
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

    Component.onCompleted: {
        let userSettings = skywalker.getUserSettings()

        if (!skywalker.bookmarks.noticeSeen(userSettings)) {
            guiSettings.notice(bookmarksView, qsTr(
                "Bluesky does not support bookmarks. This is a feature from Skywalker. " +
                "The bookmarks are locally stored on this device, and cannot be accessed from " +
                "other devices or apps. <p>" +
                "When you uninstall the app, your bookmarks may be lost. " +
                "There is no guarantee that bookmarks will be kept with upgrades."
                ),
                () => skywalker.bookmarks.setNoticeSeen(userSettings, true));
        }
    }

    Component.onDestruction: {
        skywalker.deleteBookmarksModel()
    }
}
