import QtQuick
import skywalker

Loader {
    required property list<var> thumbImageViewList
    required property list<imageview> images
    property int imageIndex: 0
    property bool swipeMode: false
    property var videoView

    signal finished
    signal activateSwipe(int imgIndex, var img)

    id: fullImageLoader
    active: false

    sourceComponent: AnimateToFullImage {
        id: animation
        thumbImage: thumbImageViewList[imageIndex]
        imageAlt: images[imageIndex].alt
        swipeMode: fullImageLoader.swipeMode

        onDone: (fullImg) => {
            if (swipeMode) {
                fullImageLoader.activateSwipe(imageIndex, fullImg)
                fullImageLoader.active = false
            } else if (videoView) {
                let imgAnimation = animation
                root.viewFullVideo(videoView, fullImg, () => { imgAnimation.reverseRun() })
            } else {
                let imgAnimation = animation
                root.viewFullImage(images, imageIndex, fullImg, () => { imgAnimation.reverseRun() })
            }
        }

        onReverseDone: {
            finished()
            fullImageLoader.active = false
        }
    }

    function show(index, swipe = false) {
        imageIndex = index
        swipeMode = swipe
        active = true
    }
}
