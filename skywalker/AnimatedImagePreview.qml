import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

RoundedFrame {
    required property string url
    property string title
    property alias status: img.status

    id: frame
    objectToRound: img
    width: img.width
    height: img.height

    ThumbAnimatedImageView {
        id: img
        width: Math.min(implicitWidth, frame.parent.width)
        Layout.fillWidth: true
        fillMode: Image.PreserveAspectFit
        url: frame.url
    }
    MouseArea {
        anchors.fill: img
        cursorShape: Qt.PointingHandCursor
        onClicked: root.viewFullAnimatedImage(url, title)
    }
}
