import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

RoundedFrame {
    property list<imageview> images

    id: frame
    objectToRound: img
    width: parent.width
    height: img.height

    ThumbImageView {
        id: img
        width: parent.width
        Layout.fillWidth: true
        fillMode: Image.PreserveAspectFit
        imageView: images[0]

        onWidthChanged: setHeight()

        function setHeight() {
            let image = images[0]
            if (image.width > 0 && image.height > 0)
                heigth = image.height / image.width * width
        }
    }
    MouseArea {
        anchors.fill: img
        cursorShape: Qt.PointingHandCursor
        onClicked: root.viewFullImage(images, 0)
    }

    Component.onCompleted: {
        img.setHeight()
    }
}
