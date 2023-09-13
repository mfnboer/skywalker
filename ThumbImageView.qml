import QtQuick
import QtQuick.Controls
import skywalker

Image {
    required property imageview imageView

    id: img
    source: imageView.thumbUrl

    Label {
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        leftInset: 10
        rightInset: 10
        topInset: 10
        bottomInset: 10
        padding: 12
        color: "white"
        text: qsTr("ALT", "alternative text indication on an image")
        visible: imageView.alt

        background: Rectangle {
            radius: 5
            color: "black"
            opacity: 0.6
        }
    }
}
