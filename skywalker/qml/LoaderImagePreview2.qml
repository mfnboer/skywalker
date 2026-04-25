import QtQuick
import skywalker

Loader {
    required property list<imageview> postImages
    property int startImageIndex: 0
    property int imageCount: 0
    required property int postContentVisibility // QEnums::PostContentVisibility
    required property string postContentWarning
    required property basicprofile postContentLabeler
    property string bodyBackgroundColor: guiSettings.backgroundColor
    property bool swipeMode: false
    property bool postVisible: true
    property int maxImageHeight: guiSettings.maxImageHeight

    signal activateSwipe(int imgIndex, var previewImg)
    signal showFullImage(int imgIndex, bool swipeMode)

    id: images2Loader
    height: calcHeight()
    active: imageCount === 2 && postVisible
    asynchronous: true

    sourceComponent: ImagePreview2 {
        width: images2Loader.width
        height: images2Loader.height
        images: postImages
        startImageIndex: images2Loader.startImageIndex
        maskColor: bodyBackgroundColor
        contentVisibility: postContentVisibility
        contentWarning: postContentWarning
        contentLabeler: postContentLabeler
        swipeMode: images2Loader.swipeMode
        maxHeight: images2Loader.maxImageHeight

        onActivateSwipe: (imgIndex, previewImg) => images2Loader.activateSwipe(imgIndex, previewImg)
        onShowFullImage: (imgIndex, swipeMode) => images2Loader.showFullImage(imgIndex, swipeMode)
    }

    onStatusChanged: {
        if (status == Loader.Ready)
            active = true
    }

    LoaderCanvas {
        backgroundColor: bodyBackgroundColor
    }

    function calcHeight() {
        if (imageCount !== 2)
            return 0

        const filter = item ? item.getFilter() : null

        if (filter && !filter.imageVisible())
            return filter.height

        const maxWidth = maxImageHeight * 2
        return Math.min(width, maxWidth) / 2
    }
}
