import QtQuick

ThumbImageView {
    required property imageview image
    required property int maxWidth
    required property int maxHeight
    property bool tileMode: false
    property bool noCrop: false
    readonly property bool isPortrait: root.width <= root.height

    width: tileMode ? maxWidth : (isPortrait ? Math.min(implicitWidth, maxWidth) : undefined)
    height: tileMode ? maxHeight : (isPortrait ? undefined : Math.min(implicitHeight, maxHeight))
    fillMode: Image.PreserveAspectFit
    imageView: image
    sourceSize.width: isPortrait ? width * Screen.devicePixelRatio : undefined
    sourceSize.height: isPortrait ? undefined : height * Screen.devicePixelRatio
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
