import QtQuick
import QtQuick.Layouts
import skywalker

RoundedFrame {
    required property int contentVisibility
    required property string contentWarning
    property list<imageview> images
    property int spacing: 4

    id: frame
    objectToRound: imgGrid
    width: parent.width
    height: filter.imageVisible() ? parent.width / 1.5 : filter.height

    Item {
        id: imgGrid
        anchors.fill: parent

        ThumbImageView {
            id: img1
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: parent.width / 1.5 - frame.spacing / 2
            height: parent.height
            Layout.fillWidth: true
            fillMode: Image.PreserveAspectCrop
            imageView: filter.getImage(0)
        }

        ThumbImageView {
            id: img2
            anchors.right: parent.right
            anchors.top: parent.top
            width: parent.width / 3 - frame.spacing / 2
            height: width
            Layout.fillWidth: true
            fillMode: Image.PreserveAspectCrop
            imageView: filter.getImage(1)
        }

        ThumbImageView {
            id: img3
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            width: parent.width / 3 - frame.spacing / 2
            height: width
            Layout.fillWidth: true
            fillMode: Image.PreserveAspectCrop
            imageView: filter.getImage(2)
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

            if (index >= 0)
                root.viewFullImage(images, index)
        }
    }

    AccessibleImage {
        image: img1
        alt: img1.imageView.alt
        visible: filter.imageVisible()
    }
    AccessibleImage {
        image: img2
        alt: img2.imageView.alt
        visible: filter.imageVisible()
    }
    AccessibleImage {
        image: img3
        alt: img3.imageView.alt
        visible: filter.imageVisible()
    }

    FilteredImageWarning {
        id: filter
        width: parent.width
        contentVisibiliy: frame.contentVisibility
        contentWarning: frame.contentWarning
        images: frame.images
    }
}
