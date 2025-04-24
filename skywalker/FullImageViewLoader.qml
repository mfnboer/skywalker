import QtQuick
import skywalker

Loader {
    required property list<var> thumbImageViewList
    required property list<imageview> images
    property int imageIndex: 0

    signal finished

    id: fullImageLoader
    active: false

    sourceComponent: AnimateToFullImage {
        id: animation
        thumbImage: thumbImageViewList[imageIndex]
        imageAlt: images[imageIndex].alt

        onDone: (fullImg) => {
            let imgAnimation = animation
            root.viewFullImage(images, imageIndex, fullImg, () => { imgAnimation.reverseRun() })
        }

        onReverseDone: {
            finished()
            fullImageLoader.active = false
        }
    }

    function show(index) {
        imageIndex = index
        active = true
    }
}
