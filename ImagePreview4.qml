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

        Image {
            id: img1
            width: parent.width / 2 - parent.spacing / 2
            height: width
            Layout.fillWidth: true
            fillMode: Image.PreserveAspectCrop
            source: images[0].thumbUrl
        }
        Image {
            id: img2
            width: parent.width / 2 - parent.spacing / 2
            height: width
            Layout.fillWidth: true
            fillMode: Image.PreserveAspectCrop
            source: images[1].thumbUrl
        }
        Image {
            id: img3
            width: parent.width / 2 - parent.spacing / 2
            height: width
            Layout.fillWidth: true
            fillMode: Image.PreserveAspectCrop
            source: images[2].thumbUrl
        }
        Image {
            id: img4
            width: parent.width / 2 - parent.spacing / 2
            height: width
            Layout.fillWidth: true
            fillMode: Image.PreserveAspectCrop
            source: images[3].thumbUrl
        }
    }
}

