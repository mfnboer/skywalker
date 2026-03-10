import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

RoundCornerMask {
    required property string url
    property string title
    required property int contentVisibility // QEnums::ContentVisibility
    required property string contentWarning
    required property basicprofile contentLabeler
    property string backgroundColor: guiSettings.backgroundColor
    property size imgSize
    readonly property double aspectRatio: (imgSize.width > 0 && imgSize.height > 0) ? imgSize.height / imgSize.width : 0.0
    property alias status: img.status

    id: frame
    width: filter.imageVisible() ? img.width : parent.width
    height: filter.imageVisible() ? img.height : filter.height
    color: guiSettings.highLightColor(backgroundColor)
    maskColor: backgroundColor

    ThumbAnimatedImageView {
        id: img
        width: aspectRatio > 0.0 ? calcWidth() : frame.parent.width
        height: aspectRatio > 0.0 ? width * aspectRatio : Math.min(width, guiSettings.maxImageHeight)
        Layout.fillWidth: true
        fillMode: Image.PreserveAspectFit
        url: filter.imageVisible() ? frame.url : ""

        onStatusChanged: {
            if (status == Image.Ready)
                imageUtils.setDominantColor(img, (color) => { frame.color = color })
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
        animatedImageAlt: title
    }

    SkyImageUtils {
        id: imageUtils
    }

    function getFilter() {
        return filter
    }
}
