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

    id: images1Loader
    height: calcHeight()
    active: postImages.length === 1 && postVisible
    asynchronous: true

    sourceComponent: ImagePreview1 {
        width: images1Loader.width
        height: images1Loader.height
        images: postImages
        maskColor: bodyBackgroundColor
        contentVisibility: postContentVisibility
        contentWarning: postContentWarning
        contentLabeler: postContentLabeler
        swipeMode: images1Loader.swipeMode

        onActivateSwipe: (imgIndex, previewImg) => images1Loader.activateSwipe(imgIndex, previewImg)
    }

    function calcHeight() {
        if (postImages.length !== 1)
            return 0

        const filter = item ? item.getFilter() : null

        if (filter && !filter.imageVisible())
            return filter.height

        const image = postImages[0]
        const imgSizeKnown = image.width > 0 && image.height > 0

        if (imgSizeKnown) {
            const idealHeight = (image.height / image.width) * width
            const exceedsMaxHeight = guiSettings.maxImageHeight > 0 && idealHeight > guiSettings.maxImageHeight
            return exceedsMaxHeight ? guiSettings.maxImageHeight : idealHeight
        }

        return width
    }
}
