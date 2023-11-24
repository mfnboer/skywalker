import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

RoundedFrame {
    required property int contentVisibility // QEnums::ContentVisibility
    required property string contentWarning
    property list<imageview> images

    id: frame
    objectToRound: img
    width: parent.width
    height: filter.imageVisible() ? img.height : filter.height

    ThumbImageView {
        id: img
        width: parent.width
        Layout.fillWidth: true
        fillMode: Image.PreserveAspectFit
        imageView: filter.getImage(0)

        onWidthChanged: setHeight()

        function setHeight() {
            let image = images[0]
            if (image.width > 0 && image.height > 0)
                height = image.height / image.width * width
        }
    }
    MouseArea {
        enabled: filter.imageVisible()
        anchors.fill: img
        cursorShape: Qt.PointingHandCursor
        onClicked: root.viewFullImage(images, 0)
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
