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

    id: images3Loader
    height: calcHeight()
    active: postImages.length === 3 && postVisible
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

    function calcHeight() {
        if (postImages.length !== 3)
            return 0

        const filter = item ? item.getFilter() : null

        if (filter && !filter.imageVisible())
            return filter.height

        const maxWidth = guiSettings.maxImageHeight * 1.5
        return Math.min(width, guiSettings.maxWidth) / 1.5
    }
}
