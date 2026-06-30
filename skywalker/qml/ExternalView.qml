import QtQuick
import skywalker

Item {
    property string userDid
    required property int contentVisibility // QEnums::ContentVisibility
    required property string contentWarning
    required property basicprofile contentLabeler
    property externalview postExternal
    property bool highlight: false
    property string maskColor: highlight ? guiSettings.postHighLightColor : guiSettings.backgroundColor
    property Skywalker skywalker: root.getSkywalker(userDid)
    readonly property bool isGif: gifUtils.isGif(postExternal.uri)
    readonly property bool isJoinLink: skywalker.chat.isJoinLinkUri(postExternal.uri)
    property bool moving: false

    id: view
    width: parent.width
    height: calcHeight()

    Accessible.role: Accessible.Link
    Accessible.name: getSpeech()
    Accessible.onPressAction: if (isLinkEnabled()) openExternalLink()

    Loader {
        readonly property int cardHeight: status == Loader.Ready ? item.columnHeight : 0 // qmllint disable missing-property

        id: cardLoader
        width: parent.width
        active: !isGif && !isJoinLink
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
            createdAt: postExternal.createdAt
            updatedAt: postExternal.updateAt
            readingTime: postExternal.readingTime
            externalSource: postExternal.source
            associatedProfiles: postExternal.associatedProfiles ? postExternal.associatedProfiles : []
            contentVisibility: view.contentVisibility
            contentWarning: view.contentWarning
            contentLabeler: view.contentLabeler
            maskColor: view.maskColor
            showSonglinkWidget: true
            moving: view.moving
        }
    }

    Loader {
        id: gifLoader
        width: parent.width
        height: calcHeight()
        active: isGif && !moving
        asynchronous: true

        sourceComponent: GifView {
            width: gifLoader.width
            height: gifLoader.height
            uri: postExternal.uri
            title: postExternal.title
            description: postExternal.description
            contentVisibility: view.contentVisibility
            contentWarning: view.contentWarning
            contentLabeler: view.contentLabeler
            backgroundColor: maskColor
        }

        onStatusChanged: {
            if (status == Loader.Ready)
                active = true
        }

        LoaderCanvas {
            backgroundColor: maskColor
        }

        function calcHeight() {
            if (!isGif)
                return 0

            const filter = item ? item.getFilter() : null

            if (filter && !filter.imageVisible())
                return filter.height

            const imgSize = gifUtils.getGifSize(postExternal.uri)
            const aspectRatio = (imgSize.width > 0 && imgSize.height > 0) ? imgSize.height / imgSize.width : 0.0

            const attributionHeight = (gifUtils.isTenorLink(postExternal.uri) || gifUtils.isGiphyLink(postExternal.uri)) ?
                                        guiSettings.gifAttributionHeight : 0

            if (aspectRatio <= 0.0)
                return Math.min(width, guiSettings.maxImageHeight) + attributionHeight

            let w = Math.min(width, imgSize.width)
            const h = w * aspectRatio

            if (h > guiSettings.maxImageHeight)
                w *= (guiSettings.maxImageHeight / h)

            return w * aspectRatio + attributionHeight
        }
    }

    Loader {
        id: joinLinkLoader
        width: parent.width
        active: isJoinLink

        sourceComponent: JoinLinkPreview {
            userDid: view.userDid
            width: parent.width
            uri: postExternal.uri
            title: postExternal.title
            maskColor: view.maskColor
        }
    }

    SkyMouseArea {
        anchors.fill: parent
        z: -1
        onClicked: openExternalLink()
        enabled: isLinkEnabled()
    }

    GifUtils {
        id: gifUtils
    }

    function calcHeight() {
        if (isGif)
            return gifLoader.height

        if (isJoinLink)
            return joinLinkLoader.height

        return cardLoader.cardHeight
    }

    function isLinkEnabled() {
        return !isGif && !isJoinLink
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
