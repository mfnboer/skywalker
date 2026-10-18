import QtQuick
import skywalker

Loader {
    property var postVideo // videoView
    required property int postContentVisibility // QEnums::PostContentVisibility
    required property string postContentWarning
    required property basicprofile postContentLabeler
    property string bodyBackgroundColor: guiSettings.backgroundColor
    property bool swipeMode: false
    property bool isDraft: false
    property bool postVisible: true
    property bool moving: false
    property bool hidden: false
    readonly property int margin: 10

    signal activateSwipe(int imgIndex, var previewImg)

    id: videoLoader
    x: swipeMode ? -margin - 1 : 0
    width: calcWidth()
    height: calcHeight() + (item ? item.extraInlineHeight : 0)
    active: Boolean(postVideo) && postVisible && !moving && !hidden
    asynchronous: true

    sourceComponent: isDraft ? videoThumbnailComponent : videoViewComponent

    onStatusChanged: {
        if (status == Loader.Ready)
            active = Qt.binding(() => !videoLoader.hidden)
    }

    LoaderCanvas {
        backgroundColor: bodyBackgroundColor
    }

    function calcWidth() {
        if (isDraft)
            return Math.min(guiSettings.draftImageHeight * 1.777, parent.width)

        if (postVideo && (gifUtils.isKlipyLink(postVideo.playlistUrl) || gifUtils.isTenorLink(postVideo.playlistUrl)) && postVideo.width > 0)
            return Math.min(postVideo.width, parent.width)

        return parent.width + (swipeMode ? 2 * margin + 2 : 0)
    }

    function calcHeight() {
        if (!postVideo)
            return 0

        if (isDraft)
            return guiSettings.draftImageHeight

        const filter = item ? item.getFilter() : null

        if (filter && !filter.imageVisible())
            return filter.height + 20

        const videoSizeIsKnown = postVideo.width > 0 && postVideo.height > 0
        const aspectRatio = videoSizeIsKnown ? postVideo.width / postVideo.height : (1 / guiSettings.videoPreviewRatio)
        const maxHeight = guiSettings.maxImageHeight
        const maxWidth = maxHeight * aspectRatio
        const image = postVideo.imageView

        if (image.isNull() || image.width <= 0 || image.height <= 0) {
            const w = (maxWidth > 0 && videoLoader.width > maxWidth) ? maxWidth : videoLoader.width
            return w / aspectRatio
        }

        const idealHeight = (image.height / image.width) * width
        const exceedsMaxHeight = guiSettings.maxImageHeight > 0 && idealHeight > guiSettings.maxImageHeight
        return exceedsMaxHeight ? guiSettings.maxImageHeight : idealHeight
    }

    GifUtils {
        id: gifUtils
    }

    Component {
        id: videoThumbnailComponent

        VideoThumbnail {
            width: videoLoader.width
            height: videoLoader.height
            videoSource: postVideo.playlistUrl
        }
    }

    Component {
        id: videoViewComponent

        VideoView {
            id: videoViewItem
            width: videoLoader.width
            height: videoLoader.height
            videoView: postVideo
            contentVisibility: postContentVisibility
            contentWarning: postContentWarning
            contentLabeler: postContentLabeler
            backgroundColor: bodyBackgroundColor
            highlight: bodyBackgroundColor === guiSettings.postHighLightColor
            swipeMode: videoLoader.swipeMode

            onActivateSwipe: (imgIndex, previewImg) => videoLoader.activateSwipe(imgIndex, previewImg)
        }
    }
}
