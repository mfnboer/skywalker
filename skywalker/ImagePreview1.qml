import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

RoundedFrame {
    required property int contentVisibility // QEnums::ContentVisibility
    required property string contentWarning
    property list<imageview> images
    readonly property int maxHeight: 1200

    id: frame
    objectToRound: img
    width: filter.imageVisible() ? img.width : parent.width
    height: filter.imageVisible() ? img.height : filter.height

    ThumbImageView {
        id: img
        width: Math.min(implicitWidth, frame.parent.width)
        Layout.fillWidth: true
        fillMode: Image.PreserveAspectFit
        imageView: filter.getImage(0)

        onWidthChanged: setHeight()

        function setHeight() {
            const image = images[0]

            if (image.width > 0 && image.height > 0) {
                const newHeight = image.height / image.width * width

                if (newHeight > maxHeight) {
                    fillMode = Image.PreserveAspectCrop
                    newHeight = maxHeight
                }

                height = newHeight
            }
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
        img.setHeight()
    }
}
