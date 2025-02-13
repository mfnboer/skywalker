import QtQuick

ThumbImageView {
    required property imageview image
    required property int maxWidth
    required property int maxHeight
    property bool tileMode: false
    property bool noCrop: false

    width: tileMode ? maxWidth : Math.min(implicitWidth, maxWidth)
    fillMode: Image.PreserveAspectFit
    imageView: image

    onStatusChanged: {
        if (status === Image.Ready) {
            const idealHeight = sourceSize.height * width / sourceSize.width

            if ((maxHeight > 0 && idealHeight > maxHeight) || tileMode) {
                implicitHeight = maxHeight

                if (!noCrop)
                    fillMode = Image.PreserveAspectCrop
            }
        }
    }
}
