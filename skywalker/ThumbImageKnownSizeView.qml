import QtQuick

ThumbImageView {
    required property imageview image
    required property int maxWidth
    required property int maxHeight
    readonly property int idealHeight: (image.height / image.width) * width

    width: Math.min(image.width, maxWidth)
    height: Math.min(idealHeight, maxHeight)
    fillMode: idealHeight <= maxHeight ? Image.PreserveAspectFit : Image.PreserveAspectCrop
    imageView: image
}
