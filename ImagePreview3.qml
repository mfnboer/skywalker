import QtQuick
import QtQuick.Layouts
import skywalker

Rectangle {
    property list<imageview> images
    property int spacing: 4

    width: parent.width
    height: width

    RoundedImage {
        id: img1
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: parent.width / 2 - parent.spacing / 2
        height: parent.height
        Layout.fillWidth: true
        fillMode: Image.PreserveAspectCrop
        source: images[0].thumbUrl
    }
    RoundedImage {
        id: img2
        anchors.right: parent.right
        anchors.top: parent.top
        width: parent.width / 2 - parent.spacing / 2
        height: width
        Layout.fillWidth: true
        fillMode: Image.PreserveAspectCrop
        source: images[1].thumbUrl
    }
    RoundedImage {
        id: img3
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        width: parent.width / 2 - parent.spacing / 2
        height: width
        Layout.fillWidth: true
        fillMode: Image.PreserveAspectCrop
        source: images[2].thumbUrl
    }
}
