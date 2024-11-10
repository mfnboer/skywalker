import QtQuick
import QtQuick.Controls
import skywalker

Item {
    required property string uri
    required property string title
    required property int contentVisibility // QEnums::ContentVisibility
    required property string contentWarning

    id: view
    height: Math.max(gifImage.height + tenorAttribution.height + 5, gifLoadingIndicator.height)

    AnimatedImagePreview {
        id: gifImage
        url: gifUtils.getGifUrl(view.uri)
        title: view.title
        contentVisibility: view.contentVisibility
        contentWarning: view.contentWarning
    }
    BusyIndicator {
        id: gifLoadingIndicator
        anchors.centerIn: parent
        running: gifImage.status === Image.Loading
    }
    Image {
        id: tenorAttribution
        anchors.right: gifImage.right
        anchors.top: gifImage.bottom
        anchors.topMargin: 5
        width: 50
        fillMode: Image.PreserveAspectFit
        source: "/images/via_tenor_logo_blue.svg"
        visible: gifUtils.isTenorLink(view.uri) && gifImage.status === Image.Ready
    }
    Image {
        id: giphyAttribution
        anchors.right: gifImage.right
        anchors.top: gifImage.bottom
        anchors.topMargin: 5
        width: 50
        fillMode: Image.PreserveAspectFit
        source: "/images/giphy_logo.png"
        visible: gifUtils.isGiphyLink(view.uri) && gifImage.status === Image.Ready
    }

    GifUtils {
        id: gifUtils
    }
}
