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

    // NOTE: these width/height can be overruled by that parent.
    width: frame.width
    height: frame.height

    RoundCornerMask {
        id: frame
        anchors.horizontalCenter: parent.horizontalCenter
        width: filter.imageVisible() ? img.width : parent.width
        height: filter.imageVisible() ? img.height : filter.height
        maskColor: preview.maskColor

        ThumbImageView {
            id: img
            z: parent.z - 1
            width: Math.min(implicitWidth, frame.parent.width)
            fillMode: Image.PreserveAspectFit
            imageView: filter.getImage(0)

            onWidthChanged: setSize()

            function setSize() {
                if (settingSize)
                    return

                settingSize = true
                const image = images[0]

                if (image.width > 0 && image.height > 0 && frame.parent.width > 0) {
                    const newWidth = Math.min(image.width, frame.parent.width)
                    let newHeight = (image.height / image.width) * newWidth

                    if (newHeight > maxHeight) {
                        fillMode = Image.PreserveAspectCrop
                        newHeight = maxHeight
                    }

                    height = newHeight
                    width = newWidth
                }

                settingSize = false
            }

            Loader {
                anchors.right: parent.right
                anchors.rightMargin: 5
                anchors.top: parent.top
                anchors.topMargin: 5
                active: swipeMode

                sourceComponent: SkySvg {
                    width: 20
                    height: 20
                    svg: SvgOutline.swipeVertical
                    color: "white"
                }
            }
        }
        MouseArea {
            enabled: filter.imageVisible()
            anchors.fill: img
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                if (img.failedCanReload)
                    img.reload()
                else if (swipeMode)
                    activateSwipe()
                else
                    root.viewFullImage(images, 0)
            }
        }

        FilteredImageWarning {
            id: filter
            width: parent.width
            contentVisibility: preview.contentVisibility
            contentWarning: preview.contentWarning
            images: preview.images
        }

        Component.onCompleted: {
            img.setSize()
        }
    }
}
