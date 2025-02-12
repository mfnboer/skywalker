import QtQuick

ThumbImageView {
    required property imageview image
    required property int maxWidth
    required property int maxHeight

    width: Math.min(implicitWidth, maxWidth)
    fillMode: Image.PreserveAspectFit
    imageView: image

    onStatusChanged: {
        if (status === Image.Ready) {
            const idealHeight = sourceSize.height * width / sourceSize.width

            if (idealHeight > maxHeight) {
                implicitHeight = maxHeight
                fillMode = Image.PreserveAspectCrop
            }
        }
    }
}
