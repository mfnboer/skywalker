import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Item {
    required property int contentVisibility // QEnums::ContentVisibility
    required property string contentWarning
    property externalview postExternal

    id: view
    width: parent.width
    height: filter.imageVisible() ? (gifUtils.isGif(postExternal.uri) ? gifImage.height + tenorAttribution.height : card.columnHeight)
                                  : filter.height

    LinkCardView {
        id: card
        anchors.fill: parent
        uri: postExternal.uri
        title: postExternal.title
        description: postExternal.description
        thumbUrl: postExternal.thumbUrl
        visible: !gifUtils.isGif(postExternal.uri) && filter.imageVisible()
    }
    AnimatedImagePreview {
        id: gifImage
        contentVisibility: view.contentVisibility
        contentWarning: view.contentWarning
        url: gifUtils.getGifUrl(postExternal.uri)
        title: url ? postExternal.title : ""
        visible: url && filter.imageVisible()
    }
    Image {
        id: tenorAttribution
        anchors.right: gifImage.right
        anchors.top: gifImage.bottom
        anchors.topMargin: 5
        width: 50
        fillMode: Image.PreserveAspectFit
        source: "/images/via_tenor_logo_blue.svg"
        visible: gifImage.visible && gifUtils.isTenorLink(postExternal.uri) && gifImage.status === Image.Ready
    }
    Image {
        id: giphyAttribution
        anchors.right: gifImage.right
        anchors.top: gifImage.bottom
        anchors.topMargin: 5
        width: 50
        fillMode: Image.PreserveAspectFit
        source: "/images/giphy_logo.png"
        visible: gifImage.visible && gifUtils.isGiphyLink(postExternal.uri) && gifImage.status === Image.Ready
    }

    FilteredImageWarning {
        id: filter
        width: parent.width - 2
        contentVisibiliy: view.contentVisibility
        contentWarning: view.contentWarning
        imageUrl: postExternal.thumbUrl
    }
    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: root.openLink(postExternal.uri)
        enabled: !gifUtils.isGif(postExternal.uri) && filter.imageVisible()
    }

    GifUtils {
        id: gifUtils
    }
}
