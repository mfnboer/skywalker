import QtQuick
import skywalker

Loader {
    required property list<var> thumbImageViewList
    property list<point> thumbImageOrigList
    property list<imageview> images
    property int imageIndex: 0
    property bool swipeMode: false
    property var videoView
    property bool isAnimatedImage: false
    property string animatedImageAlt

    signal started
    signal finished
    signal activateSwipe(int imgIndex, var img)

    id: fullImageLoader
    active: false

    sourceComponent: AnimateToFullImage {
        property var startCb

        id: animation
        thumbImage: thumbImageViewList[imageIndex]
        thumbImageOrig: thumbImageOrigList[imageIndex]
        imageAlt: isAnimatedImage ? animatedImageAlt : images[imageIndex].alt
        swipeMode: fullImageLoader.swipeMode

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

    function show(index, swipe = false) {
        thumbImageOrigList = []

        for (const thumbImage of thumbImageViewList) {
            const orig = thumbImage.mapToItem(root.contentItem, 0, 0)
            thumbImageOrigList.push(orig)
        }

        imageIndex = index
        swipeMode = swipe
        active = true
    }
}
