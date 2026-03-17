import QtQuick
import skywalker

Loader {
    required property list<imageview> postImages
    required property int postContentVisibility // QEnums::PostContentVisibility
    required property string postContentWarning
    required property basicprofile postContentLabeler
    property string bodyBackgroundColor: guiSettings.backgroundColor
    property bool swipeMode: false
    property bool postVisible: true
    property bool moving: false

    signal activateSwipe(int imgIndex, var previewImg)

    id: images3Loader
    height: calcHeight()
    active: postImages.length === 3 && postVisible && !moving
    asynchronous: true

    sourceComponent: ImagePreview3 {
        width: images3Loader.width
        height: images3Loader.height
        images: postImages
        maskColor: bodyBackgroundColor
        contentVisibility: postContentVisibility
        contentWarning: postContentWarning
        contentLabeler: postContentLabeler
        swipeMode: images3Loader.swipeMode

        onActivateSwipe: (imgIndex, previewImg) => images3Loader.activateSwipe(imgIndex, previewImg)
    }

    onStatusChanged: {
        if (status == Loader.Ready)
            active = true
    }

    LoaderCanvas {
        backgroundColor: bodyBackgroundColor
    }

    function calcHeight() {
        if (postImages.length !== 3)
            return 0

        const filter = item ? item.getFilter() : null

        if (filter && !filter.imageVisible())
            return filter.height

        const maxWidth = guiSettings.maxImageHeight * 1.5
        return Math.min(width, maxWidth) / 1.5
    }
}
