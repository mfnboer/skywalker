import QtQuick
import skywalker

RoundedFrame {
    required property int contentVisibility // QEnums::ContentVisibility
    required property string contentWarning
    property list<imageview> images
    readonly property int maxHeight: 1200
    property bool settingSize: false

    id: frame
    objectToRound: img
    width: filter.imageVisible() ? img.width : parent.width
    height: filter.imageVisible() ? img.height : filter.height

    ThumbImageView {
        id: img
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
    }
    MouseArea {
        enabled: filter.imageVisible()
        anchors.fill: img
        cursorShape: Qt.PointingHandCursor
        onClicked: root.viewFullImage(images, 0)
    }

    AccessibleImage {
        image: img
        alt: img.imageView.alt
        visible: filter.imageVisible()
    }

    FilteredImageWarning {
        id: filter
        width: parent.width
        contentVisibiliy: frame.contentVisibility
        contentWarning: frame.contentWarning
        images: frame.images
    }

    Component.onCompleted: {
        img.setSize()
    }
}
