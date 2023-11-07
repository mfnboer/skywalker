import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

ListView {
    required property var skywalker
    property bool inTopOvershoot: false
    property bool inBottomOvershoot: false

    signal closed

    id: bookmarksView
    spacing: 0
    model: skywalker.createBookmarksModel()
    ScrollIndicator.vertical: ScrollIndicator {}

    header: Rectangle {
        width: parent.width
        height: guiSettings.headerHeight
        z: guiSettings.headerZLevel
        color: guiSettings.headerColor

        RowLayout {
            id: headerRow
            width: parent.width
            height: guiSettings.headerHeight

            SvgButton {
                id: backButton
                iconColor: "white"
                Material.background: "transparent"
                svg: svgOutline.arrowBack
                onClicked: bookmarksView.closed()
            }

            Text {
                id: headerTexts
                Layout.alignment: Qt.AlignVCenter
                leftPadding: 10
                font.bold: true
                font.pointSize: guiSettings.scaledFont(10/8)
                color: "white"
                text: qsTr("Bookmarks") + ` (${skywalker.bookmarks.size} / ${skywalker.bookmarks.maxSize})`
            }
        }
    }
    headerPositioning: ListView.OverlayHeader

    delegate: PostFeedViewDelegate {
        viewWidth: bookmarksView.width
    }

    onVerticalOvershootChanged: {
        if (verticalOvershoot < 0)  {
            if (!inTopOvershoot && !model.inProgress) {
                skywalker.getBookmarksPage(true)
            }

            inTopOvershoot = true
        } else {
            inTopOvershoot = false
        }

        if (verticalOvershoot > 0) {
            if (!inBottomOvershoot && !model.inProgress) {
                skywalker.getBookmarksPage()
            }

            inBottomOvershoot = true;
        } else {
            inBottomOvershoot = false;
        }
    }

    SvgImage {
        id: noPostImage
        width: 150
        height: 150
        y: height + (parent.headerItem ? parent.headerItem.height : 0)
        anchors.horizontalCenter: parent.horizontalCenter
        color: "grey"
        svg: svgOutline.noPosts
        visible: bookmarksView.count === 0
    }
    Text {
        id: noPostText
        y: noPostImage.y
        anchors.horizontalCenter: parent.horizontalCenter
        font.pointSize: guiSettings.scaledFont(10/8)
        color: "grey"
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
