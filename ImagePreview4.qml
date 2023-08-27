import QtQuick
import QtQuick.Layouts
import skywalker

Rectangle {
    property list<imageview> images

    width: parent.width
    height: width

    Grid {
        anchors.fill: parent
        columns: 2
        spacing: 4

        RoundedImage {
            id: img1
            width: parent.width / 2 - parent.spacing / 2
            height: width
            Layout.fillWidth: true
            fillMode: Image.PreserveAspectCrop
            source: images[0].thumbUrl
        }
        RoundedImage {
            id: img2
            width: parent.width / 2 - parent.spacing / 2
            height: width
            Layout.fillWidth: true
            fillMode: Image.PreserveAspectCrop
            source: images[1].thumbUrl
        }
        RoundedImage {
            id: img3
            width: parent.width / 2 - parent.spacing / 2
            height: width
            Layout.fillWidth: true
            fillMode: Image.PreserveAspectCrop
            source: images[2].thumbUrl
        }
        RoundedImage {
            id: img4
            width: parent.width / 2 - parent.spacing / 2
            height: width
            Layout.fillWidth: true
            fillMode: Image.PreserveAspectCrop
            source: images[3].thumbUrl
        }
    }
}

