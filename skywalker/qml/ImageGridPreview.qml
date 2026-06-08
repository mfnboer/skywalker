import QtQuick
import QtQuick.Controls
import skywalker

Item {
    required property list<imageview> images
    required property int contentVisibility // QEnums::PostContentVisibility
    required property string contentWarning
    required property basicprofile contentLabeler
    property string bodyBackgroundColor: guiSettings.backgroundColor
    property bool swipeMode: false
    property bool postVisible: true
    readonly property int maxGridSize: guiSettings.maxPreviewImageGridSize
    readonly property int numPages: Math.ceil(images.length / maxGridSize)

    signal activateSwipe(int imgIndex, var previewImg)

    id: previewItem
    width: parent.width
    height: parent.height

    SwipeView {
        id: view
        width: parent.width
        height: calcHeight()
        spacing: 4
        clip: numPages > 1

        Repeater {
            id: gridRepeater
            model: numPages

            Item {
                readonly property int startIndex: SwipeView.index * maxGridSize
                readonly property int endIndex: Math.min(startIndex + maxGridSize, images.length)
                readonly property int imageCount: endIndex - startIndex

                id: grid
                width: view.width

                LoaderImagePreview1 {
                    id: images1Loader
                    width: view.width
                    postImages: images
                    startImageIndex: grid.startIndex
                    imageCount: grid.imageCount
                    postContentVisibility: contentVisibility
                    postContentWarning: contentWarning
                    postContentLabeler: contentLabeler
                    bodyBackgroundColor: previewItem.bodyBackgroundColor
                    swipeMode: previewItem.swipeMode
                    postVisible: previewItem.postVisible
                    maxImageHeight: grid.startIndex > 0 ? view.height : guiSettings.maxImageHeight

                    onActivateSwipe: (imgIndex, previewImg) => previewItem.activateSwipe(imgIndex, previewImg)
                    onShowFullImage: (imgIndex, swipeMode) => zoomToImage(imgIndex, swipeMode)
                }
                LoaderImagePreview2 {
                    id: images2Loader
                    width: view.width
                    postImages: images
                    startImageIndex: grid.startIndex
                    imageCount: grid.imageCount
                    postContentVisibility: contentVisibility
                    postContentWarning: contentWarning
                    postContentLabeler: contentLabeler
                    bodyBackgroundColor: previewItem.bodyBackgroundColor
                    swipeMode: previewItem.swipeMode
                    postVisible: previewItem.postVisible
                    maxImageHeight: grid.startIndex > 0 ? view.height : guiSettings.maxImageHeight

                    onActivateSwipe: (imgIndex, previewImg) => previewItem.activateSwipe(imgIndex, previewImg)
                    onShowFullImage: (imgIndex, swipeMode) => zoomToImage(imgIndex, swipeMode)
                }
                LoaderImagePreview3 {
                    id: images3Loader
                    width: view.width
                    postImages: images
                    startImageIndex: grid.startIndex
                    imageCount: grid.imageCount
                    postContentVisibility: contentVisibility
                    postContentWarning: contentWarning
                    postContentLabeler: contentLabeler
                    bodyBackgroundColor: previewItem.bodyBackgroundColor
                    swipeMode: previewItem.swipeMode
                    postVisible: previewItem.postVisible
                    maxImageHeight: grid.startIndex > 0 ? view.height : guiSettings.maxImageHeight

                    onActivateSwipe: (imgIndex, previewImg) => previewItem.activateSwipe(imgIndex, previewImg)
                    onShowFullImage: (imgIndex, swipeMode) => zoomToImage(imgIndex, swipeMode)
                }
                LoaderImagePreview4 {
                    id: images4Loader
                    width: view.width
                    postImages: images
                    startImageIndex: grid.startIndex
                    imageCount: grid.imageCount
                    postContentVisibility: contentVisibility
                    postContentWarning: contentWarning
                    postContentLabeler: contentLabeler
                    bodyBackgroundColor: previewItem.bodyBackgroundColor
                    swipeMode: previewItem.swipeMode
                    postVisible: previewItem.postVisible
                    maxImageHeight: grid.startIndex > 0 ? view.height : guiSettings.maxImageHeight

                    onActivateSwipe: (imgIndex, previewImg) => previewItem.activateSwipe(imgIndex, previewImg)
                    onShowFullImage: (imgIndex, swipeMode) => zoomToImage(imgIndex, swipeMode)
                }

                function calcHeight() {
                    if (imageCount === 1)
                        return images1Loader.calcHeight()
                    else if (imageCount === 2)
                        return images2Loader.calcHeight()
                    else if (imageCount === 3)
                        return images3Loader.calcHeight()
                    else if (imageCount === 4)
                        return images4Loader.calcHeight()
                    else
                        return 0
                }

                function getFilter() {
                    if (images1Loader.item)
                        return images1Loader.item.getFilter()
                    else if (images2Loader.item)
                        return images2Loader.item.getFilter()
                    else if (images3Loader.item)
                        return images3Loader.item.getFilter()
                    else if (images4Loader.item)
                        return images4Loader.item.getFilter()
                }

                function getThumbImageList() {
                    if (images1Loader.item)
                        return images1Loader.item.imgList
                    else if (images2Loader.item)
                        return images2Loader.item.imgList
                    else if (images3Loader.item)
                        return images3Loader.item.imgList
                    else if (images4Loader.item)
                        return images4Loader.item.imgList
                }
            }
        }

        function calcHeight() {
            if (gridRepeater.count === 0)
                return 0

            return gridRepeater.itemAt(0).calcHeight()
        }

        function getFilter() {
            if (gridRepeater.count === 0)
                return 0

            return gridRepeater.itemAt(0).getFilter()
        }

        function getThumbImageList() {
            let imgList = []

            for (let i = 0; i < gridRepeater.count; ++i)
                imgList = imgList.concat(gridRepeater.itemAt(i).getThumbImageList())

            return imgList
        }
    }

    AccessibleText {
        id: moreLeft
        anchors.top: view.bottom
        padding: 10
        color: guiSettings.linkColor
        text: qsTr("← more")
        visible: view.currentIndex > 0

        SkyMouseArea {
            anchors.fill: parent
            onClicked: view.currentIndex--
        }
    }

    PageIndicator {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: view.bottom
        anchors.topMargin: 10
        Material.foreground: guiSettings.textColor
        currentIndex: view.currentIndex
        count: view.count
        visible: view.count > 1
    }

    AccessibleText {
        id: moreRight
        anchors.right: parent.right
        anchors.top: view.bottom
        padding: 10
        color: guiSettings.linkColor
        text: qsTr("more →")
        visible: view.currentIndex < view.count - 1

        SkyMouseArea {
            anchors.fill: parent
            onClicked: view.currentIndex++
        }
    }

    function getFilter() {
        return view.getFilter()
    }

    function closeMedia(mediaIndex, closeCb) {
        const gridIndex = Math.floor(mediaIndex / maxGridSize)
        const cellIndex = mediaIndex % maxGridSize
        view.setCurrentIndex(gridIndex)
        fullImageLoader.hide(mediaIndex, swipeMode, closeCb)
    }

    function zoomToImage(imgIndex, swipeMode) {
        fullImageLoader.startGridIndex = view.currentIndex
        fullImageLoader.show(imgIndex, swipeMode)
    }

    FullImageViewLoader {
        id: fullImageLoader
        thumbImageViewList: view.getThumbImageList()
        images: previewItem.images
        gridWidth: view.width

        onActivateSwipe: (imgIndex, previewImg) => previewItem.activateSwipe(imgIndex, previewImg)

        onGoingBackTo: (imgIndex) => {
            const gridIndex = Math.floor(imgIndex / maxGridSize)
            console.debug("Go back to image:", imgIndex, "grid:", gridIndex)
            view.setCurrentIndex(gridIndex)
        }
    }

    Component.onCompleted: {
        // Prevent the swipe gesture to propagate up when the end of the SwipeView is
        // reached. Propagating up causes the feed to swipe to the next pinned feed.
        view.contentItem.boundsBehavior = Flickable.DragOverBounds
        view.contentItem.boundsMovement = Flickable.StopAtBounds
    }
}
