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
        url: isGifImage() ? getGifUrl() : ""
        title: isGifImage() ? postExternal.title : ""
        visible: isGifImage() && filter.imageVisible()
    }
    Image {
        id: tenorAttribution
        anchors.right: gifImage.right
        anchors.top: gifImage.bottom
        anchors.topMargin: 5
        width: 50
        fillMode: Image.PreserveAspectFit
        source: "/images/via_tenor_logo_blue.svg"
        visible: gifImage.visible && isTenorGif() && gifImage.status === Image.Ready
    }
    Image {
        id: giphyAttribution
        anchors.right: gifImage.right
        anchors.top: gifImage.bottom
        anchors.topMargin: 5
        width: 50
        fillMode: Image.PreserveAspectFit
        source: "/images/giphy_logo.png"
        visible: gifImage.visible && isGiphyGif() && gifImage.status === Image.Ready
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
        enabled: !isGifImage() && filter.imageVisible()
    }

    function isTenorGif() {
        return postExternal.uri.startsWith("https://media.tenor.com/") ||
                postExternal.uri.startsWith("https://tenor.com/") ||
                postExternal.uri.startsWith("https://graysky.app/gif/")
    }

    function isGiphyGif() {
        return postExternal.uri.startsWith("https://giphy.com/gifs/") ||
                postExternal.uri.startsWith("https://media.giphy.com/media/")
    }

    function isGifImage() {
        if (postExternal.uri.startsWith("https://media.tenor.com/")) {
            if (postExternal.uri.endsWith(".gif")) {
                return true
            }

            return getGifUrl() !== ""
        }

        if (postExternal.uri.startsWith("https://graysky.app/gif/")) {
            return getGifUrl() !== ""
        }

        if (postExternal.uri.startsWith("https://tenor.com/")) {
            return getGifUrl() !== ""
        }

        if (postExternal.uri.startsWith("https://giphy.com/gifs/")) {
            return getGifUrl() !== ""
        }

        if (postExternal.uri.startsWith("https://media.giphy.com/media/")) {
            return getGifUrl() !== ""
        }

        return false
    }

    function getGifUrl() {
        let url = postExternal.uri
        console.debug("Get GIF url for:", url)

        if (url.startsWith("https://giphy.com/gifs/"))
            return getGiphyGifUrl()

        if (url.startsWith("https://media.giphy.com/media/"))
            return getGiphyMediaGifUrl()

        if (url.startsWith("https://tenor.com/"))
            return getTenorViewGif()

        if (url.startsWith("https://graysky.app/gif/")) {
            url = postExternal.uri.split("?")[0]
            url = "https://media.tenor.com" + url.slice(23)
            console.debug("Extracted Tenor URL from Graysky:", url)
        }

        // Qt6 has a bug that blocks playing video from a https source.
        // As a workaround we convert the MP4 url from Tenor to GIF (this may not work forever)
        // These are examples of Tenor URL's:
        //
        // https://media.tenor.com/2w1XsfvQD5kAAAPo/hhgf.mp4 (normal MP4 format)
        // https://media.tenor.com/2w1XsfvQD5kAAAAd/hhgf.gif (medium GIF format)
        //
        // The last 2 characters of the TenorId, e.g. 2w1XsfvQD5kAAAPo indicates the format.
        //
        // Po = normal MP4
        // Ad = medium GIF

        if (url.endsWith(".mp4") || url.endsWith(".webm")) {
            console.debug("Convert video URL to gif URL:", url)
            let urlParts = url.split("/")

            if (urlParts.length < 4) {
                console.warn("Unknow URL format:", url)
                return ""
            }

            const tenorId = urlParts[urlParts.length - 2]

            if (tenorId.length < 3) {
                console.warn("Unknown Tenor ID:", url)
                return ""
            }

            const baseTenorId = tenorId.slice(0, tenorId.length - 2)
            const fileName = urlParts[urlParts.length - 1]
            const baseFileName = fileName.split(".")[0]

            urlParts[urlParts.length - 2] = baseTenorId + "Ad"
            urlParts[urlParts.length - 1] = baseFileName + ".gif"
            url = urlParts.join("/")

            console.debug("Converted video URL to gif URL:", url)
        }

        if (url.endsWith(".gif"))
            return url;

        console.debug("Unsupported GIF format:", url)
        return ""
    }

    function getGiphyGifUrl() {
        let url = postExternal.uri
        console.debug("Get Giphy GIF url for:", url)

        let urlParts = url.split("/")

        if (urlParts.length !== 5) {
            console.warn("Unknown Giphy URL format:", url)
            return ""
        }

        let gifLongId = urlParts[urlParts.length - 1]
        let gifLongIdParts = gifLongId.split("-")
        let gifId = gifLongIdParts[gifLongIdParts.length - 1]
        url = "https://i.giphy.com/" + gifId + ".gif"
        console.debug("Giphy GIF url:", url)
        return url
    }

    function getGiphyMediaGifUrl() {
        let url = postExternal.uri
        console.debug("Get Giphy media GIF url for:", url)

        if (url.endsWith(".gif"))
            return url

        return ""
    }

    function getTenorViewGif() {
        let url = postExternal.uri
        console.debug("Get Tenor view GIF url for:", url)

        let urlParts = url.split("/")

        if (urlParts.length < 5) {
            console.warn("Unknown Tenor view format:", url)
            return ""
        }

        if (urlParts[3] !== "view" && urlParts[4] !== "view") {
            console.warn("Not a Tenor view format:", url)
            return ""
        }

        let queryIndex = url.indexOf("?")

        if (queryIndex !== -1)
            url = url.slice(0, queryIndex)

        url = url + ".gif"
        console.debug("Tenor view GIF url:", url)
        return url
    }
}
