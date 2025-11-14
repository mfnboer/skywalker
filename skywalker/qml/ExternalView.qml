import QtQuick
import skywalker

Item {
    property string userDid
    required property int contentVisibility // QEnums::ContentVisibility
    required property string contentWarning
    required property basicprofile contentLabeler
    property externalview postExternal
    property bool highlight: false
    property string borderColor: highlight ? guiSettings.borderHighLightColor : guiSettings.borderColor
    property string maskColor: highlight ? guiSettings.postHighLightColor : guiSettings.backgroundColor
    readonly property bool isGif: gifUtils.isGif(postExternal.uri)

    id: view
    width: parent.width
    height: isGif ? gifLoader.height : cardLoader.cardHeight

    Accessible.role: Accessible.Link
    Accessible.name: getSpeech()
    Accessible.onPressAction: if (isLinkEnabled()) openExternalLink()

    Loader {
        readonly property int cardHeight: status == Loader.Ready ? item.columnHeight : 0 // qmllint disable missing-property

        id: cardLoader
        width: parent.width
        active: !isGif
        visible: status == Loader.Ready

        sourceComponent: LinkCardView {
            width: parent.width
            userDid: view.userDid
            uri: postExternal.uri
            title: postExternal.title
            titleIsHtml: postExternal.hasHtmlTitle()
            description: postExternal.description
            descriptionIsHtml: postExternal.hasHtmlDescription()
            thumbUrl: postExternal.thumbUrl
            contentVisibility: view.contentVisibility
            contentWarning: view.contentWarning
            contentLabeler: view.contentLabeler
            borderColor: view.borderColor
            maskColor: view.maskColor
            showSonglinkWidget: true
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
            contentLabeler: view.contentLabeler
        }
    }

    MouseArea {
        anchors.fill: parent
        z: -1
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
        root.openLink(postExternal.uri, "", userDid)
    }

    function getSpeech() {
        if (isGif)
            return qsTr(`GIF image: ${postExternal.title}`)

        const hostname = new URL(postExternal.uri).hostname
        return qsTr("link card: ") + postExternal.title + "\n\nfrom: " + hostname + "\n\n" + postExternal.description
    }
}
