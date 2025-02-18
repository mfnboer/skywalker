import QtQuick

ThumbImageView {
    required property imageview image
    required property int maxWidth
    required property int maxHeight
    readonly property int idealHeight: (image.height / image.width) * width
    readonly property bool exceedsMaxHeight: maxHeight > 0 && idealHeight > maxHeight
    property bool tileMode: false
    property bool noCrop: false

    width: tileMode ? maxWidth : Math.min(image.width, maxWidth)
    height: (!exceedsMaxHeight && !tileMode) ? idealHeight : maxHeight
    fillMode: ((!exceedsMaxHeight && !tileMode) || noCrop) ? Image.PreserveAspectFit : Image.PreserveAspectCrop
    imageView: image
    sourceSize.width: width * Screen.devicePixelRatio
    sourceSize.height: height * Screen.devicePixelRatio
    smooth: false
}
