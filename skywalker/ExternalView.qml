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
    height: filter.imageVisible() ? (isGifImage() ? gifImage.height + tenorAttribution.height : card.columnHeight)
                                  : filter.height

    LinkCardView {
        id: card
        anchors.fill: parent
        uri: postExternal.uri
        title: postExternal.title
        description: postExternal.description
        thumbUrl: postExternal.thumbUrl
        visible: !isGifImage() && filter.imageVisible()
    }
    AnimatedImagePreview {
        id: gifImage
        contentVisibility: view.contentVisibility
        contentWarning: view.contentWarning
        url: isGifImage() ? postExternal.uri : ""
        title: isGifImage() ? postExternal.title : ""
        visible: isGifImage() && filter.imageVisible()
    }
    Image {
        id: tenorAttribution
        anchors.right: parent.right
        anchors.top: gifImage.bottom
        anchors.topMargin: 5
        width: 80
        fillMode: Image.PreserveAspectFit
        source: "/images/via_tenor_logo_blue.svg"
        visible: isGifImage()
    }

    FilteredImageWarning {
        id: filter
        width: parent.width - 2
        contentVisibiliy: view.contentVisibility
        contentWarning: view.contentWarning
        images: [postExternal.thumbUrl]
    }
    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: root.openLink(postExternal.uri)
        enabled: !isGifImage() && filter.imageVisible()
    }

    function isGifImage() {
        return postExternal.uri.startsWith("https://media.tenor.com/") &&
               postExternal.uri.endsWith(".gif")
    }
}
