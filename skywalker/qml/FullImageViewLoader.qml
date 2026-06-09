import QtQuick
import QtQuick.Controls
import skywalker

Loader {
    required property list<var> thumbImageViewList
    property list<point> thumbImageOrigList
    property list<imageview> images
    property int gridWidth: 0
    property int maxGridSize: guiSettings.maxPreviewImageGridSize
    property int startGridIndex: 0
    property int startImageIndex: 0
    property int imageIndex: 0
    property bool swipeMode: false
    property bool reverse: false
    property bool noHeader: false
    property var animationStartCb
    property var videoView
    property bool isAnimatedImage: false
    property string animatedImageAlt
    readonly property int thumbImageIndex: imageIndex - startImageIndex

    signal started
    signal finished(int imgIndex)
    signal goingBackTo(int imgIndex)
    signal activateSwipe(int imgIndex, var img)

    id: fullImageLoader
    active: false

    sourceComponent: AnimateToFullImage {
        property var startCb: animationStartCb

        id: animation
        thumbImage: thumbImageViewList[thumbImageIndex]
        thumbImageOrig: thumbImageIndex < thumbImageOrigList.length ? thumbImageOrigList[thumbImageIndex] : Qt.point(0, 0)
        imageAlt: isAnimatedImage ? animatedImageAlt : images[imageIndex].alt
        swipeMode: fullImageLoader.swipeMode
        reverse: fullImageLoader.reverse
        noHeader: fullImageLoader.noHeader

        onStarted: {
            fullImageLoader.started()

            if (startCb) {
                startCb()
                startCb = null
            }
        }

        onDone: (fullImg) => {
            if (swipeMode) {
                fullImageLoader.activateSwipe(imageIndex, fullImg)
                fullImageLoader.active = false
            } else if (videoView) {
                let imgAnimation = animation
                root.viewFullVideo(videoView, fullImg, imgAnimation.reverseRun)
            } else if (isAnimatedImage) {
                let imgAnimation = animation
                root.viewFullAnimatedImage(thumbImageViewList[thumbImageIndex].url, animatedImageAlt, fullImg, imgAnimation.reverseRun)
            } else {
                let imgAnimation = animation
                root.viewFullImage(images, imageIndex, fullImg, imgAnimation.goBack)
            }
        }

        onReverseDone: {
            finished(imageIndex)
            fullImageLoader.active = false
        }

        function goBack(imgIndex, closeCb) {
            fullImageLoader.goingBackTo(imgIndex)
            startCb = closeCb
            imageIndex = imgIndex
            reverseRun()
        }
    }

    function initThumbImages() {
        thumbImageOrigList = []

        // When the user scrolls through the full images, then the image to shrink
        // can be on another grid than the grid from which the zoom started.
        // We calculate the offset in the x-coordinate to make sure the image shrinks to
        // the position of the new grid.
        for (const [thumbIndex, thumbImage] of thumbImageViewList.entries()) {
            const gridIndex = Math.floor(thumbIndex / maxGridSize)
            const offset = gridWidth * (gridIndex - startGridIndex)
            const orig = thumbImage.mapToItem(null, -offset, 0)
            thumbImageOrigList.push(orig)
        }
    }

    function show(index, swipe = false) {
        console.debug("Show:", index)
        initThumbImages()
        imageIndex = index
        swipeMode = swipe
        reverse = false
        animationStartCb = null
        active = true
    }

    function hide(index, swipe = false, closeCb) {
        imageIndex = index
        swipeMode = swipe
        reverse = true

        animationStartCb = () => {
            closeCb()

            // NOTE: this is a bit of hack
            // Coordinate mapping in initThumbImages works only if the full screen view is closed.
            // Closing the full screen mode earlier causes screen flicker
            fullImageLoader.initThumbImages()
        }

        active = true
    }
}
