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
    property int maxImageHeight: guiSettings.maxImageHeight

    signal activateSwipe(int imgIndex, var previewImg)

    id: loader
    height: calcHeight() + (postImages.length > guiSettings.maxPreviewImageGridSize ? guiSettings.appFontHeight + 20 : 0)
    active: postImages.length > 0 && postVisible && !moving

    sourceComponent: ImageGridPreview {
        images: postImages
        contentVisibility: postContentVisibility
        contentWarning: postContentWarning
        contentLabeler: postContentLabeler
        bodyBackgroundColor: postBody.bodyBackgroundColor
        swipeMode: loader.swipeMode
        postVisible: postVisible

        onActivateSwipe: (imgIndex, previewImg) => postBody.activateSwipe(imgIndex, previewImg)
    }

    onStatusChanged: {
        // Avoid unloading when moving becomes true again
        if (status == Loader.Ready)
            active = true
    }

    function calcHeight() {
        const imgCount = postImages.length

        if (imgCount === 0)
            return 0

        const filter = item ? item.getFilter() : null

        if (filter && !filter.imageVisible())
            return filter.height

        if (imgCount === 1)
            return calcHeight1()

        if (imgCount === 2)
            return calcHeight2()

        if (imgCount === 3)
            return calcHeight3()

        return calcHeight4()
    }

    function calcHeight1() {
        const image = postImages[0]
        const imgSizeKnown = image.width > 0 && image.height > 0

        if (imgSizeKnown) {
            const idealHeight = (image.height / image.width) * width
            const exceedsMaxHeight = maxImageHeight > 0 && idealHeight > maxImageHeight
            return exceedsMaxHeight ? maxImageHeight : idealHeight
        }

        return Math.min(width, maxImageHeight)
    }

    function calcHeight2() {
        const maxWidth = maxImageHeight * 2
        return Math.min(width, maxWidth) / 2
    }

    function calcHeight3() {
        const maxWidth = maxImageHeight * 1.5
        return Math.min(width, maxWidth) / 1.5
    }

    function calcHeight4() {
        const maxWidth = maxImageHeight * 2
        return Math.min(width, maxWidth)
    }
}
