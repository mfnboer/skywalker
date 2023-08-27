import QtQuick
import QtQuick.Layouts
import skywalker

Rectangle {
    property list<imageview> images

    width: parent.width
    height: width / 2

    Row {
        anchors.fill: parent
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
    }
}
