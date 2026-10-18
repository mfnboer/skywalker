import QtQuick
import skywalker

Item {
    required property string gifUri
    required property bool show
    width: guiSettings.gifAttributionWidth
    height: guiSettings.gifAttributionHeight

    Loader {
        id: klipyAttribution
        anchors.fill: parent
        active: show && gifUtils.isKlipyLink(gifUri)

        sourceComponent: Image {
            fillMode: Image.PreserveAspectFit
            source: "/images/klipy_watermark.svg"
            asynchronous: true
        }
    }
    Loader {
        id: tenorAttribution
        anchors.fill: parent
        active: show && gifUtils.isTenorLink(gifUri)

        sourceComponent: Image {
            fillMode: Image.PreserveAspectFit
            source: "/images/via_tenor_logo_blue.svg"
            asynchronous: true
        }
    }
    Loader {
        id: giphyAttribution
        anchors.fill: parent
        active: show && gifUtils.isGiphyLink(gifUri)

        sourceComponent: Image {
            fillMode: Image.PreserveAspectFit
            source: "/images/giphy_logo.png"
            asynchronous: true
        }
    }
}
