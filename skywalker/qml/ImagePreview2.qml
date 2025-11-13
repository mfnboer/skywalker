import QtQuick
import QtQuick.Layouts
import skywalker

RoundCornerMask {
    required property int contentVisibility
    required property string contentWarning
    required property basicprofile contentLabeler
    property list<imageview> images
    property bool swipeMode: false
    readonly property list<var> imgList: [img1, img2]
    readonly property int maxWidth: (guiSettings.maxImageHeight) * 2

    signal activateSwipe

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
        Rectangle {
            width: parent.width / 2 - parent.spacing / 2
            height: width
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
            width: parent.width / 2 - parent.spacing / 2
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
    }
    MouseArea {
        enabled: filter.imageVisible()
        anchors.fill: imgRow
        cursorShape: Qt.PointingHandCursor
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

            if (index >= 0) {
                if (swipeMode)
                    activateSwipe()
                else
                    fullImageLoader.show(index)
            }
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
    }
}
