import QtQuick
import skywalker

ThumbImageView {
    required property imageview image
    required property int maxWidth
    required property int maxHeight
    readonly property int idealHeight: (image.height / image.width) * width
    readonly property bool exceedsMaxHeight: maxHeight > 0 && idealHeight > maxHeight
    property bool tileMode: false
    property bool noCrop: false

    // NOTE: image.width and image.height express the aspect ratio, not the real size
    // width:  tileMode ? maxWidth : Math.min(image.width, maxWidth)
    width: maxWidth
    height: (!exceedsMaxHeight && !tileMode) ? idealHeight : maxHeight
    fillMode: ((!exceedsMaxHeight && !tileMode) || noCrop) ? Image.PreserveAspectFit : Image.PreserveAspectCrop
    imageView: image
    sourceSize.width: width * Screen.devicePixelRatio
    sourceSize.height: height * Screen.devicePixelRatio
    smooth: false
}
