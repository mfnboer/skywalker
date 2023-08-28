import QtQuick
import QtQuick.Layouts
import skywalker

RoundedFrame {
    property list<imageview> images
    property int spacing: 4

    objectToRound: imgGrid
    width: parent.width
    height: parent.width / 1.5

    Item {
        id: imgGrid
        anchors.fill: parent

        Image {
            id: img1
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: parent.width / 1.5 - parent.spacing / 2
            height: parent.height
            Layout.fillWidth: true
            fillMode: Image.PreserveAspectCrop
            source: images[0].thumbUrl
        }
        Image {
            id: img2
            anchors.right: parent.right
            anchors.top: parent.top
            width: parent.width / 3 - parent.spacing / 2
            height: width
            Layout.fillWidth: true
            fillMode: Image.PreserveAspectCrop
            source: images[1].thumbUrl
        }
        Image {
            id: img3
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            width: parent.width / 3 - parent.spacing / 2
            height: width
            Layout.fillWidth: true
            fillMode: Image.PreserveAspectCrop
            source: images[2].thumbUrl
        }
    }
}
