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
    height: Math.max(gifImage.height, gifLoadingIndicator.height)

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
    GifAttribution {
        anchors.right: gifImage.right
        anchors.rightMargin: 5
        anchors.bottom: gifImage.bottom
        anchors.bottomMargin: 5
        gifUri: view.uri
        show: gifImage.getFilter().imageVisible()
    }

    GifUtils {
        id: gifUtils
    }

    function getFilter() {
        return gifImage.getFilter()
    }
}
