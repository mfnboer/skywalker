import QtQuick
import QtQuick.Layouts
import skywalker

RoundedImage {
    property list<imageview> images

    width: parent.width
    Layout.fillWidth: true
    source: images[0].thumbUrl
    fillMode: Image.PreserveAspectFit
}
