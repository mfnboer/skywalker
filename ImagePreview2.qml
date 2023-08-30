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
}
