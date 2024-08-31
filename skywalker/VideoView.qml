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
                ImageAutoRetry {
                    id: thumbImg
                    x: 1
                    width: parent.width - 2
                    source: filter.imageVisible() ? videoView.thumbUrl : ""
                    fillMode: Image.PreserveAspectFit
                }
            }

            SvgButton {
                x: (parent.width - width) / 2
                y: (parent.height - height) / 2
                width: 70
                height: width
                imageMargin: 0
                opacity: 0.5
                accessibleName: qsTr("play video")
                svg: svgFilled.play
                visible: !videoPlayer.playing

                onClicked: {
                    videoPlayer.play()
                }
            }
        }

        Item {
            id: player
            width: parent.width
            height: parent.height

            MediaPlayer {
                id: videoPlayer
                source: videoView.playlistUrl
                videoOutput: videoOutput
                audioOutput: AudioOutput {}

                onPlaybackStateChanged: {
                    if (playbackState == MediaPlayer.StoppedState) {
                        source = ""
                        source = videoView.playlistUrl
                    }
                }

                onErrorOccurred: (error, errorString) => { console.debug("Video error:", error, errorString) }

                function playPause() {
                    if (playbackState == MediaPlayer.PausedState)
                        play()
                    else
                        pause()
                }
            }
            VideoOutput {
                id: videoOutput
                anchors.fill: parent
            }
            BusyIndicator {
                anchors.centerIn: parent
                running: videoPlayer.playing && videoPlayer.mediaStatus < MediaPlayer.BufferedMedia
            }
            AccessibleText {
                color: "white"
                text: `${(Math.round(videoPlayer.position / 1000))} / ${(Math.round(videoPlayer.duration / 1000))}`
            }
        }
    }

    Rectangle {
        anchors.top: player.bottom
        width: parent.width
        height: playPauseButton.height
        visible: videoPlayer.playbackState == MediaPlayer.PlayingState || videoPlayer.playbackState == MediaPlayer.PausedState

        SvgTransparentButton {
            id: playPauseButton
            width: 24
            height: width
            svg: videoPlayer.playing ? svgFilled.pause : svgFilled.play
            accessibleName: qsTr("pause video")
            color: guiSettings.textColor

            onClicked: videoPlayer.playPause()
        }
    }

    GuiSettings {
        id: guiSettings
    }
}
