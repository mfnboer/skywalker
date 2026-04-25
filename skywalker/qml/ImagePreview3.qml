import QtQuick
import QtQuick.Layouts
import skywalker

RoundCornerMask {
    required property int contentVisibility
    required property string contentWarning
    required property basicprofile contentLabeler
    property list<imageview> images
    property int startImageIndex: 0
    property int spacing: 4
    property bool swipeMode: false
    readonly property list<var> imgList: [img1, img2, img3]
    property int maxHeight: guiSettings.maxImageHeight
    readonly property int maxWidth: maxHeight * 1.5

    signal activateSwipe(int imgIndex, var previewImg)
    signal showFullImage(int imgIndex, bool swipeMode)

    id: frame
    width: parent.width
    height: filter.imageVisible() ? Math.min(width, maxWidth) / 1.5 : filter.height
    maskWidth: imgGrid.width
    cornerRadius: swipeMode ? 0 : guiSettings.radius

    Item {
        id: imgGrid
        x: (parent.width - width) / 2
        z: parent.z - 1
        width: Math.min(parent.width, maxWidth)
        height: width / 1.5

        Item {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: parent.width / 1.5 - frame.spacing / 2

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
            anchors.right: parent.right
            anchors.top: parent.top
            width: parent.width / 3 - frame.spacing / 2
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

        Item {
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            width: parent.width / 3 - frame.spacing / 2
            height: width

            ThumbImageView {
                id: img3
                anchors.fill: parent
                fillMode: Image.PreserveAspectCrop
                imageView: filter.getImage(startImageIndex + 2)
                sourceSize.width: width * Screen.devicePixelRatio
                sourceSize.height: height * Screen.devicePixelRatio
                smooth: false
            }
        }
    }
    SkyMouseArea {
        enabled: filter.imageVisible()
        anchors.fill: imgGrid
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
