import QtQuick
import skywalker

LoaderVideoPreview {
    id: gifVideoLoader

    Loader {
        id: klipyAttribution
        anchors.right: gifVideoLoader.right
        anchors.rightMargin: 5
        anchors.bottom: gifVideoLoader.bottom
        anchors.bottomMargin: 5
        width: guiSettings.gifAttributionWidth
        height: guiSettings.gifAttributionHeight
        z: gifVideoLoader.item ? gifVideoLoader.item.z + 1 : 1
        active: gifUtils.isKlipyLink(postVideo.playlistUrl) && gifVideoLoader.item && gifVideoLoader.item.contentFilter.imageVisible()

        sourceComponent: Image {
            fillMode: Image.PreserveAspectFit
            source: "/images/klipy_watermark.svg"
            asynchronous: true
        }
    }

    GifUtils {
        id: gifUtils
    }
}
