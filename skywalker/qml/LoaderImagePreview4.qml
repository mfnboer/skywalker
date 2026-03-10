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

    signal activateSwipe(int imgIndex, var previewImg)

    id: images4Loader
    height: calcHeight()
    active: postImages.length === 4 && postVisible
    asynchronous: true

    sourceComponent: ImagePreview4 {
        width: images4Loader.width
        height: images4Loader.height
        images: postImages
        maskColor: bodyBackgroundColor
        contentVisibility: postContentVisibility
        contentWarning: postContentWarning
        contentLabeler: postContentLabeler
        swipeMode: images4Loader.swipeMode

        onActivateSwipe: (imgIndex, previewImg) => images4Loader.activateSwipe(imgIndex, previewImg)
    }

    function calcHeight() {
        if (postImages.length !== 4)
            return 0

        const filter = item ? item.getFilter() : null

        if (filter && !filter.imageVisible())
            return filter.height

        const maxWidth = guiSettings.maxImageHeight * 2
        return Math.min(width, maxWidth)
    }
}
