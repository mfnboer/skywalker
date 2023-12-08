import QtQuick
import QtQuick.Controls
import skywalker

AnimatedImageAutoRetry {
    required property string url

    id: img
    playing: skywalker.getUserSettings().getGifAutoPlay()
    source: url

    Label {
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        leftInset: 10
        rightInset: 10
        topInset: 10
        bottomInset: 10
        padding: 12
        color: "white"
        text: qsTr("GIF", "label on GIF")

        background: Rectangle {
            radius: 5
            color: "black"
            opacity: 0.6
        }
    }
}
