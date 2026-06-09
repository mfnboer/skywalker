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
    width: parent.width
    height: guiSettings.mustShowImageCarousel(postImages.length) ? calcImageHeight() : 0
    active: guiSettings.mustShowImageCarousel(postImages.length) && postVisible && !moving

    sourceComponent: ImageCarouselPreview {
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

    function calcImageHeight() {
        return Math.min(width * guiSettings.carouselImageRatio, guiSettings.maxImageCarouselHeight)
    }
}
