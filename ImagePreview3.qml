import QtQuick
import QtQuick.Layouts
import skywalker

RoundedFrame {
    property list<imageview> images
    property int spacing: 4

    id: frame
    objectToRound: imgGrid
    width: parent.width
    height: parent.width / 1.5

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
            imageView: images[0]
        }

        ThumbImageView {
            id: img2
            anchors.right: parent.right
            anchors.top: parent.top
            width: parent.width / 3 - frame.spacing / 2
            height: width
            Layout.fillWidth: true
            fillMode: Image.PreserveAspectCrop
            imageView: images[1]
        }

        ThumbImageView {
            id: img3
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            width: parent.width / 3 - frame.spacing / 2
            height: width
            Layout.fillWidth: true
            fillMode: Image.PreserveAspectCrop
            imageView: images[2]
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

            if (index >= 0)
                root.viewFullImage(images, index)
        }
    }
}
