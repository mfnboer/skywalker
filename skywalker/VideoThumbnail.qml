import QtQuick
import QtQuick.Controls
import QtMultimedia
import skywalker

Video {
    property string videoSource

    id: videoThumbnail
    fillMode: VideoOutput.PreserveAspectCrop
    source: videoSource

    onPlaying: pauseTimer.start()

    onErrorOccurred: (error, errorString) => console.debug("ERROR:", error, errorString)

    onVideoSourceChanged: {
        if (Boolean(videoSource))
            Qt.callLater(play)
    }

    SvgImage {
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
        onTriggered: videoThumbnail.pause()
    }
}
