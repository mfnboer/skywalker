import QtQuick
import QtQuick.Layouts
import skywalker

RoundedFrame {
    required property int contentVisibility
    required property string contentWarning
    property list<imageview> images

    id: frame
    objectToRound: imgGrid
    width: parent.width
    height: filter.imageVisible() ? width : filter.height

    Grid {
        id: imgGrid
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
        }

        ThumbImageView {
            id: img2
            width: parent.width / 2 - parent.spacing / 2
            height: width
            Layout.fillWidth: true
            fillMode: Image.PreserveAspectCrop
            imageView: filter.getImage(1)
        }

        ThumbImageView {
            id: img3
            width: parent.width / 2 - parent.spacing / 2
            height: width
            Layout.fillWidth: true
            fillMode: Image.PreserveAspectCrop
            imageView: filter.getImage(2)
        }

        ThumbImageView {
            id: img4
            width: parent.width / 2 - parent.spacing / 2
            height: width
            Layout.fillWidth: true
            fillMode: Image.PreserveAspectCrop
            imageView: filter.getImage(3)
        }
    }
    MouseArea {
        enabled: filter.imageVisible()
        anchors.fill: imgGrid
        cursorShape: Qt.PointingHandCursor
        onClicked: {
            let p = Qt.point(mouseX, mouseY)
            let index = -1

            if (img1.contains(mapToItem(img1, p)))
                index = 0
            else if (img2.contains(mapToItem(img2, p)))
                index = 1
            else if (img3.contains(mapToItem(img3, p)))
                index = 2
            else if (img4.contains(mapToItem(img4, p)))
                index = 3

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
    AccessibleImage {
        image: img4
        alt: img4.imageView.alt
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

