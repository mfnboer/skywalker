import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

RoundedFrame {
    required property string url
    property string title
    required property int contentVisibility // QEnums::ContentVisibility
    required property string contentWarning
    required property basicprofile contentLabeler
    property size imgSize
    readonly property double aspectRatio: (imgSize.width > 0 && imgSize.height > 0) ? imgSize.height / imgSize.width : 0.0
    property alias status: img.status

    id: frame
    objectToRound: img
    width: filter.imageVisible() ? img.width : parent.width
    height: filter.imageVisible() ? img.height : filter.height

    ThumbAnimatedImageView {
        id: img
        width: aspectRatio > 0.0 ? calcWidth() : Math.min(implicitWidth, frame.parent.width)
        height: aspectRatio > 0.0 ? width * aspectRatio : undefined
        Layout.fillWidth: true
        fillMode: Image.PreserveAspectFit
        url: filter.imageVisible() ? frame.url : ""

        function calcWidth() {
            let w = Math.min(frame.parent.width, imgSize.width)
            const h = w * aspectRatio

            if (h > guiSettings.maxImageHeight)
                w *= (guiSettings.maxImageHeight / h)

            return w
        }
    }
    MouseArea {
        anchors.fill: img
        scrollGestureEnabled: false
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
}
