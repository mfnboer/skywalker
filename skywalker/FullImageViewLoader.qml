import QtQuick
import skywalker

Loader {
    required property list<var> thumbImageViewList
    required property list<imageview> images
    property int imageIndex: 0

    id: fullImageLoader
    active: false

    sourceComponent: AnimateToFullImage {
        thumbImage: thumbImageViewList[imageIndex]

        onDone: (fullImg) => {
            root.viewFullImage(images, imageIndex, fullImg)
            fullImageLoader.active = false
        }
    }

    function show(index) {
        imageIndex = index
        active = true
    }
}
