import QtQuick
import QtQuick.Controls
import QtMultimedia
import skywalker

Column {
    property string postCid // if set, then video source will be stored in the postFeedModel
    required property var videoView // videoView
    required property int contentVisibility // QEnums::ContentVisibility
    required property string contentWarning
    property string controlColor: guiSettings.textColor
    property string disabledColor: guiSettings.disabledColor
    property string backgroundColor: "transparent"
    property bool highlight: false
    property bool isVideoFeed: false
    property string borderColor: highlight ? guiSettings.borderHighLightColor : guiSettings.borderColor
    property int maxHeight: 0
    property bool isFullViewMode: false
    readonly property bool isFullVideoFeedViewMode: isFullViewMode && isVideoFeed
    readonly property bool isPlaying: videoPlayer.playing || videoPlayer.restarting
    property var userSettings: root.getSkywalker().getUserSettings()
    property string videoSource
    property string transcodedSource // Could be the same as videoSource if transcoding failed or not needed
    property bool autoLoad: userSettings.videoAutoPlay || userSettings.videoAutoLoad || isVideoFeed
    property bool autoPlay: userSettings.videoAutoPlay || isFullVideoFeedViewMode
    property int footerHeight: 0
    property int useIfNeededHeight: 0
    readonly property int playControlsWidth: playControls.width
    readonly property int playControlsHeight: playControls.height
    readonly property bool showPlayControls: playControls.show

    // Cache
    property list<string> tmpVideos: []

    signal videoLoaded

    id: videoStack
    spacing: isFullViewMode ? -playControls.height : 10

    Rectangle {
        property int fullVideoFeedHeight: Math.max(playControls.height, (parent.height - useIfNeededHeight - videoColumn.height) / 2 - parent.spacing)

        width: parent.width
        // Move video to top if it is close to the top of the screen
        height: isFullVideoFeedViewMode ? (fullVideoFeedHeight < playControls.height + 30 ? playControls.height : fullVideoFeedHeight) : 0
        color: "transparent"
        visible: isFullVideoFeedViewMode
    }

    Rectangle {
        id: videoRect
        width: parent.width
        height: videoColumn.height
        color: "transparent"
        visible: videoPlayer.videoFound || videoPlayer.error == MediaPlayer.NoError

        FilteredImageWarning {
            id: filter
            x: 10
            y: 10
            width: parent.width - 20
            contentVisibiliy: videoStack.contentVisibility
            contentWarning: videoStack.contentWarning
            imageUrl: videoView.thumbUrl
            isVideo: true
        }

        Column {
            id: videoColumn
            width: parent.width
            topPadding: 1
            spacing: 3

            Rectangle {
                width: parent.width
                height: filter.height > 0 ? filter.height + 20 : 0
                color: "transparent"
            }

            Rectangle {
                id: imgPreview
                width: parent.width
                height: defaultThumbImg.visible ? defaultThumbImg.height : thumbImg.height
                color: "transparent"

                ThumbImageView {
                    property double aspectRatio: implicitHeight > 0 ? implicitWidth / implicitHeight : 0
                    property double maxWidth: maxHeight * aspectRatio

                    id: thumbImg
                    x: (parent.width - width) / 2
                    width: parent.width - 2
                    imageView: filter.imageVisible() ? videoView.imageView : filter.nullImage
                    fillMode: Image.PreserveAspectFit
                    enableAlt: !isFullViewMode

                    onWidthChanged: Qt.callLater(setSize)
                    onHeightChanged: Qt.callLater(setSize)

                    function setSize() {
                        if (maxWidth > 0 && width > maxWidth)
                            height = maxHeight
                        else if (aspectRatio > 0)
                            height = width / aspectRatio
                    }

                    Component.onCompleted: setSize()
                }
                Rectangle {
                    property double maxWidth: maxHeight * videoStack.getAspectRatio()

                    id: defaultThumbImg
                    x: (parent.width - width) / 2
                    width: (maxWidth > 0 && parent.width - 2 > maxWidth) ? maxWidth : parent.width - 2
                    height: width / videoStack.getAspectRatio()
                    color: guiSettings.avatarDefaultColor
                    visible: videoView.imageView.isNull() || thumbImg.status != Image.Ready && filter.imageVisible()

                    onHeightChanged: {
                        if (maxHeight && height > maxHeight)
                            Qt.callLater(setMaxHeight)
                    }

                    function setMaxHeight() {
                        const ratio = width / height
                        width = Math.floor(maxHeight * ratio)
                    }

                    SkySvg {
                        x: (parent.width - width) / 2
                        y: (parent.height - height) / 2 + height
                        width: Math.min(parent.width, 150)
                        height: width
                        color: "white"
                        svg: SvgFilled.film
                    }
                }

                SkyLabel {
                    anchors.right: parent.right
                    anchors.rightMargin: 10
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 5
                    backgroundColor: "black"
                    backgroundOpacity: 0.6
                    color: "white"
                    text: guiSettings.videoDurationToString(videoPlayer.getDuration())
                    visible: !isFullViewMode && filter.imageVisible()
                }
            }
        }

        SvgButton {
            x: (parent.width - width) / 2
            y: (parent.height - height) / 2
            width: 50
            height: width
            opacity: 0.5
            accessibleName: qsTr("play video")
            svg: SvgFilled.play
            visible: filter.imageVisible() && !videoPlayer.playing && !videoPlayer.restarting
            enabled: videoPlayer.hasVideo || !autoLoad

            onClicked: {
                if (transcodedSource)
                    videoPlayer.start()
                else if (!autoLoad)
                    m3u8Reader.loadStream()
            }

            BusyIndicator {
                anchors.fill: parent
                running: parent.visible && !parent.enabled && (videoPlayer.error == MediaPlayer.NoError || videoPlayer.videoFound)
            }
        }

        Item {
            id: player
            width: parent.width
            height: parent.height

            MediaPlayer {
                property bool videoFound: false
                property bool restarting: false
                property int m3u8DurationMs: 0

                id: videoPlayer
                source: transcodedSource
                loops: MediaPlayer.Infinite
                videoOutput: videoOutput
                audioOutput: audioOutput

                onHasVideoChanged: {
                    if (hasVideo)
                        videoFound = true
                }

                onPlayingChanged: {
                    if (videoPlayer.playing) {
                        restartTimer.set(false)
                    }
                }

                onErrorOccurred: (error, errorString) => { console.debug("Video error:", source, error, errorString) }

                function getDuration() {
                    return duration === 0 ? m3u8DurationMs : duration
                }

                function playPause() {
                    if (playbackState == MediaPlayer.PausedState)
                        play()
                    else
                        pause()
                }

                function start() {
                    if (!filter.imageVisible())
                        return

                    restartTimer.set(true)
                    play()
                }

                function stopPlaying() {
                    stop()
                    restartTimer.set(false)
                }

                function isLoading() {
                    return mediaStatus === MediaPlayer.LoadingMedia
                }
            }
            VideoOutput {
                id: videoOutput
                anchors.fill: parent
            }
            AudioOutput {
                id: audioOutput
                muted: !userSettings.videoSound || (autoPlay && !isFullViewMode)

                function toggleSound() {
                    if (autoPlay)
                        muted = !muted
                    else
                        userSettings.videoSound = !userSettings.videoSound
                }
            }

            // Adding the condition on transcoding to the first busy indicator
            // did not work as there is short moment when loading has stopped
            // and transcoding not started yet. Whem transcoding starts the busy
            // inidcator will not restart anymore.
            BusyIndicator {
                anchors.centerIn: parent
                running: (videoPlayer.playing && videoPlayer.isLoading()) ||
                         videoPlayer.restarting ||
                         (m3u8Reader.loading && !autoLoad)
            }
            BusyIndicator {
                anchors.centerIn: parent
                running: videoUtils.transcoding && !autoLoad
            }

            Timer {
                id: restartTimer
                interval: 5000
                onTriggered: {
                    videoPlayer.stop()
                    videoPlayer.restarting = false
                    console.warn("Failed to start video")
                }

                function set(on) {
                    videoPlayer.restarting = on

                    if (on)
                        start()
                    else
                        stop()
                }
            }
        }

        MouseArea {
            width: parent.width
            height: parent.height
            z: -1
            enabled: filter.imageVisible() && (!isVideoFeed || isFullViewMode)

            onClicked: {
                if (isFullViewMode)
                    playControls.show = !playControls.show
                else
                    root.viewFullVideo(videoView, transcodedSource)
            }
        }

        RoundCornerMask {
            width: parent.width
            height: parent.height
            maskColor: videoStack.backgroundColor == "transparent" ? guiSettings.backgroundColor : videoStack.backgroundColor
            visible: !isFullViewMode
        }
    }

    Rectangle {
        width: parent.width
        height: isFullVideoFeedViewMode ? Math.max(0, parent.height - videoRect.y - videoRect.height - footerHeight - parent.spacing) : 0
        color: "transparent"
        visible: isFullVideoFeedViewMode
    }

    Rectangle {
        property bool show: true

        id: playControls
        x: (parent.width - width) / 2
        width: 2 + (defaultThumbImg.visible ? defaultThumbImg.width : Math.min(thumbImg.width, thumbImg.maxWidth ? thumbImg.maxWidth : thumbImg.width))
        height: visible ? playPauseButton.height : 0
        color: "transparent"
        visible: show && (videoPlayer.playbackState == MediaPlayer.PlayingState || videoPlayer.playbackState == MediaPlayer.PausedState || videoPlayer.restarting)

        Loader {
            anchors.fill: parent
            active: isFullViewMode

            Rectangle {
                anchors.fill: parent
                gradient: Gradient {
                    GradientStop { position: 0.0; color: isVideoFeed ? "#D0000000" : "#00000000" }
                    GradientStop { position: 1.0; color: isVideoFeed ? "#FF000000" : "#5F000000" }
                }
            }
        }

        SvgTransparentButton {
            id: playPauseButton
            x: isFullViewMode ? 10 : 0
            width: 32
            height: width
            svg: videoPlayer.playing ? SvgFilled.pause : SvgFilled.play
            accessibleName: videoPlayer.playing ? qsTr("pause video") : qsTr("continue playing video")
            color: controlColor

            onClicked: videoPlayer.playPause()
        }

        SvgTransparentButton {
            id: stopButton
            anchors.left: playPauseButton.right
            anchors.leftMargin: 10
            width: 32
            height: width
            svg: SvgFilled.stop
            accessibleName: qsTr("stop video")
            color: controlColor

            onClicked: videoPlayer.stopPlaying()
        }

        ProgressBar {
            id: playProgress
            anchors.left: stopButton.right
            anchors.leftMargin: 15
            anchors.right: remainingTimeText.left
            anchors.rightMargin: 20
            anchors.verticalCenter: parent.verticalCenter
            from: 0
            to: videoPlayer.duration
            value: videoPlayer.position

            background: Rectangle {
                implicitWidth: parent.width
                implicitHeight: 4
                color: disabledColor
            }

            contentItem: Item {
                implicitWidth: parent.width
                implicitHeight: 4

                Rectangle {
                    width: playProgress.visualPosition * parent.width
                    height: parent.height
                    color: controlColor
                }
            }

            Rectangle {
                x: playProgress.visualPosition * playProgress.availableWidth
                y: playProgress.topPadding + playProgress.availableHeight / 2 - height / 2

                width: 10
                height: width
                radius: width / 2
                color: controlColor
            }
        }

        MouseArea {
            x: playProgress.x
            width: playProgress.width
            height: playControls.height
            onClicked: (event) => {
                videoPlayer.position = (videoPlayer.duration / width) * event.x
            }
            onPositionChanged: (event) => {
                videoPlayer.position = (videoPlayer.duration / width) * event.x
            }
        }

        AccessibleText {
            id: remainingTimeText
            anchors.right: soundButton.left
            anchors.rightMargin: 10
            anchors.verticalCenter: parent.verticalCenter
            font.pointSize: guiSettings.scaledFont(6/8)
            color: controlColor
            text: guiSettings.videoDurationToString(videoPlayer.duration - videoPlayer.position)
        }

        SvgTransparentButton {
            id: soundButton
            anchors.right: parent.right
            anchors.rightMargin: isFullViewMode ? 10 : 0
            width: visible ? 32 : 0
            height: width
            svg: audioOutput.muted ? SvgOutline.soundOff : SvgOutline.soundOn
            accessibleName: audioOutput.muted ? qsTr("turn sound on") : qsTr("turn sound off")
            color: controlColor
            visible: videoPlayer.hasAudio

            onClicked: audioOutput.toggleSound()
        }
    }

    Rectangle {
        width: parent.width
        height: errorText.height
        radius: 10
        border.width: 1
        border.color: videoStack.borderColor
        color: "transparent"
        visible: !videoPlayer.videoFound && videoPlayer.error != MediaPlayer.NoError

        AccessibleText {
            id: errorText
            padding: 10
            text: qsTr(`⚠️ Video error: ${videoPlayer.errorString}`)
        }
    }

    M3U8Reader {
        id: m3u8Reader

        onGetVideoStreamOk: (durationMs) => {
            videoPlayer.m3u8DurationMs = durationMs

            if (autoLoad)
                loadStream()
        }

        onGetVideoStreamError: {
            videoSource = videoView.playlistUrl
            transcodedSource = videoSource

            if (autoPlay)
                videoPlayer.start()
        }

        onLoadStreamOk: (videoStream) => {
            videoSource = videoStream

            // TS streams do not loop well in the media player, also seeking works mediocre.
            // Therefore we transcode to MP4
            if (videoStream.endsWith(".ts")) {
                console.debug("Transcode to MP4:", videoStream)
                videoUtils.transcodeVideo(videoStream.slice(7), -1, -1, -1, false)
            }
            else if (!autoLoad || autoPlay) {
                transcodedSource = videoSource
                videoUtils.setVideoTranscodedSource(postCid, transcodedSource)
                videoPlayer.start()
            }
            else {
                videoLoaded()
            }
        }

        onLoadStreamError: {
            console.warn("Could not load stream")
        }
    }

    VideoUtils {
        id: videoUtils
        skywalker: root.getSkywalker()

        onTranscodingOk: (inputFileName, outputFileName) => {
            console.debug("Set MP4 source:", outputFileName)
            transcodedSource = "file://" + outputFileName
            videoStack.tmpVideos.push(transcodedSource)
            videoUtils.setVideoTranscodedSource(postCid, transcodedSource)

            if (!autoLoad || autoPlay)
                videoPlayer.start()
            else
                videoLoaded()
        }

        onTranscodingFailed: (inputFileName, errorMsg) => {
            console.debug("Could not transcode to MP4:", inputFileName, "error:", errorMsg)
            transcodedSource = videoSource

            if (!autoLoad || autoPlay)
                videoPlayer.start()
            else
                videoLoaded()
        }
    }

    function pause() {
        if (videoPlayer.playing)
        {
            if (!autoPlay)
                videoPlayer.pause()
            else
                audioOutput.muted = true
        }
    }

    function play() {
        if (!videoPlayer.playing)
            videoPlayer.start()
    }

    function getAspectRatio() {
        if (videoView && videoView.width > 0 && videoView.height > 0)
            return videoView.width / videoView.height
        else
            return 16/9
    }

    function getDurationMs() {
        return videoPlayer.getDuration()
    }

    function setVideoSource() {
        console.debug("Set video source for:", videoView.playlistUrl)

        if (videoView.playlistUrl.endsWith(".m3u8")) {
            m3u8Reader.getVideoStream(videoView.playlistUrl)
        }
        else {
            videoSource = videoView.playlistUrl
            transcodedSource = videoSource

            if (autoPlay)
                videoPlayer.start()
        }
    }

    Component.onDestruction: {
        tmpVideos.forEach((value, index, array) => {
            videoUtils.setVideoTranscodedSource(postCid, "")
            videoUtils.dropVideo(value)
        })
    }

    Component.onCompleted: {
        if (!transcodedSource)
            setVideoSource()
        else if (autoPlay)
            videoPlayer.start()
    }
}
