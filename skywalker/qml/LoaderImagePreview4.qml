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

    id: images4Loader
    height: calcHeight()
    active: imageCount === 4 && postVisible
    asynchronous: true

    sourceComponent: ImagePreview4 {
        width: images4Loader.width
        height: images4Loader.height
        images: postImages
        startImageIndex: images4Loader.startImageIndex
        maskColor: bodyBackgroundColor
        contentVisibility: postContentVisibility
        contentWarning: postContentWarning
        contentLabeler: postContentLabeler
        swipeMode: images4Loader.swipeMode
        maxHeight: images4Loader.maxImageHeight

        onActivateSwipe: (imgIndex, previewImg) => images4Loader.activateSwipe(imgIndex, previewImg)
        onShowFullImage: (imgIndex, swipeMode) => images4Loader.showFullImage(imgIndex, swipeMode)
    }

    onStatusChanged: {
        if (status == Loader.Ready)
            active = true
    }

    LoaderCanvas {
        backgroundColor: bodyBackgroundColor
    }

    function calcHeight() {
        if (imageCount !== 4)
            return 0

        const filter = item ? item.getFilter() : null

        if (filter && !filter.imageVisible())
            return filter.height

        const maxWidth = maxImageHeight * 2
        return Math.min(width, maxWidth)
    }
}
