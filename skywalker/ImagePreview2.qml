import QtQuick
import QtQuick.Layouts
import skywalker

RoundedFrame {
    required property int contentVisibility
    required property string contentWarning
    property list<imageview> images

    id: frame
    objectToRound: imgRow
    width: parent.width
    height: filter.imageVisible() ? width / 2 : filter.height

    Row {
        id: imgRow
        anchors.fill: parent
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
    }
    MouseArea {
        enabled: filter.imageVisible()
        anchors.fill: imgRow
        cursorShape: Qt.PointingHandCursor
        onClicked: {
            let p = Qt.point(mouseX, mouseY)
            let index = -1

            if (img1.contains(mapToItem(img1, p)))
                index = 0
            else if (img2.contains(mapToItem(img2, p)))
                index = 1

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

    FilteredImageWarning {
        id: filter
        width: parent.width
        contentVisibiliy: frame.contentVisibility
        contentWarning: frame.contentWarning
        images: frame.images
    }
}
