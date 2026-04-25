import QtQuick
import QtQuick.Layouts
import skywalker

RoundCornerMask {
    required property int contentVisibility
    required property string contentWarning
    required property basicprofile contentLabeler
    property list<imageview> images
    property int startImageIndex: 0
    property int maxHeight: guiSettings.maxImageHeight
    property bool swipeMode: false
    readonly property list<var> imgList: [img1, img2]
    readonly property int maxWidth: maxHeight * 2

    signal activateSwipe(int imgIndex, var previewImg)
    signal showFullImage(int imgIndex, bool swipeMode)

    id: frame
    width: parent.width
    height: filter.imageVisible() ? Math.min(width, maxWidth) / 2 : filter.height
    maskWidth: imgRow.width
    cornerRadius: swipeMode ? 0 : guiSettings.radius

    Row {
        id: imgRow
        x: (parent.width - width) / 2
        z: parent.z - 1
        width: Math.min(parent.width, maxWidth)
        spacing: 4

        // The rectangle is here to keep an empty space when the image is made invisble
        // by AnimateToFullImage
        Item {
            width: parent.width / 2 - parent.spacing / 2
            height: width

            ThumbImageView {
                id: img1
                anchors.fill: parent
                fillMode: Image.PreserveAspectCrop
                imageView: filter.getImage(startImageIndex)
                sourceSize.width: width * Screen.devicePixelRatio
                sourceSize.height: height * Screen.devicePixelRatio
                smooth: false
            }
        }

        Item {
            width: parent.width / 2 - parent.spacing / 2
            height: width

            ThumbImageView {
                id: img2
                anchors.fill: parent
                fillMode: Image.PreserveAspectCrop
                imageView: filter.getImage(startImageIndex + 1)
                sourceSize.width: width * Screen.devicePixelRatio
                sourceSize.height: height * Screen.devicePixelRatio
                smooth: false
            }
        }
    }
    SkyMouseArea {
        enabled: filter.imageVisible()
        anchors.fill: imgRow
        onClicked: {
            let p = Qt.point(mouseX, mouseY)
            let index = -1

            for (let i = 0; i < imgList.length; ++i) {
                const img = imgList[i]

                if (img.contains(mapToItem(img, p))) {
                    if (img.failedCanReload)
                        img.reload()
                    else
                        index = i
                }
            }

            if (index >= 0)
                showFullImage(startImageIndex + index, swipeMode)
        }
    }

    FilteredImageWarning {
        id: filter
        x: swipeMode ? 10 : 0
        width: parent.width - x * 2
        contentVisibility: frame.contentVisibility
        contentWarning: frame.contentWarning
        contentLabeler: frame.contentLabeler
        images: frame.images
    }

    function getFilter() {
        return filter
    }
}
