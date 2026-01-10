import QtQuick
import QtQuick.Controls
import skywalker

Loader {
    required property list<var> thumbImageViewList
    property list<point> thumbImageOrigList
    property list<imageview> images
    property int imageIndex: 0
    property bool swipeMode: false
    property bool reverse: false
    property bool noHeader: false
    property var animationStartCb
    property var videoView
    property bool isAnimatedImage: false
    property string animatedImageAlt

    signal started
    signal finished
    signal activateSwipe(int imgIndex, var img)

    id: fullImageLoader
    active: false

    sourceComponent: AnimateToFullImage {
        property var startCb: animationStartCb

        id: animation
        thumbImage: thumbImageViewList[imageIndex]
        thumbImageOrig: imageIndex < thumbImageOrigList.length ? thumbImageOrigList[imageIndex] : Qt.point(0, 0)
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
                root.viewFullAnimatedImage(thumbImageViewList[imageIndex].url, animatedImageAlt, fullImg, imgAnimation.reverseRun)
            } else {
                let imgAnimation = animation
                root.viewFullImage(images, imageIndex, fullImg, imgAnimation.goBack)
            }
        }

        onReverseDone: {
            finished()
            fullImageLoader.active = false
        }

        function goBack(imgIndex, closeCb) {
            startCb = closeCb
            imageIndex = imgIndex
            reverseRun()
        }
    }

    function initThumbImages() {
        thumbImageOrigList = []

        for (const thumbImage of thumbImageViewList) {
            const orig = thumbImage.mapToItem(root.contentItem, 0, 0)
            thumbImageOrigList.push(orig)
        }
    }

    function show(index, swipe = false) {
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
            // Coordinate mapping in initThumImages works only if the full screen view is closed.
            // Closing the full screen mode earlier causes screen flicker
            fullImageLoader.initThumbImages()
        }

        active = true
    }
}
