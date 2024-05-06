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
    height: gifUtils.isGif(postExternal.uri) ?
                Math.max(gifImage.height + tenorAttribution.height + 5, gifLoadingIndicator.height) :
                card.columnHeight

    Accessible.role: Accessible.Link
    Accessible.name: getSpeech()
    Accessible.onPressAction: if (isLinkEnabled()) openExternalLink()

    LinkCardView {
        id: card
        anchors.fill: parent
        uri: postExternal.uri
        title: postExternal.title
        description: postExternal.description
        thumbUrl: postExternal.thumbUrl
        contentVisibility: view.contentVisibility
        contentWarning: view.contentWarning
        visible: !gifUtils.isGif(postExternal.uri)
    }
    AnimatedImagePreview {
        id: gifImage
        url: gifUtils.getGifUrl(postExternal.uri)
        title: url ? postExternal.title : ""
        contentVisibility: view.contentVisibility
        contentWarning: view.contentWarning
        visible: url
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

    MouseArea {
        anchors.fill: parent
        z: -1
        cursorShape: Qt.PointingHandCursor
        onClicked: openExternalLink()
        enabled: isLinkEnabled()
    }

    GifUtils {
        id: gifUtils
    }

    function isLinkEnabled() {
        return !gifUtils.isGif(postExternal.uri)
    }

    function openExternalLink() {
        root.openLink(postExternal.uri)
    }

    function getSpeech() {
        if (gifUtils.isGif(postExternal.uri))
            return qsTr(`GIF image: ${postExternal.title}`)

        const hostname = new URL(postExternal.uri).hostname
        return qsTr("link card: ") + postExternal.title + "\n\nfrom: " + hostname + "\n\n" + postExternal.description
    }
}
