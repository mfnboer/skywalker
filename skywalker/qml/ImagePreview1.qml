import QtQuick
import QtQuick.Controls
import skywalker

Item {
    required property int contentVisibility // QEnums::ContentVisibility
    required property string contentWarning
    required property basicprofile contentLabeler
    property list<imageview> images
    readonly property int maxHeight: guiSettings.maxImageHeight
    property bool settingSize: false
    property bool swipeMode: false
    property string maskColor: guiSettings.backgroundColor
    readonly property bool imgSizeKnown: images[0].width > 0 && images[0].height > 0 && frame.parent.width > 0

    signal activateSwipe

    id: preview

    // NOTE: these width/height can be overruled by the parent.
    width: parent.width
    height: frame.height

    RoundCornerMask {
        id: frame
        cornerRadius: swipeMode ? 0 : guiSettings.radius
        x: (parent.width - width) / 2
        width: filter.imageVisible() ? (img.item ? img.item.width : 0) : parent.width
        height: filter.imageVisible() ? (img.item ? img.item.height : 0) : filter.height

        // Due to scaling the painted size can be smaller than the image size
        maskWidth: filter.imageVisible() ? (img.item ? getImageMaskWidth() : 0) : parent.width
        maskHeight: filter.imageVisible() ? (img.item ? getImageMaskHeight() : 0) : filter.height

        maskColor: preview.maskColor

        Loader {
            id: img
            z: parent.z - 1
            active: filter.imageVisible()

            sourceComponent: imgSizeKnown ? knownSizeComp : unknownSizeComp
        }

        MouseArea {
            enabled: filter.imageVisible()
            anchors.fill: img
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                if (img.item && img.item.failedCanReload)
                    img.item.reload()
                else if (swipeMode)
                    activateSwipe()
                else
                    fullImageLoader.show(0)
            }
        }

        FilteredImageWarning {
            id: filter
            x: swipeMode ? 10 : 0
            width: parent.width - x * 2
            contentVisibility: preview.contentVisibility
            contentWarning: preview.contentWarning
            contentLabeler: preview.contentLabeler
            images: preview.images
        }
    }

    FullImageViewLoader {
        id: fullImageLoader
        thumbImageViewList: [img.item]
        images: preview.images
    }

    Component {
        id: unknownSizeComp

        ThumbImageFixedSizeView {
            width: Math.min(preview.width, preview.maxHeight)
            height: width
            image: images[0]
        }
    }

    Component {
        id: knownSizeComp

        ThumbImageKnownSizeView {
            maxWidth: preview.width
            maxHeight: preview.maxHeight
            image: images[0]
            noCrop: true
        }
    }

    function getImageMaskWidth() {
        if (!imgSizeKnown)
            return img.item.width

        // HACK: the painted with seems sometimes 1 or 2 pixels off??
        if (img.item.width - img.item.paintedWidth <= 2)
            return img.item.width

        return img.item.paintedWidth
    }

    function getImageMaskHeight() {
        if (!imgSizeKnown)
            return img.item.height

        if (img.item.height - img.item.paintedHeight <= 2)
            return img.item.height

        return img.item.paintedHeight
    }
}
