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

    SvgImage {
        id: noPostImage
        width: 150
        height: 150
        y: height + (parent.headerItem ? parent.headerItem.height : 0)
        anchors.horizontalCenter: parent.horizontalCenter
        color: Material.color(Material.Grey)
        svg: svgOutline.noPosts
        visible: bookmarksView.count === 0
    }
    Text {
        id: noPostText
        y: noPostImage.y
        anchors.horizontalCenter: parent.horizontalCenter
        font.pointSize: guiSettings.scaledFont(10/8)
        color: Material.color(Material.Grey)
        elide: Text.ElideRight
        text: qsTr("No bookmarks")
        visible: bookmarksView.count === 0
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
