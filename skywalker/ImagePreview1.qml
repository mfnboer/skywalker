import QtQuick
import QtQuick.Controls
import skywalker

Item {
    required property int contentVisibility // QEnums::ContentVisibility
    required property string contentWarning
    property list<imageview> images
    readonly property int maxHeight: guiSettings.maxImageHeight
    property bool settingSize: false
    property bool swipeMode: false
    property string maskColor: guiSettings.backgroundColor
    readonly property bool imgSizeKnown: images[0].width > 0 && images[0].height > 0 && frame.parent.width > 0

    signal activateSwipe

    id: preview

    // NOTE: these width/height can be overruled by the parent.
    width: frame.width
    height: frame.height

    RoundCornerMask {
        id: frame
        cornerRadius: swipeMode ? 0 : 10
        x: (parent.width - width) / 2
        width: filter.imageVisible() ? (img.item ? img.item.width : 0) : parent.width
        height: filter.imageVisible() ? (img.item ? img.item.height : 0) : filter.height

        // Due to scaling the painted size can be smaller than the image size
        maskWidth: filter.imageVisible() ? (img.item ? getImageMaskWidth() : 0) : parent.width
        maskHeight: filter.imageVisible() ? (img.item ? getImageMakskHeight() : 0) : filter.height

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
            width: frame.parent.width
            height: width
            image: images[0]
        }
    }

    Component {
        id: knownSizeComp

        ThumbImageKnownSizeView {
            maxWidth: frame.parent.width
            maxHeight: preview.maxHeight
            image: images[0]
            noCrop: true
        }
    }

    function getImageMaskWidth() {
        return imgSizeKnown ? img.item.paintedWidth : img.item.width
    }

    function getImageMakskHeight() {
        return imgSizeKnown ? img.item.paintedHeight : img.item.height
    }
}
