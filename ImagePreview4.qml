import QtQuick
import QtQuick.Layouts
import skywalker

RoundedFrame {
    property list<imageview> images

    objectToRound: imgGrid
    width: parent.width
    height: width

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
            imageView: images[0]
        }

        ThumbImageView {
            id: img2
            width: parent.width / 2 - parent.spacing / 2
            height: width
            Layout.fillWidth: true
            fillMode: Image.PreserveAspectCrop
            imageView: images[1]
        }

        ThumbImageView {
            id: img3
            width: parent.width / 2 - parent.spacing / 2
            height: width
            Layout.fillWidth: true
            fillMode: Image.PreserveAspectCrop
            imageView: images[2]
        }

        ThumbImageView {
            id: img4
            width: parent.width / 2 - parent.spacing / 2
            height: width
            Layout.fillWidth: true
            fillMode: Image.PreserveAspectCrop
            imageView: images[3]
        }
    }
    MouseArea {
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
}

