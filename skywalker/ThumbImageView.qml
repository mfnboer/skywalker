import QtQuick
import QtQuick.Controls
import skywalker

ImageAutoRetry {
    required property imageview imageView

    id: img
    source: imageView.thumbUrl

    Label {
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        leftInset: 4
        rightInset: 4
        topInset: 5
        bottomInset: 5
        padding: 5
        color: "white"
        font.pointSize: guiSettings.scaledFont(4.5/8)
        text: qsTr("ALT", "alternative text indication on an image")
        visible: imageView.alt

        background: Rectangle {
            radius: 3
            color: "black"
            opacity: 0.6
        }
    }

    GuiSettings {
        id: guiSettings
    }
}
