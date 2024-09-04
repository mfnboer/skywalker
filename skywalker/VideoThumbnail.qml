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

    // Play video for 200ms to get a still frame showing
    // I did 50ms first. That does not always work.
    Timer {
        id: pauseTimer
        interval: 200
        onTriggered: videoThumbnail.pause()
    }
}
