import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Item {
    required property int contentVisibility // QEnums::ContentVisibility
    required property string contentWarning
    property externalview postExternal
    readonly property bool isGif: gifUtils.isGif(postExternal.uri)

    id: view
    width: parent.width
    height: isGif ? gifLoader.height : cardLoader.cardHeight

    Accessible.role: Accessible.Link
    Accessible.name: getSpeech()
    Accessible.onPressAction: if (isLinkEnabled()) openExternalLink()

    Loader {
        readonly property int cardHeight: status == Loader.Ready ? item.columnHeight : 0

        id: cardLoader
        width: parent.width
        active: !isGif
        visible: status == Loader.Ready

        sourceComponent: LinkCardView {
            anchors.fill: parent
            uri: postExternal.uri
            title: postExternal.title
            description: postExternal.description
            thumbUrl: postExternal.thumbUrl
            contentVisibility: view.contentVisibility
            contentWarning: view.contentWarning
        }
    }

    Loader {
        id: gifLoader
        width: parent.width
        active: isGif
        visible: status == Loader.Ready

        sourceComponent: GifView {
            width: parent.width
            uri: postExternal.uri
            title: postExternal.title
            contentVisibility: view.contentVisibility
            contentWarning: view.contentWarning
        }
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
        return !isGif
    }

    function openExternalLink() {
        root.openLink(postExternal.uri)
    }

    function getSpeech() {
        if (isGif)
            return qsTr(`GIF image: ${postExternal.title}`)

        const hostname = new URL(postExternal.uri).hostname
        return qsTr("link card: ") + postExternal.title + "\n\nfrom: " + hostname + "\n\n" + postExternal.description
    }
}
