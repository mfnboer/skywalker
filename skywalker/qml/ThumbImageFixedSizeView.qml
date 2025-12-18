import QtQuick
import QtQuick.Controls
import skywalker

ThumbImageView {
    required property imageview image
    property color canvasColor: guiSettings.postHighLightColor

    id: thumb
    fillMode: Image.PreserveAspectFit
    imageView: image
    sourceSize.width: width * Screen.devicePixelRatio
    sourceSize.height: height * Screen.devicePixelRatio
    smooth: false

    Rectangle {
        property bool hideImage: false

        id: canvas
        width: parent.width
        height: parent.height
        z: parent.z - (hideImage ? -1 : 1)
        color: canvasColor
        visible: fillMode == Image.PreserveAspectFit
    }

    SkyImageUtils {
        id: imageUtils
    }

    onStatusChanged: {
        if (fillMode != Image.PreserveAspectFit)
            return

        if (status != Image.Ready)
            return

        imageUtils.setDominantColor(thumb, (color) => { thumb.canvasColor = color })
    }

    function getVisible() {
        return fillMode == Image.PreserveAspectFit ? !canvas.hideImage : visible
    }

    function setVisible(v) {
        if (fillMode == Image.PreserveAspectFit)
            canvas.hideImage = !v
        else
            visible = v
    }
}
