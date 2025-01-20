import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import skywalker

SkyPage {
    required property var videoView // videoview
    property string videoSource
    property string transcodedSource
    property bool isVideoFeed: false

    signal closed

    id: page
    width: parent.width
    height: parent.height
    background: Rectangle { color: "black" }

    VideoView {
        id: view
        y: (parent.height - height) / 2
        width: parent.width
        maxHeight: parent.height
        videoView: page.videoView
        videoSource: page.videoSource
        transcodedSource: page.transcodedSource
        contentVisibility: QEnums.CONTENT_VISIBILITY_SHOW
        contentWarning: ""
        controlColor: "white"
        disabledColor: "darkslategrey"
        backgroundColor: "black"
        isVideoFeed: page.isVideoFeed
        isFullViewMode: true
    }

    Rectangle {
        width: parent.width
        anchors.top: altFlick.top
        anchors.bottom: parent.bottom
        color: "black"
        opacity: 0.7
        visible: altFlick.visible
    }

    Flickable {
        id: altFlick
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10
        width: parent.width
        height: Math.min(contentHeight, 6 * 21)
        clip: true
        contentWidth: parent.width
        contentHeight: altText.contentHeight
        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds
        ScrollBar.vertical: ScrollBar { id: altScrollBar }
        visible: !view.isPlaying

        onHeightChanged: setScrollBarPolicy()
        onContentHeightChanged: setScrollBarPolicy()

        function setScrollBarPolicy() {
            altScrollBar.policy = contentHeight > height ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
            altScrollBar.contentItem.color = "#1d3030"
        }

        SkyCleanedText {
            id: altText
            leftPadding: 10
            width: parent.width - 15
            wrapMode: Text.Wrap
            color: "white"
            plainText: videoView.alt
            visible: Boolean(videoView.alt)
        }
    }

    SvgButton {
        x: 10
        iconColor: "white"
        Material.background: "black"
        opacity: 0.7
        svg: SvgOutline.arrowBack
        accessibleName: qsTr("go back")
        visible: !view.isPlaying
        onClicked: page.closed()
    }

    SvgButton {
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.rightMargin: 10
        iconColor: "white"
        Material.background: "black"
        opacity: 0.7
        svg: SvgOutline.moreVert
        accessibleName: qsTr("more options")
        visible: !view.isPlaying
        onClicked: moreMenu.open()

        Menu {
            id: moreMenu
            modal: true

            MenuItem {
                text: qsTr("Save video")
                onTriggered: page.saveVideo()

                MenuItemSvg {
                    svg: SvgOutline.save
                }
            }

            MenuItem {
                text: qsTr("Translate")
                enabled: Boolean(videoView.alt)
                onTriggered: root.translateText(videoView.alt)

                MenuItemSvg {
                    svg: SvgOutline.googleTranslate
                }
            }
        }
    }

    BusyIndicator {
        anchors.centerIn: parent
        running: m3u8Reader.loading || videoUtils.transcoding
    }

    M3U8Reader {
        id: m3u8Reader

        onGetVideoStreamOk: (durationMs) => {
            const fileName = videoUtils.getVideoFileNameForGallery("ts")

            if (!fileName) {
                root.getSkywalker().showStatusMessage(qsTr("Cannot create gallery file"), QEnums.STATUS_LEVEL_ERROR)
                return
            }

            loadStream(fileName)
        }

        // NOTE: Transcoding to MP4 results in an MP4 that cannot be played by the gallery
        // app. The MP4 can be played with other players. For now save as mpeg-ts. This
        // can be played by the gallery app.
        onGetVideoStreamError: root.getSkywalker().showStatusMessage(qsTr("Failed to save video"), QEnums.STATUS_LEVEL_ERROR)

        onLoadStreamOk: (videoSource) => {
            videoUtils.indexGalleryFile(videoSource.slice(7))
            root.getSkywalker().showStatusMessage(qsTr("Video saved"), QEnums.STATUS_LEVEL_INFO)
        }

        onLoadStreamError: root.getSkywalker().showStatusMessage(qsTr("Failed to save video"), QEnums.STATUS_LEVEL_ERROR)
    }

    VideoUtils {
        id: videoUtils
        skywalker: root.getSkywalker()

        onCopyVideoOk: root.getSkywalker().showStatusMessage(qsTr("Video saved"), QEnums.STATUS_LEVEL_INFO)
        onCopyVideoFailed: (error) => root.getSkywalker().showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
    }

    function saveVideo() {
        if (view.videoSource && view.videoSource.startsWith("file://")) {
            videoUtils.copyVideoToGallery(view.videoSource.slice(7))
        }
        else if (videoView.playlistUrl.endsWith(".m3u8")) {
            m3u8Reader.getVideoStream(videoView.playlistUrl)
            statusPopup.show(qsTr("Saving video"), QEnums.STATUS_LEVEL_INFO, 60)
        }
        else {
            root.getSkywalker().showStatusMessage(qsTr(`Cannot save: ${videoView.playlistUrl}`), QEnums.STATUS_LEVEL_ERROR)
        }
    }
}
