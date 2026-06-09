import QtQuick
import QtQuick.Controls
import skywalker

Item {
    required property int contentVisibility // QEnums::ContentVisibility
    required property string contentWarning
    required property basicprofile contentLabeler
    property list<imageview> images
    property int startImageIndex: 0
    property bool swipeMode: false
    property string maskColor: guiSettings.backgroundColor
    readonly property list<var> imgList: [img.item]

    signal activateSwipe(int imgIndex, var previewImg)
    signal showFullImage(int imgIndex, bool swipeMode)

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
        maskColor: preview.maskColor

        Loader {
            id: img
            z: parent.z - 1
            active: filter.imageVisible()

            sourceComponent: fixedSizeComp
        }

        SkyMouseArea {
            enabled: filter.imageVisible()
            anchors.fill: img
            onClicked: {
                if (img.item && img.item.failedCanReload)
                    img.item.reload()
                else
                    showFullImage(startImageIndex, swipeMode)
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

    Component {
        id: fixedSizeComp

        ThumbImageFixedSizeView {
            width: preview.width
            height: preview.height
            image: images[startImageIndex]
            fillMode: Image.PreserveAspectCrop
        }
    }

    function getFilter() {
        return filter
    }
}
