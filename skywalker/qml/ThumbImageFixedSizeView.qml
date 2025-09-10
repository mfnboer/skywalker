import QtQuick
import QtQuick.Controls
import skywalker

ThumbImageView {
    required property imageview image

    id: thumb
    fillMode: Image.PreserveAspectFit
    imageView: image
    sourceSize.width: width * Screen.devicePixelRatio
    sourceSize.height: height * Screen.devicePixelRatio
    smooth: false

    Rectangle {
        width: parent.width
        height: parent.height
        z: parent.z - 1
        color: guiSettings.postHighLightColor
        visible: fillMode == Image.PreserveAspectFit
    }
}
