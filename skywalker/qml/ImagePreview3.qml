import QtQuick
import QtQuick.Layouts
import skywalker

RoundCornerMask {
    required property int contentVisibility
    required property string contentWarning
    required property basicprofile contentLabeler
    property list<imageview> images
    property int spacing: 4
    property bool swipeMode: false
    readonly property list<var> imgList: [img1, img2, img3]
    readonly property int maxWidth: (guiSettings.maxImageHeight) * 1.5

    signal activateSwipe(int imgIndex, var previewImg)

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

        Rectangle {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: parent.width / 1.5 - frame.spacing / 2
            color: "transparent"

            ThumbImageView {
                id: img1
                anchors.fill: parent
                fillMode: Image.PreserveAspectCrop
                imageView: filter.getImage(0)
                sourceSize.width: width * Screen.devicePixelRatio
                sourceSize.height: height * Screen.devicePixelRatio
                smooth: false
            }
        }

        Rectangle {
            anchors.right: parent.right
            anchors.top: parent.top
            width: parent.width / 3 - frame.spacing / 2
            height: width
            color: "transparent"

            ThumbImageView {
                id: img2
                anchors.fill: parent
                fillMode: Image.PreserveAspectCrop
                imageView: filter.getImage(1)
                sourceSize.width: width * Screen.devicePixelRatio
                sourceSize.height: height * Screen.devicePixelRatio
                smooth: false
            }
        }

        Rectangle {
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            width: parent.width / 3 - frame.spacing / 2
            height: width
            color: "transparent"

            ThumbImageView {
                id: img3
                anchors.fill: parent
                fillMode: Image.PreserveAspectCrop
                imageView: filter.getImage(2)
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
                fullImageLoader.show(index, swipeMode)
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

    FullImageViewLoader {
        id: fullImageLoader
        thumbImageViewList: imgList
        images: frame.images

        onActivateSwipe: (imgIndex, previewImg) => frame.activateSwipe(imgIndex, previewImg)
    }

    function closeMedia(mediaIndex, closeCb) {
        if (mediaIndex < 3)
            fullImageLoader.hide(mediaIndex, swipeMode, closeCb)
        else
            closeCb()
    }
}
