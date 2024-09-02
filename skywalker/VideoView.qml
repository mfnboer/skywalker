import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia
import skywalker

Column {
    required property var videoView // videoView
    required property int contentVisibility // QEnums::ContentVisibility
    required property string contentWarning

    id: videoStack

    Rectangle {
        width: parent.width
        height: videoPreview.height
        color: "transparent"

        RoundedFrame {
            id: videoPreview
            width: parent.width
            objectToRound: videoColumn
            border.width: 1
            border.color: guiSettings.borderColor

            FilteredImageWarning {
                id: filter
                width: parent.width - 2
                contentVisibiliy: videoStack.contentVisibility
                contentWarning: videoStack.contentWarning
                imageUrl: videoView.thumbUrl
            }

            Column {
                id: videoColumn
                width: parent.width
                topPadding: 1
                spacing: 3

                // HACK: The filter should be in this place, but inside a rounded object links
                // cannot be clicked.
                Rectangle {
                    width: filter.width
                    height: filter.height
                    color: "transparent"
                }
                ThumbImageView {
                    id: thumbImg
                    x: 1
                    width: parent.width - 2
                    imageView: filter.imageVisible() ? videoView.imageView : filter.nullImage
                    fillMode: Image.PreserveAspectFit
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
                visible: !videoPlayer.playing && !videoPlayer.restarting

                onClicked: videoPlayer.start()
            }
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
                    if (playing)
                        restarting = false
                }

                onErrorOccurred: (error, errorString) => { console.debug("Video error:", error, errorString) }

                function playPause() {
                    if (playbackState == MediaPlayer.PausedState)
                        play()
                    else
                        pause()
                }

                function start() {
                    restarting = true
                    play()
                }

                function restart() {
                    restarting = true
                    stop()
                    source = ""
                    source = videoView.playlistUrl
                    play()
                }
            }
            VideoOutput {
                id: videoOutput
                anchors.fill: parent
            }
            AudioOutput {
                id: audioOutput

                function toggleSound() {
                    muted = !muted
                }
            }

            BusyIndicator {
                anchors.centerIn: parent
                running: (videoPlayer.playing && videoPlayer.mediaStatus < MediaPlayer.BufferedMedia) || videoPlayer.restarting
            }
        }
    }

    Rectangle {
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
            color: guiSettings.textColor

            onClicked: videoPlayer.playPause()
        }

        SvgTransparentButton {
            id: replayButton
            anchors.left: playPauseButton.right
            anchors.leftMargin: 10
            width: 24
            height: width
            svg: svgOutline.refresh
            accessibleName: qsTr("restart video")
            color: guiSettings.textColor

            onClicked: videoPlayer.restart()
        }

        AccessibleText {
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
            color: guiSettings.textColor

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
}
