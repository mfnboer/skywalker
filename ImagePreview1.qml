import QtQuick
import QtQuick.Layouts
import skywalker

RoundedFrame {
    property list<imageview> images

    objectToRound: img
    width: parent.width
    height: img.height

    Image {
        id: img
        width: parent.width
        Layout.fillWidth: true
        source: images[0].thumbUrl
        fillMode: Image.PreserveAspectFit
    }
}
