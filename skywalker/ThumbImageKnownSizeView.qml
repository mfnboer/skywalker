import QtQuick

ThumbImageView {
    required property imageview image
    required property int maxWidth
    required property int maxHeight
    readonly property int idealHeight: (image.height / image.width) * width
    readonly property bool exceedsMaxHeight: maxHeight > 0 && idealHeight > maxHeight

    width: Math.min(image.width, maxWidth)
    height: !exceedsMaxHeight ? idealHeight : maxHeight
    fillMode: !exceedsMaxHeight ? Image.PreserveAspectFit : Image.PreserveAspectCrop
    imageView: image
}
