import QtQuick
import skywalker

ThumbImageView {
    required property imageview image
    required property int maxWidth
    required property int maxHeight
    property bool tileMode: false
    property bool noCrop: false

    width: maxWidth
    height: 50
    fillMode: Image.PreserveAspectFit
    imageView: image
    smooth: false

    onStatusChanged: {
        if (status === Image.Ready)
            resize()
    }

    onMaxWidthChanged: Qt.callLater(resize)
    onMaxHeightChanged: Qt.callLater(resize)

    function resize() {
        if (sourceSize.width === 0 || sourceSize.height === 0)
            return

        const scale = Math.min(maxWidth / sourceSize.width, maxHeight / sourceSize.height)
        const idealHeight = sourceSize.height * scale
        const idealWidth = sourceSize.width * scale

        if ((maxHeight > 0 && idealHeight > maxHeight) || tileMode) {
            height = maxHeight

            if (!noCrop)
                fillMode = Image.PreserveAspectCrop
        }
        else {
            height = idealHeight
        }

        if ((maxWidth > 0 && idealWidth > maxWidth) || tileMode) {
            width = maxWidth

            if (!noCrop)
                fillMode = Image.PreserveAspectCrop
        }
        else {
            width = idealWidth
        }
    }
}
