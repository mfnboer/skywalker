import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyPage {
    required property var videoView // videoview
    property string videoSource

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
        contentVisibility: QEnums.CONTENT_VISIBILITY_SHOW
        contentWarning: ""
        controlColor: "white"
        disabledColor: "darkslategrey"
        backgroundColor: "black"
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
        svg: svgOutline.arrowBack
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
        svg: svgOutline.moreVert
        accessibleName: qsTr("more options")
        visible: !view.isPlaying
        onClicked: moreMenu.open()

        Menu {
            id: moreMenu
            modal: true

            MenuItem {
                text: qsTr("Save video")
                onTriggered: page.saveVideo()
                enabled: view.videoSource && view.videoSource.startsWith("file://")

                MenuItemSvg {
                    svg: svgOutline.save
                }
            }

            MenuItem {
                text: qsTr("Translate")
                enabled: Boolean(videoView.alt)
                onTriggered: root.translateText(videoView.alt)

                MenuItemSvg {
                    svg: svgOutline.googleTranslate
                }
            }
        }
    }

    BusyIndicator {
        running: m3u8Reader.loading || videoUtils.transcoding
    }

    M3U8Reader {
        property int duration: 0

        id: m3u8Reader

        onGetVideoStreamOk: (durationMs) => {
            duration = durationMs
            loadStream()
        }

        onGetVideoStreamError: root.getSkywalker().showStatusMessage(qsTr("Failed to save video"), QEnums.STATUS_LEVEL_ERROR)
        onLoadStreamOk: (videoSource) => videoUtils.copyVideoToGallery(videoSource.slice(7)) // videoUtils.transcodeVideo(videoSource.slice(7), 360, 0, duration)
        onLoadStreamError: root.getSkywalker().showStatusMessage(qsTr("Failed to save video"), QEnums.STATUS_LEVEL_ERROR)
    }

    VideoUtils {
        id: videoUtils

        // TODO: drop temp file
        onTranscodingOk: (inputFileName, outputFileName) => videoUtils.copyVideoToGallery(outputFileName)
        onTranscodingFailed: (inputFileName, error) => root.getSkywalker().showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
        onCopyVideoOk: root.getSkywalker().showStatusMessage(qsTr("Video saved"), QEnums.STATUS_LEVEL_INFO)
        onCopyVideoFailed: (error) => root.getSkywalker().showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
    }

    function saveVideo() {
        if (videoView.playlistUrl.endsWith(".m3u8"))
            m3u8Reader.getVideoStream(videoView.playlistUrl)
        else
            root.getSkywalker().showStatusMessage(qsTr(`Cannot save: ${videoView.playlistUrl}`), QEnums.STATUS_LEVEL_ERROR)
    }
}
