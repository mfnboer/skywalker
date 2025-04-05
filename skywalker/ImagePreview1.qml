import QtQuick
import skywalker

Item {
    required property int contentVisibility // QEnums::ContentVisibility
    required property string contentWarning
    property list<imageview> images
    readonly property int maxHeight: 1200
    property bool settingSize: false
    property bool swipeMode: false
    property string maskColor: guiSettings.backgroundColor

    signal activateSwipe

    id: preview

    // NOTE: these width/height can be overruled by the parent.
    width: frame.width
    height: frame.height

    RoundCornerMask {
        id: frame
        cornerRadius: swipeMode ? 0 : 10
        anchors.horizontalCenter: parent.horizontalCenter
        width: filter.imageVisible() ? (img.item ? img.item.width : 0) : parent.width
        height: filter.imageVisible() ? (img.item ? img.item.height : 0) : filter.height
        maskColor: preview.maskColor

        Loader {
            id: img
            z: parent.z - 1
            active: filter.imageVisible()

            sourceComponent: images[0].width > 0 && images[0].height > 0 && frame.parent.width > 0 ?
                                 knownSizeComp : unknownSizeComp
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
                    root.viewFullImage(images, 0)
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

    Component {
        id: unknownSizeComp

        ThumbImageUnknownSizeView {
            maxWidth: frame.parent.width
            maxHeight: preview.maxHeight
            image: images[0]
        }
    }

    Component {
        id: knownSizeComp

        ThumbImageKnownSizeView {
            maxWidth: frame.parent.width
            maxHeight: preview.maxHeight
            image: images[0]
        }
    }
}
