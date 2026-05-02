import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

RoundCornerMask {
    required property string url
    property string title
    property string description
    required property int contentVisibility // QEnums::ContentVisibility
    required property string contentWarning
    required property basicprofile contentLabeler
    property string backgroundColor: guiSettings.backgroundColor
    property size imgSize
    readonly property double aspectRatio: (imgSize.width > 0 && imgSize.height > 0) ? imgSize.height / imgSize.width : 0.0
    property alias status: img.status

    // HACK: Alt text for a GIF is stored in the description iwth this prefix.
    readonly property string altPrefix: "Alt: "

    id: frame
    width: filter.imageVisible() ? img.width : parent.width
    height: filter.imageVisible() ? img.height : filter.height
    maskColor: backgroundColor

    Rectangle {
        id: frameBackground
        anchors.fill: parent
        color: guiSettings.highLightColor(backgroundColor)
    }

    ThumbAnimatedImageView {
        id: img
        width: aspectRatio > 0.0 ? calcWidth() : frame.parent.width
        height: aspectRatio > 0.0 ? width * aspectRatio : Math.min(width, guiSettings.maxImageHeight)
        Layout.fillWidth: true
        fillMode: Image.PreserveAspectFit
        sourceSize.width: width * Screen.devicePixelRatio
        sourceSize.height: height * Screen.devicePixelRatio
        url: filter.imageVisible() ? frame.url : ""
        showAlt: hasAlt()

        onStatusChanged: {
            if (status == Image.Ready)
                imageUtils.setDominantColor(img, (color) => { frameBackground.color = color })
        }

        function calcWidth() {
            let w = Math.min(frame.parent.width, imgSize.width)
            const h = w * aspectRatio

            if (h > guiSettings.maxImageHeight)
                w *= (guiSettings.maxImageHeight / h)

            return w
        }
    }
    SkyMouseArea {
        anchors.fill: img
        enabled: filter.imageVisible()

        onClicked: {
            if (img.status == Image.Ready)
                fullImageLoader.show(0, false)
            else
                root.viewFullAnimatedImage(url, title, null, () => {})
        }
    }
    FilteredImageWarning {
        id: filter
        width: parent.width
        contentVisibility: frame.contentVisibility
        contentWarning: frame.contentWarning
        contentLabeler: frame.contentLabeler
        imageUrl: frame.url
    }

    FullImageViewLoader {
        id: fullImageLoader
        thumbImageViewList: [img]
        isAnimatedImage: true
        animatedImageAlt: getAltText()
    }

    SkyImageUtils {
        id: imageUtils
    }

    function hasAlt() {
        return description.startsWith(altPrefix)
    }

    function getAltText() {
        if (hasAlt())
            return description.slice(altPrefix.length)

        return title
    }

    function getFilter() {
        return filter
    }
}
