import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

RoundedFrame {
    required property string url
    property string title
    required property int contentVisibility // QEnums::ContentVisibility
    required property string contentWarning
    property alias status: img.status

    id: frame
    objectToRound: img
    width: filter.imageVisible() ? img.width : parent.width
    height: filter.imageVisible() ? img.height : filter.height

    ThumbAnimatedImageView {
        id: img
        width: Math.min(implicitWidth, frame.parent.width)
        Layout.fillWidth: true
        fillMode: Image.PreserveAspectFit
        url: filter.imageVisible() ? frame.url : ""
    }
    MouseArea {
        anchors.fill: img
        cursorShape: Qt.PointingHandCursor
        enabled: filter.imageVisible()
        onClicked: root.viewFullAnimatedImage(url, title)
    }
    FilteredImageWarning {
        id: filter
        width: parent.width
        contentVisibility: frame.contentVisibility
        contentWarning: frame.contentWarning
        imageUrl: frame.url
    }
}
