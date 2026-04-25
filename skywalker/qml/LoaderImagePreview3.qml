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

    id: images3Loader
    height: calcHeight()
    active: imageCount === 3 && postVisible
    asynchronous: true

    sourceComponent: ImagePreview3 {
        width: images3Loader.width
        height: images3Loader.height
        images: postImages
        startImageIndex: images3Loader.startImageIndex
        maskColor: bodyBackgroundColor
        contentVisibility: postContentVisibility
        contentWarning: postContentWarning
        contentLabeler: postContentLabeler
        swipeMode: images3Loader.swipeMode
        maxHeight: images3Loader.maxImageHeight

        onActivateSwipe: (imgIndex, previewImg) => images3Loader.activateSwipe(imgIndex, previewImg)
        onShowFullImage: (imgIndex, swipeMode) => images3Loader.showFullImage(imgIndex, swipeMode)
    }

    onStatusChanged: {
        if (status == Loader.Ready)
            active = true
    }

    LoaderCanvas {
        backgroundColor: bodyBackgroundColor
    }

    function calcHeight() {
        if (imageCount !== 3)
            return 0

        const filter = item ? item.getFilter() : null

        if (filter && !filter.imageVisible())
            return filter.height

        const maxWidth = maxImageHeight * 1.5
        return Math.min(width, maxWidth) / 1.5
    }
}
