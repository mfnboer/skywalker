import QtQuick
import QtQuick.Layouts
import skywalker

RoundCornerMask {
    required property int contentVisibility
    required property string contentWarning
    property list<imageview> images
    property bool swipeMode: false

    signal activateSwipe

    id: frame
    width: parent.width
    height: filter.imageVisible() ? width : filter.height
    cornerRadius: swipeMode ? 0 : 10

    Grid {
        id: imgGrid
        z: parent.z - 1
        anchors.fill: parent
        columns: 2
        spacing: 4

        ThumbImageView {
            id: img1
            width: parent.width / 2 - parent.spacing / 2
            height: width
            Layout.fillWidth: true
            fillMode: Image.PreserveAspectCrop
            imageView: filter.getImage(0)
            sourceSize.width: width * Screen.devicePixelRatio
            sourceSize.height: height * Screen.devicePixelRatio
            smooth: false
        }

        ThumbImageView {
            id: img2
            width: parent.width / 2 - parent.spacing / 2
            height: width
            Layout.fillWidth: true
            fillMode: Image.PreserveAspectCrop
            imageView: filter.getImage(1)
            sourceSize.width: width * Screen.devicePixelRatio
            sourceSize.height: height * Screen.devicePixelRatio
            smooth: false
        }

        ThumbImageView {
            id: img3
            width: parent.width / 2 - parent.spacing / 2
            height: width
            Layout.fillWidth: true
            fillMode: Image.PreserveAspectCrop
            imageView: filter.getImage(2)
            sourceSize.width: width * Screen.devicePixelRatio
            sourceSize.height: height * Screen.devicePixelRatio
            smooth: false
        }

        ThumbImageView {
            id: img4
            width: parent.width / 2 - parent.spacing / 2
            height: width
            Layout.fillWidth: true
            fillMode: Image.PreserveAspectCrop
            imageView: filter.getImage(3)
            sourceSize.width: width * Screen.devicePixelRatio
            sourceSize.height: height * Screen.devicePixelRatio
            smooth: false
        }
    }
    MouseArea {
        enabled: filter.imageVisible()
        anchors.fill: imgGrid
        cursorShape: Qt.PointingHandCursor
        onClicked: {
            let p = Qt.point(mouseX, mouseY)
            let index = -1

            if (img1.contains(mapToItem(img1, p))) {
                if (img1.failedCanReload)
                    img1.reload()
                else
                    index = 0
            }
            else if (img2.contains(mapToItem(img2, p))) {
                if (img2.failedCanReload)
                    img2.reload()
                else
                    index = 1
            }
            else if (img3.contains(mapToItem(img3, p))) {
                if (img3.failedCanReload)
                    img3.reload()
                else
                    index = 2
            }
            else if (img4.contains(mapToItem(img4, p))) {
                if (img4.failedCanReload)
                    img4.reload()
                else
                    index = 3
            }

            if (index >= 0) {
                if (swipeMode)
                    activateSwipe()
                else
                    root.viewFullImage(images, index)
            }
        }
    }

    FilteredImageWarning {
        id: filter
        x: swipeMode ? 10 : 0
        width: parent.width - x * 2
        contentVisibility: frame.contentVisibility
        contentWarning: frame.contentWarning
        images: frame.images
    }
}

