import QtQuick
import QtQuick.Controls
import QtMultimedia
import skywalker

Video {
    property string videoSource
    property int videoStartMs: 0

    id: videoThumbnail
    fillMode: VideoOutput.PreserveAspectCrop
    source: videoSource
    muted: true

    onPlaying: pauseTimer.start()

    onErrorOccurred: (error, errorString) => console.debug("ERROR:", source, error, errorString)

    onVideoSourceChanged: {
        if (Boolean(videoSource))
            Qt.callLater(play)
    }

    onVideoStartMsChanged: videoThumbnail.position = videoStartMs

    SkySvg {
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2 + height
        width: 50
        height: width
        color: "white"
        outlineColor: "black"
        opacity: 0.6
        svg: svgFilled.film
    }

    // Play video for 200ms to get a still frame showing
    // I did 50ms first. That does not always work.
    Timer {
        id: pauseTimer
        interval: 200
        onTriggered: {
            videoThumbnail.pause()
            videoThumbnail.position = videoStartMs
        }
    }
}
