import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia
import skywalker

SkyPage {
    property string videoSource
    readonly property int margin: 10

    id: page
    width: parent.width
    height: parent.height
    // topPadding: margin
    // bottomPadding: margin
    padding: margin

    signal cancel
    signal videoEdited(string source)

    header: SimpleHeader {
        text: qsTr("Video editor")
        backIsCancel: true
        onBack: page.cancel()

        SvgButton {
            id: okButton
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            svg: svgOutline.check
            accessibleName: qsTr("process video")
            onClicked: videoEdited(videoSource)
        }
    }

    VideoThumbnail {
        readonly property var videoResolution: metaData.value(MediaMetaData.Resolution)
        readonly property int videoWidth: videoResolution ? videoResolution.width : 0
        readonly property int videoHeight: videoResolution ? videoResolution.height : 0
        readonly property double aspectRatio: videoHeight > 0 ? videoWidth / videoHeight : 0
        readonly property int maxHeight: parent.height - resizeControl.height - durationControl.height - 5
        readonly property int maxWidth: maxHeight * aspectRatio

        id: video
        videoSource: page.videoSource
        fillMode: VideoOutput.PreserveAspectFit
        width: maxWidth > 0 && parent.width > maxWidth ? maxWidth : parent.width
        height: width / aspectRatio
        showFilmIcon: false
    }

    RangeSlider {
        id: durationControl
        anchors.top: video.bottom
        anchors.topMargin: 5
        width: parent.width
        from: 0
        to: video.duration
        stepSize: 1
        snapMode: RangeSlider.SnapOnRelease
        first.value: 0
        second.value: to

        first.onValueChanged: video.position = first.value
        second.onValueChanged: video.position = second.value

        Text {
            x: durationControl.first.handle.x - 5
            y: 0
            font.pointSize: guiSettings.scaledFont(6/8)
            text: guiSettings.videoDurationToString(durationControl.first.value)
        }

        Text {
            x: durationControl.second.handle.x - 5
            y: 0
            font.pointSize: guiSettings.scaledFont(6/8)
            text: guiSettings.videoDurationToString(durationControl.second.value)
        }
    }

    GridLayout {
        id: resizeControl
        anchors.top: durationControl.bottom
        width: parent.width
        columns: 2

        AccessibleText {
            font.bold: true
            text: qsTr("Resolution")
        }

        AccessibleText {
            Layout.fillWidth: true
            text: `${video.videoWidth} x ${video.videoHeight}`
        }

        AccessibleText {
            font.bold: true
            text: qsTr("Resize to")
            visible: calcHdResolution() !== Qt.size(0, 0)
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: -1
            visible: calcHdResolution() !== Qt.size(0, 0)

            SkyRadioButton {
                Layout.fillWidth: true
                text: `HD (${(sizeString(calcHdResolution()))})`
                checked: true
            }
            SkyRadioButton {
                Layout.fillWidth: true
                text: `SD (${(sizeString(calcSdResolution()))})`
            }
        }
    }

    function calcHdResolution() {
        if (video.videoWidth > 1280 || video.videoHeight > 720) {
            const xScale = video.videoWidth / 1280
            const yScale = video.videoHeight / 720
            const scale = Math.max(xScale, yScale)
            return Qt.size(video.videoWidth / scale, video.videoHeight / scale)
        }

        if (video.videoWidth > 640 || video.videoHeight > 360)
            return Qt.size(video.videoWidth, video.videoHeight)

        return Qt.size(0, 0)
    }

    function calcSdResolution() {
        if (video.videoWidth > 640 || video.videoHeight > 360) {
            const xScale = video.videoWidth / 640
            const yScale = video.videoHeight / 360
            const scale = Math.max(xScale, yScale)
            return Qt.size(video.videoWidth / scale, video.videoHeight / scale)
        }

        return Qt.size(video.videoWidth, video.videoHeight)
    }

    function sizeString(sz) {
        return `${sz.width} x ${sz.height}`
    }

    GuiSettings {
        id: guiSettings
    }
}
