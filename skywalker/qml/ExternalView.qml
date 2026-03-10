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
            maskColor: view.maskColor
            showSonglinkWidget: true
        }
    }

    Loader {
        id: gifLoader
        width: parent.width
        height: calcHeight()
        active: isGif
        asynchronous: true
        visible: status == Loader.Ready

        sourceComponent: GifView {
            width: gifLoader.width
            height: gifLoader.height
            uri: postExternal.uri
            title: postExternal.title
            contentVisibility: view.contentVisibility
            contentWarning: view.contentWarning
            contentLabeler: view.contentLabeler
        }

        function calcHeight() {
            if (!isGif)
                return 0

            const filter = item ? item.getFilter() : null

            if (filter && !filter.imageVisible())
                return filter.height

            const imgSize = gifUtils.getGifSize(postExternal.uri)
            const aspectRatio = (imgSize.width > 0 && imgSize.height > 0) ? imgSize.height / imgSize.width : 0.0

            if (aspectRatio <= 0.0)
                return Math.min(width, guiSettings.maxImageHeight) + guiSettings.gifAttributionHeight

            let w = Math.min(width, imgSize.width)
            const h = w * aspectRatio

            if (h > guiSettings.maxImageHeight)
                w *= (guiSettings.maxImageHeight / h)

            return w * aspectRatio + guiSettings.gifAttributionHeight
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
