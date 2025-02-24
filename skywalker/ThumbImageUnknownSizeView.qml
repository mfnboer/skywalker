import QtQuick

ThumbImageView {
    required property imageview image
    required property int maxWidth
    required property int maxHeight
    property bool tileMode: false
    property bool noCrop: false

    width: tileMode ? maxWidth : Math.min(implicitWidth, maxWidth)
    height: tileMode ? maxHeight : undefined
    fillMode: Image.PreserveAspectFit
    imageView: image
    sourceSize.width: width * Screen.devicePixelRatio
    smooth: false

    onStatusChanged: {
        if (status === Image.Ready) {
            const idealHeight = sourceSize.height * width / sourceSize.width

            if ((maxHeight > 0 && idealHeight > maxHeight) || tileMode) {
                height = maxHeight

                if (!noCrop)
                    fillMode = Image.PreserveAspectCrop
            }
        }
    }
}
