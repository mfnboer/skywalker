import QtQuick
import QtQuick.Controls
import skywalker

Item {
    required property string uri
    required property string title
    required property string description
    required property int contentVisibility // QEnums::ContentVisibility
    required property string contentWarning
    required property basicprofile contentLabeler
    property string backgroundColor: guiSettings.backgroundColor

    id: view
    width: parent.width
    height: Math.max(gifImage.height + (tenorAttribution.visible ? tenorAttribution.height : 0) + (giphyAttribution.visible ? giphyAttribution.height : 0) + 5, gifLoadingIndicator.height)

    AnimatedImagePreview {
        id: gifImage
        url: gifUtils.getGifUrl(view.uri)
        title: view.title
        description: view.description
        contentVisibility: view.contentVisibility
        contentWarning: view.contentWarning
        contentLabeler: view.contentLabeler
        backgroundColor: view.backgroundColor

        // Use the view URI as the url query params have been removed by getGifUrl
        imgSize: gifUtils.getGifSize(view.uri)
    }
    BusyIndicator {
        id: gifLoadingIndicator
        anchors.centerIn: parent
        running: gifImage.status === Image.Loading
    }
    Image {
        id: klipyAttribution
        anchors.left: gifImage.left
        anchors.leftMargin: 5
        anchors.bottom: gifImage.bottom
        anchors.bottomMargin: 5
        width: guiSettings.gifAttributionWidth
        height: guiSettings.gifAttributionHeight
        fillMode: Image.PreserveAspectFit
        source: "/images/klipy_watermark.svg"
        asynchronous: true
        visible: gifUtils.isKlipyLink(view.uri)
    }
    Image {
        id: tenorAttribution
        anchors.right: gifImage.right
        anchors.top: gifImage.bottom
        anchors.topMargin: 5
        width: guiSettings.gifAttributionWidth
        height: guiSettings.gifAttributionHeight
        fillMode: Image.PreserveAspectFit
        source: "/images/via_tenor_logo_blue.svg"
        asynchronous: true
        visible: gifUtils.isTenorLink(view.uri)
    }
    Image {
        id: giphyAttribution
        anchors.right: gifImage.right
        anchors.top: gifImage.bottom
        anchors.topMargin: 5
        width: guiSettings.gifAttributionWidth
        height: guiSettings.gifAttributionHeight
        fillMode: Image.PreserveAspectFit
        source: "/images/giphy_logo.png"
        asynchronous: true
        visible: gifUtils.isGiphyLink(view.uri)
    }

    GifUtils {
        id: gifUtils
    }

    function getFilter() {
        return gifImage.getFilter()
    }
}
