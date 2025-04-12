import QtQuick
import QtQuick.Controls
import QtMultimedia
import skywalker

Column {
    required property var videoView // videoView
    required property int contentVisibility // QEnums::ContentVisibility
    required property string contentWarning
    property string controlColor: guiSettings.textColor
    property string disabledColor: guiSettings.disabledColor
    property string backgroundColor: "transparent"
    property bool highlight: false
    property bool swipeMode: false
    property string borderColor: highlight ? guiSettings.borderHighLightColor : guiSettings.borderColor
    property int maxHeight: root.height
    property bool isFullViewMode: false
    readonly property bool isFullVideoFeedViewMode: isFullViewMode && swipeMode
    readonly property bool isPlaying: videoPlayer.playing || videoPlayer.restarting
    property var userSettings: root.getSkywalker().getUserSettings()

    property bool streamingEnabled: userSettings.videoStreamingEnabled
    property string videoSource: streamingEnabled ? videoView.playlistUrl : ""
    property string transcodedSource

    property bool autoLoad: (userSettings.videoAutoPlay || userSettings.videoAutoLoad) && !swipeMode || isFullVideoFeedViewMode || streamingEnabled
    property bool autoPlay: (userSettings.videoAutoPlay && !swipeMode) || isFullVideoFeedViewMode

    property int useIfNeededHeight: 0
    property bool tileMode: false
    readonly property int playControlsWidth: playControls.width
    readonly property int playControlsHeight: playControls.height
    readonly property bool showPlayControls: playControls.show
    readonly property bool videoPlayingOrPaused: videoPlayer.playbackState == MediaPlayer.PlayingState || videoPlayer.playbackState == MediaPlayer.PausedState || videoPlayer.restarting

    // The meta information for video can be wrong.
    // I have seen 2160 x 3840 whereas the video and the thumbnail are 1080 x 1920
    // I have also seen a correct size for the video 1080 x 1920, but with a thumbnail image
    // having size 1080 x 1080
    // The meta information is most importan for timeline views, to know the size of a
    // full post in advance. For full screen video, use as much of the screen and ignore
    // meta information.
    readonly property bool videoSizeIsKnown: videoView.width > 0 && videoView.height > 0 && !isFullViewMode

    // Move video to top in swipe mode if it is close to the top to avoid covering too much
    // by the post text
    readonly property bool moveToTop: isFullVideoFeedViewMode && thumbImg && (height - thumbImg.getHeight()) / 2 < useIfNeededHeight + playControlsHeight && height !== thumbImg.getHeight()

    property alias contentFilter: filter

    // Cache
    property var videoHandle

    signal videoLoaded
    signal activateSwipe

    id: videoStack
    spacing: isFullViewMode ? -playControls.height : 10

    Rectangle {
        id: videoRect
        width: parent.width
        height: tileMode ? parent.height : videoColumn.height

        // The high light color is visible when the thumbnail image is smaller than the
        // given aspect ratio size.
        color: isFullViewMode ? "transparent" : guiSettings.postHighLightColor
        visible: videoPlayer.videoFound || videoPlayer.error == MediaPlayer.NoError

        FilteredImageWarning {
            id: filter
            x: 10
            y: 10
            width: parent.width - 20
            contentVisibility: videoStack.contentVisibility
            contentWarning: videoStack.contentWarning
            imageUrl: videoView.thumbUrl
            isVideo: true
        }

        Column {
            id: videoColumn
            width: parent.width
            spacing: 3

            Rectangle {
                width: parent.width
                height: filter.height > 0 ? filter.height + 20 : 0
                color: "transparent"
            }

            Rectangle {
                id: imgPreview
                width: parent.width
                height: tileMode ? videoStack.height : (defaultThumbImg.visible ? defaultThumbImg.height : thumbImg.getDisplayHeight())
                color: "transparent"

                onWidthChanged: {
                    // Force image to resize
                    if (thumbImg.active) {
                        thumbImg.active = false
                        thumbImg.active = true
                    }
                }

                Loader {
                    id: thumbImg
                    x: (parent.width - getWidth()) / 2
                    y: moveToTop ? 0 : (parent.height - getHeight()) / 2
                    active: filter.imageVisible()
                    sourceComponent: videoSizeIsKnown ? knownSizeComp : unknownSizeComp

                    function getWidth() {
                        return item ? item.width : 0
                    }

                    function getHeight() {
                        return item ? item.height: 0
                    }

                    function getDisplayWidth() {
                        if (!filter.imageVisible())
                            return filter.width

                        if (isFullViewMode)
                            return videoStack.width

                        return item ? (videoSizeIsKnown ? item.width : item.paintedWidth) : 0
                    }

                    function getDisplayHeight() {
                        if (isFullViewMode)
                            return videoStack.height

                        return item ? (videoSizeIsKnown ? item.height : item.paintedHeight) : 0
                    }

                    function getStatus() {
                        return item ? item.status : Image.Null
                    }
                }

                Rectangle {
                    property double maxWidth: maxHeight * videoStack.getAspectRatio()

                    id: defaultThumbImg
                    x: (parent.width - width) / 2
                    width: tileMode ? parent.width : ((maxWidth > 0 && parent.width > maxWidth) ? maxWidth : parent.width)
                    height: tileMode ? videoStack.height : (width / videoStack.getAspectRatio())
                    color: guiSettings.avatarDefaultColor
                    visible: (videoView.imageView.isNull() || thumbImg.getStatus() !== Image.Ready && filter.imageVisible()) && !videoPlayer.playing

                    onHeightChanged: {
                        if (maxHeight && height > maxHeight && !tileMode)
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
                    visible: !isFullViewMode && !tileMode && filter.imageVisible()
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
            enabled: videoPlayer.hasVideo || !autoLoad || streamingEnabled

            onClicked: {
                if (swipeMode && !isFullViewMode) {
                    activateSwipe()
                    return
                }

                if (transcodedSource) {
                    videoPlayer.start()
                    return
                }

                videoHandle = videoUtils.getVideoFromCache(videoView.playlistUrl)

                if (videoHandle.isValid()) {
                    videoSource = videoView.playlistUrl
                    transcodedSource = "file://" + videoHandle.fileName
                    videoPlayer.start()
                    return;
                }


                if (!autoLoad) {
                    if (streamingEnabled) {
                        transcodedSource = videoSource
                        videoPlayer.start()
                    }
                    else {
                        m3u8Reader.loadStream()
                    }
                }
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
            visible: !swipeMode || isFullViewMode

            MediaPlayer {
                property bool videoFound: false
                property bool restarting: false
                property int m3u8DurationMs: 0
                property var keepScreenOnHandle

                id: videoPlayer
                source: transcodedSource
                loops: userSettings.videoLoopPlay ? MediaPlayer.Infinite : 1
                videoOutput: videoOutput
                audioOutput: audioOutput

                onHasVideoChanged: {
                    if (hasVideo)
                        videoFound = true
                }

                onPlayingChanged: {
                    if (videoPlayer.playing) {
                        restartTimer.set(false)
                        keepScreenOnHandle = displayUtils.keepScreenOn()
                    }
                    else {
                        keepScreenOnHandle.destroy()
                    }
                }

                onErrorOccurred: (error, errorString) => {
                    if (error === MediaPlayer.ResourceError &&
                        videoUtils.isTempVideoSource(transcodedSource) &&
                        !videoUtils.videoSourceExists(transcodedSource))
                    {
                        console.debug("Video error:", source, error, errorString)
                        console.debug("Reload video")
                        transcodedSource = ""
                        videoHandle.destroy()
                        setVideoSource()
                    }
                }

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

                    if (!transcodedSource) {
                        console.warn("No transcoded source:", videoView.playlistUrl)
                        setVideoSource()
                    }

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
                y: moveToTop ? -contentRect.y : 0
                width: parent.width
                height: parent.height

                // Avoid flicker when video is moved to top
                visible: !moveToTop || contentRect.y > 0
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
                    console.warn("Failed to start video:", transcodedSource, videoView.playlistUrl)
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
            enabled: filter.imageVisible() && (!swipeMode || isFullViewMode)

            onClicked: {
                if (isFullViewMode)
                    playControls.show = !playControls.show
                else
                    root.viewFullVideo(videoView)
            }
        }

        RoundCornerMask {
            width: parent.width
            height: parent.height
            maskColor: videoStack.backgroundColor == "transparent" ? guiSettings.backgroundColor : videoStack.backgroundColor
            visible: !isFullViewMode && !swipeMode && !tileMode
        }
    }

    Rectangle {
        property bool show: true

        id: playControls
        x: (parent.width - width) / 2
        width: 2 + (defaultThumbImg.visible ? defaultThumbImg.width : thumbImg.getDisplayWidth())
        height: visible ? playPauseButton.height : 0
        color: "transparent"
        visible: show && videoPlayingOrPaused

        Loader {
            anchors.fill: parent
            active: isFullViewMode

            sourceComponent: Rectangle {
                anchors.fill: parent
                gradient: Gradient {
                    GradientStop { position: 0.0; color: swipeMode ? "#D0000000" : "#00000000" }
                    GradientStop { position: 1.0; color: swipeMode ? "#FF000000" : "#5F000000" }
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
        videoQuality: userSettings.videoQuality // qmllint disable missing-type

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
        skywalker: root.getSkywalker() // qmllint disable missing-type

        onTranscodingOk: (inputFileName, outputFileName) => {
            console.debug("Set MP4 source:", outputFileName)
            videoHandle = videoUtils.cacheVideo(videoView.playlistUrl, outputFileName)
            transcodedSource = "file://" + videoHandle.fileName
            m3u8Reader.resetStream()
            videoSource = ""

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

    DisplayUtils {
        id: displayUtils
        skywalker: root.getSkywalker() // qmllint disable missing-type
    }

    Timer {
        id: inactiveTimer
        interval: 30000
        onTriggered: {
            console.debug() << "Entering inactive state"
            clearCache()
        }
    }

    Component {
        id: unknownSizeComp

        ThumbImageUnknownSizeView {
            maxWidth: imgPreview.width
            maxHeight: videoStack.tileMode ? imgPreview.height : videoStack.maxHeight
            image: videoView.imageView
            indicateLoading: false
            tileMode: videoStack.tileMode
            noCrop: videoStack.isFullViewMode
        }
    }

    Component {
        id: knownSizeComp

        ThumbImageKnownSizeView {
            maxWidth: imgPreview.width
            maxHeight: videoStack.tileMode ? imgPreview.height : videoStack.maxHeight
            image: videoView.imageView
            indicateLoading: false
            tileMode: videoStack.tileMode
            noCrop: videoStack.isFullViewMode
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
        if (videoSizeIsKnown)
            return videoView.width / videoView.height
        else
            return videoStack.width / videoStack.height
    }

    function getDurationMs() {
        return videoPlayer.getDuration()
    }

    function setVideoSource() {
        console.debug("Set video source for:", videoView.playlistUrl)

        if (streamingEnabled) {
            console.debug("Streaming enabled, autoLoad:", autoLoad, "autoPlay:", autoPlay, "videoSource:", videoSource)

            if (autoLoad)
                transcodedSource = videoSource

            if (autoPlay)
                videoPlayer.start()
        }
        else if (videoView.playlistUrl.endsWith(".m3u8")) {
            videoHandle = videoUtils.getVideoFromCache(videoView.playlistUrl)

            if (videoHandle.isValid()) {
                videoSource = videoView.playlistUrl
                transcodedSource = "file://" + videoHandle.fileName

                if (autoPlay)
                    videoPlayer.start()
            }
            else {
                m3u8Reader.getVideoStream(videoView.playlistUrl)
            }
        }
        else {
            videoSource = videoView.playlistUrl
            transcodedSource = videoSource

            if (autoPlay)
                videoPlayer.start()
        }
    }

    function clearCache() {
        console.debug("Clear cache:", videoView.playlistUrl)

        if (transcodedSource && videoUtils.isTempVideoSource(transcodedSource))
            transcodedSource = ""

        if (videoHandle)
            videoHandle.destroy()
    }

    function activate() {
        console.debug("Activate VideoView:", videoView.playlistUrl)
        inactiveTimer.stop()
    }

    function deactivate() {
        console.debug("Deactivate VideoView:", videoView.playlistUrl)
        inactiveTimer.start()
    }

    Component.onDestruction: {
        console.debug("Destruct VideoView:", videoView.playlistUrl)
        clearCache()
    }

    Component.onCompleted: {
        if (!transcodedSource)
            setVideoSource()
        else if (autoPlay)
            videoPlayer.start()
    }
}
