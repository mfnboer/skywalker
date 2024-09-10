import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia
import skywalker

SkyPage {
    property string videoSource
    readonly property int maxDurationMs: 60000
    readonly property int margin: 10

    id: page
    width: parent.width
    height: parent.height
    padding: margin

    signal cancel
    signal videoEdited(int height, int startMs, int endMs)

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
            onClicked: videoEdited(getNewHeight(), durationControl.first.value, durationControl.second.value)
        }
    }

    Video {
        readonly property var videoResolution: metaData.value(MediaMetaData.Resolution)
        readonly property int videoWidth: videoResolution ? videoResolution.width : 0
        readonly property int videoHeight: videoResolution ? videoResolution.height : 0
        readonly property double aspectRatio: videoHeight > 0 ? videoWidth / videoHeight : 0
        readonly property int maxHeight: parent.height - resizeControl.height - durationControl.height - 5
        readonly property int maxWidth: maxHeight * aspectRatio

        id: video
        source: page.videoSource
        fillMode: VideoOutput.PreserveAspectFit
        width: maxWidth > 0 && parent.width > maxWidth ? maxWidth : parent.width
        height: width / aspectRatio

        onPositionChanged: {
            if (video.playbackState !== MediaPlayer.PlayingState)
                return

            if (position >= durationControl.second.value)
                pause()
        }

        SvgButton {
            x: (parent.width - width) / 2
            y: (parent.height - height) / 2
            width: 50
            height: width
            opacity: 0.5
            accessibleName: video.playbackState !== MediaPlayer.PlayingState ? qsTr("play video") : qsTr("stop video")
            svg: video.playbackState !== MediaPlayer.PlayingState ? svgFilled.play : svgFilled.stop
            enabled: video.hasVideo

            onClicked: {
                if (video.playbackState !== MediaPlayer.PlayingState) {
                    video.position = durationControl.first.value
                    video.play()
                }
                else {
                    video.pause()
                }
            }
        }

        Label {
            anchors.right: parent.right
            anchors.rightMargin: 5
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 5
            padding: 5
            color: "white"
            font.pointSize: guiSettings.scaledFont(6/8)
            text: guiSettings.videoDurationToString(durationControl.duration)

            background: Rectangle {
                radius: 3
                color: "black"
                opacity: 0.6
            }
        }
    }

    RangeSlider {
        property int duration: second.value - first.value

        id: durationControl
        anchors.top: video.bottom
        anchors.topMargin: 5
        width: parent.width
        from: 0
        to: video.duration
        stepSize: 1
        snapMode: RangeSlider.SnapOnRelease
        first.value: 0
        second.value: Math.min(to, maxDurationMs)

        first.onValueChanged: {
            if (second.value - first.value > maxDurationMs)
                second.value = first.value + maxDurationMs

            video.position = first.value
        }

        second.onValueChanged: {
            if (second.value - first.value > maxDurationMs)
                first.value = second.value - maxDurationMs

            video.position = second.value
        }

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

        Rectangle {
            x: durationControl.leftPadding + video.position * (durationControl.availableWidth / video.duration) - width / 2
            y: durationControl.topPadding + durationControl.availableHeight / 2 - height / 2

            width: 10
            height: width
            radius: width / 2
            color: guiSettings.buttonColor
            visible: video.playbackState === MediaPlayer.PlayingState
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
            id: resolutionChoice
            Layout.fillWidth: true
            spacing: -1
            visible: calcHdResolution() !== Qt.size(0, 0)

            SkyRadioButton {
                id: hdRadioButton
                Layout.fillWidth: true
                text: `HD (${(sizeString(calcHdResolution()))})`
                checked: true
            }
            SkyRadioButton {
                id: sdRadioButton
                Layout.fillWidth: true
                text: `SD (${(sizeString(calcSdResolution()))})`
            }
        }
    }

    Timer {
        id: pauseTimer
        interval: 200
        onTriggered: video.pause()
    }

    function calcHdResolution() {
        if (video.videoWidth > 1280 || video.videoHeight > 720) {
            const xScale = video.videoWidth / 1280
            const yScale = video.videoHeight / 720
            const scale = Math.max(xScale, yScale)
            return Qt.size(Math.floor(video.videoWidth / scale), Math.floor(video.videoHeight / scale))
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
            return Qt.size(Math.floor(video.videoWidth / scale), Math.floor(video.videoHeight / scale))
        }

        return Qt.size(video.videoWidth, video.videoHeight)
    }

    function sizeString(sz) {
        return `${sz.width} x ${sz.height}`
    }

    function getNewHeight() {
        if (!resolutionChoice.visible)
            return video.videoHeight

        if (hdRadioButton.checked)
            return calcHdResolution().height

        return calcSdResolution().height
    }

    GuiSettings {
        id: guiSettings
    }

    Component.onCompleted: {
        video.play()
        pauseTimer.start()
    }
}
