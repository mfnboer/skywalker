import QtQuick
import QtQuick.Controls
import QtMultimedia
import skywalker

Video {
    id: videoAttachement
    fillMode: VideoOutput.PreserveAspectCrop

    onPlaying: pauseTimer.start()

    onErrorOccurred: (error, errorString) => console.debug("ERROR:", error, errorString)

    function remove() {
        postUtils.dropVideo(video)
        video = ""
        altText = ""
    }

    function hasAltText() {
        return Boolean(altText)
    }

    // Play video for 50ms to get a still frame showing
    Timer {
        id: pauseTimer
        interval: 50
        onTriggered: videoAttachement.pause()
    }

    SkyButton {
        height: 34
        flat: videoAttachement.hasAltText()
        text: videoAttachement.hasAltText() ? qsTr("ALT") : qsTr("+ALT", "add alternative text button")
        onClicked: videoAttachement.editAltText()
        Accessible.name: imageScroller.hasAltText(index) ? qsTr(`edit alt text for picture ${(index + 1)}`) : qsTr(`add alt text to picture ${(index + 1)}`)
    }

    SvgButton {
        x: parent.width - width
        width: 34
        height: width
        svg: svgOutline.close
        accessibleName: qsTr("remove video")
        onClicked: videoAttachement.remove()
    }
}
