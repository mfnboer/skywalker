import QtQuick
import QtQuick.Controls
import skywalker

Flickable {
    required property list<imageview> images
    required property int contentVisibility // QEnums::PostContentVisibility
    required property string contentWarning
    required property basicprofile contentLabeler
    property string bodyBackgroundColor: guiSettings.backgroundColor
    property bool swipeMode: false
    property bool postVisible: true

    signal activateSwipe(int imgIndex, var previewImg)

    id: scroller
    contentWidth: imageRow.width
    contentHeight: height

    Row {
        id: imageRow
        height: parent.height
        spacing: 4

        Repeater {
            id: imageRepeater
            model: images

            ImagePreviewInCarousel {
                required property imageview modelData
                required property int index

                width: calcImageWidth(modelData)
                height: parent.height
                contentVisibility: scroller.contentVisibility
                contentWarning: scroller.contentWarning
                contentLabeler: scroller.contentLabeler
                images: scroller.images
                startImageIndex: index
                swipeMode: scroller.swipeMode
                maskColor: bodyBackgroundColor

                onActivateSwipe: (imgIndex, previewImg) => scroller.activateSwipe(imgIndex, previewImg)
                onShowFullImage: (imgIndex, swipeMode) => zoomToImage(imgIndex, swipeMode)

                SkyLabel {
                    anchors.right: parent.right
                    anchors.rightMargin: 10
                    anchors.top: parent.top
                    anchors.topMargin: 5
                    backgroundColor: "black"
                    backgroundOpacity: 0.6
                    color: "white"
                    text: `${index + 1}/${images.length}`
                }
            }
        }

        function getFilter() {
            if (imageRepeater.count === 0)
                return 0

            return imageRepeater.itemAt(0).getFilter()
        }

        function getThumbImageList() {
            let imgList = []

            for (let i = 0; i < imageRepeater.count; ++i)
                imgList = imgList.concat(imageRepeater.itemAt(i).imgList)

            return imgList
        }
    }

    function calcImageWidth(img) {
        if (img.width <= 0 || img.height <= 0)
            return height

        const ratio = img.height / img.width
        return Math.min(height / ratio, width)
    }

    function getFilter() {
        return imageRow.getFilter()
    }

    function closeMedia(mediaIndex, closeCb) {
        fullImageLoader.hide(mediaIndex, swipeMode, closeCb)
    }

    function zoomToImage(imgIndex, swipeMode) {
        fullImageLoader.show(imgIndex, swipeMode)
    }

    FullImageViewLoader {
        id: fullImageLoader
        thumbImageViewList: imageRow.getThumbImageList()
        images: scroller.images
        gridWidth: imageRow.width
        maxGridSize: scroller.images.length

        onActivateSwipe: (imgIndex, previewImg) => scroller.activateSwipe(imgIndex, previewImg)

        onGoingBackTo: (imgIndex) => {
            scroller.flickToChild(imageRepeater.itemAt(imgIndex), Flickable.Visible)
        }
    }
}
