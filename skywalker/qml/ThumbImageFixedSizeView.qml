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
        width: parent.width
        height: parent.height
        z: parent.z - 1
        color: canvasColor
        visible: fillMode == Image.PreserveAspectFit
    }

    ImageUtils {
        id: imageUtils
    }

    onStatusChanged: {
        if (fillMode != Image.PreserveAspectFit)
            return

        if (status != Image.Ready)
            return

        const cutX = Math.max((width - paintedWidth) / 2, 0)
        const cutY = Math.max((height - paintedHeight) / 2, 0)
        const cutWidth = Math.min(paintedWidth, width - cutX)
        const cutHeight = Math.min(paintedHeight, height - cutY)
        const cutRect = Qt.rect(cutX * Screen.devicePixelRatio, cutY * Screen.devicePixelRatio,
                                cutWidth * Screen.devicePixelRatio, cutHeight * Screen.devicePixelRatio)

        const grabbed = thumb.grabToImage((result) => {
            canvasColor = imageUtils.getDominantColor(result.image, cutRect, 16 * Screen.devicePixelRatio)
        })
    }
}
