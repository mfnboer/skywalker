import QtQuick
import QtQuick.Layouts
import skywalker

RoundedFrame {
    property list<imageview> images

    objectToRound: imgRow
    width: parent.width
    height: width / 2

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
    }
    MouseArea {
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
}
