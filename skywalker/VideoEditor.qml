import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia
import skywalker

SkyPage {
    property string videoSource
    property int startMs: 0
    property int endMs: 0
    property int newHeight: 0
    readonly property int maxDurationMs: 60000
    readonly property int margin: 10
    property var userSettings: root.getSkywalker().getUserSettings()

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
            enabled: durationControl.first.value < durationControl.second.value
        }
    }

    Video {
        readonly property var videoResolution: metaData.value(MediaMetaData.Resolution)
        readonly property int videoWidth: videoResolution ? videoResolution.width : 0
        readonly property int videoHeight: videoResolution ? videoResolution.height : 0
        readonly property var videoOrientationValue: metaData.value(MediaMetaData.Orientation)
        readonly property int videoOrientation: videoOrientationValue ? videoOrientationValue : 0
        readonly property bool videoRotated: [-90, 90, 270].includes(videoOrientation)
        readonly property double aspectRatio: !videoRotated ? (videoHeight > 0 ? videoWidth / videoHeight : 0) : (videoWidth > 0 ? videoHeight / videoWidth : 0)
        readonly property int maxHeight: durationControl.y - 5
        readonly property int maxWidth: maxHeight * aspectRatio

        id: video
        source: page.videoSource
        fillMode: VideoOutput.PreserveAspectFit
        x: (parent.width - width) / 2
        width: maxWidth > 0 && parent.width > maxWidth ? maxWidth : parent.width
        height: width / aspectRatio
        muted: !userSettings.videoSound

        onPositionChanged: {
            if (video.playbackState !== MediaPlayer.PlayingState)
                return

            if (position >= durationControl.second.value)
                pause()
        }

        function toggleSound() {
            userSettings.videoSound = !userSettings.videoSound
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

        SkyLabel {
            anchors.left: parent.left
            anchors.leftMargin: 5
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 5
            backgroundColor: "black"
            backgroundOpacity: 0.6
            color: "white"
            text: guiSettings.videoDurationToString(durationControl.duration)
        }

        SvgButton {
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            width: 34
            height: 34
            svg: video.muted ? svgOutline.soundOff : svgOutline.soundOn
            accessibleName: video.muted ? qsTr("turn sound on") : qsTr("turn sound off")

            onClicked: video.toggleSound()
        }
    }

    RangeSlider {
        property int duration: second.value - first.value

        id: durationControl
        anchors.bottom: resizeControl.top
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
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10
        width: parent.width
        columns: 2

        AccessibleText {
            font.bold: true
            text: qsTr("Resolution")
        }

        AccessibleText {
            Layout.fillWidth: true
            text: sizeString(Qt.size(video.videoWidth, video.videoHeight))
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
        onTriggered: {
            video.pause()

            if (endMs > 0)
                durationControl.second.value = endMs

            durationControl.first.value = startMs

            if (page.newHeight > 0 && page.newHeight === calcSdResolution().height)
                sdRadioButton.checked = true
        }
    }

    function calcResolution(longRes, shortRes) {
        const longSide = video.videoWidth >= video.videoHeight ? video.videoWidth : video.videoHeight
        const shortSide = video.videoWidth >= video.videoHeight ? video.videoHeight : video.videoWidth

        if (longSide > longRes || shortSide > shortRes) {
            const longScale = longSide / longRes
            const shortScale = shortSide / shortRes
            const scale = Math.max(longScale, shortScale)
            return Qt.size(Math.floor(video.videoWidth / scale), Math.floor(video.videoHeight / scale))
        }

        return Qt.size(0, 0)
    }

    function calcHdResolution() {
        const resolution = calcResolution(1280, 720)
        console.debug("HD RESOULUTION", resolution)

        if (resolution !== Qt.size(0, 0))
            return resolution

        if (calcResolution(640, 360) !== Qt.size(0, 0))
            return Qt.size(video.videoWidth, video.videoHeight)

        return Qt.size(0, 0)
    }

    function calcSdResolution() {
        const resolution = calcResolution(640, 360)

        if (resolution !== Qt.size(0, 0))
            return resolution

        return Qt.size(video.videoWidth, video.videoHeight)
    }

    function sizeString(sz) {
        return !video.videoRotated ? `${sz.width} x ${sz.height}` : `${sz.height} x ${sz.width}`
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
