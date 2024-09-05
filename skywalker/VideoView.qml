import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia
import skywalker

Column {
    required property var videoView // videoView
    required property int contentVisibility // QEnums::ContentVisibility
    required property string contentWarning
    property string controlColor: guiSettings.textColor
    property string disabledColor: guiSettings.disabledColor
    property string backgroundColor: guiSettings.backgroundColor
    property int maxHeight: 0
    property bool isFullViewMode: false
    property var userSettings: root.getSkywalker().getUserSettings()

    id: videoStack
    spacing: 10

    Rectangle {
        width: parent.width
        height: videoColumn.height
        color: "transparent"
        clip: true

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
                width: parent.width
                height: defaultThumbImg.visible ? defaultThumbImg.height : thumbImg.height
                color: "transparent"

                ThumbImageView {
                    id: thumbImg
                    x: (parent.width - width) / 2
                    width: parent.width - 2
                    imageView: filter.imageVisible() ? videoView.imageView : filter.nullImage
                    fillMode: Image.PreserveAspectFit
                    enableAlt: !isFullViewMode

                    onHeightChanged: {
                        if (maxHeight && height > maxHeight)
                            Qt.callLater(setMaxHeight)
                    }

                    function setMaxHeight() {
                        const ratio = width / height
                        height = maxHeight
                        width = height * ratio
                    }
                }
                Rectangle {
                    id: defaultThumbImg
                    x: (parent.width - width) / 2
                    width: parent.width - 2
                    height: width / videoStack.getAspectRatio()
                    color: guiSettings.avatarDefaultColor
                    visible: videoView.imageView.isNull() || thumbImg.status != Image.Ready && filter.imageVisible()

                    onHeightChanged: {
                        if (maxHeight && height > maxHeight)
                            Qt.callLater(setMaxHeight)
                    }

                    function setMaxHeight() {
                        const ratio = width / height
                        height = maxHeight
                        width = height * ratio
                    }

                    SvgImage {
                        x: (parent.width - width) / 2
                        y: (parent.height - height) / 2 + height
                        width: Math.min(parent.width, 150)
                        height: width
                        color: "white"
                        svg: svgFilled.film
                    }
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
            svg: svgFilled.play
            visible: filter.imageVisible() && !videoPlayer.playing && !videoPlayer.restarting
            enabled: videoPlayer.hasVideo

            onClicked: videoPlayer.start()
        }

        Item {
            id: player
            width: parent.width
            height: parent.height

            MediaPlayer {
                property bool restarting: false

                id: videoPlayer
                source: videoView.playlistUrl
                videoOutput: videoOutput
                audioOutput: audioOutput

                onPlaybackStateChanged: {
                    if (playbackState == MediaPlayer.StoppedState) {
                        source = ""
                        source = videoView.playlistUrl
                    }
                }

                onPlayingChanged: {
                    if (videoPlayer.playing)
                        restartTimer.set(false)
                }

                onErrorOccurred: (error, errorString) => { console.debug("Video error:", error, errorString) }

                function playPause() {
                    if (playbackState == MediaPlayer.PausedState)
                        play()
                    else
                        pause()
                }

                function start() {
                    restartTimer.set(true)
                    play()
                }

                function restart() {
                    stopPlaying()
                    source = ""
                    source = videoView.playlistUrl
                    start()
                }

                function stopPlaying() {
                    stop()
                    restartTimer.set(false)
                }

                function isLoading() {
                    return [MediaPlayer.LoadingMedia,
                            MediaPlayer.BufferingMedia].includes(mediaStatus)
                }
            }
            VideoOutput {
                id: videoOutput
                anchors.fill: parent
            }
            AudioOutput {
                id: audioOutput
                muted: !userSettings.videoSound

                function toggleSound() {
                    userSettings.videoSound = !userSettings.videoSound
                }
            }

            BusyIndicator {
                anchors.centerIn: parent
                running: (videoPlayer.playing && videoPlayer.isLoading()) || videoPlayer.restarting
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
            onClicked: root.viewFullVideo(videoView)
            enabled: !isFullViewMode
        }

        RoundCornerMask {
            width: parent.width
            height: parent.height
            color: "transparent"
            maskColor: videoStack.backgroundColor
        }
    }

    Rectangle {
        id: playControls
        width: parent.width
        height: playPauseButton.height
        color: "transparent"
        visible: videoPlayer.playbackState == MediaPlayer.PlayingState || videoPlayer.playbackState == MediaPlayer.PausedState || videoPlayer.restarting

        SvgTransparentButton {
            id: playPauseButton
            width: 24
            height: width
            svg: videoPlayer.playing ? svgFilled.pause : svgFilled.play
            accessibleName: videoPlayer.playing ? qsTr("pause video") : qsTr("continue playing video")
            color: controlColor

            onClicked: videoPlayer.playPause()
        }

        SvgTransparentButton {
            id: stopButton
            anchors.left: playPauseButton.right
            anchors.leftMargin: 10
            width: 24
            height: width
            svg: svgFilled.stop
            accessibleName: qsTr("stop video")
            color: controlColor

            onClicked: videoPlayer.stopPlaying()
        }

        SvgTransparentButton {
            id: replayButton
            anchors.left: stopButton.right
            anchors.leftMargin: 10
            width: 24
            height: width
            svg: svgOutline.refresh
            accessibleName: qsTr("restart video")
            color: controlColor

            onClicked: videoPlayer.restart()
        }

        ProgressBar {
            id: playProgress
            anchors.left: replayButton.right
            anchors.leftMargin: 10
            anchors.right: remainingTimeText.left
            anchors.rightMargin: 10
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
        }

        MouseArea {
            x: playProgress.x
            width: playProgress.width
            height: playControls.height
            onClicked: (event) => {
                videoPlayer.position = (videoPlayer.duration / width) * event.x
            }
        }

        AccessibleText {
            id: remainingTimeText
            anchors.right: soundButton.left
            anchors.rightMargin: 10
            anchors.verticalCenter: parent.verticalCenter
            font.pointSize: guiSettings.scaledFont(6/8)
            text: guiSettings.videoDurationToString(videoPlayer.duration - videoPlayer.position)
        }

        SvgTransparentButton {
            id: soundButton
            anchors.right: parent.right
            width: 24
            height: width
            svg: audioOutput.muted ? svgOutline.soundOff : svgOutline.soundOn
            accessibleName: audioOutput.muted ? qsTr("turn sound on") : qsTr("turn sound off")
            color: controlColor

            onClicked: audioOutput.toggleSound()
        }
    }

    GuiSettings {
        id: guiSettings
    }

    function pause() {
        if (videoPlayer.playing)
            videoPlayer.pause()
    }

    function getAspectRatio() {
        if (videoView && videoView.width > 0 && videoView.height > 0)
            return videoView.width / videoView.height
        else
            return 16/9
    }
}
