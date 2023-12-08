import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

RoundedFrame {
    required property int contentVisibility // QEnums::ContentVisibility
    required property string contentWarning
    required property string url
    property string title

    id: frame
    objectToRound: img
    width: filter.imageVisible() ? img.width : parent.width
    height: filter.imageVisible() ? img.height : filter.height

    ThumbAnimatedImageView {
        id: img
        width: Math.min(implicitWidth, frame.parent.width)
        Layout.fillWidth: true
        fillMode: Image.PreserveAspectFit
        url: frame.url

        onImplicitWidthChanged: console.debug("IMAGE WIDTH:", implicitWidth)
    }
    MouseArea {
        enabled: filter.imageVisible()
        anchors.fill: img
        cursorShape: Qt.PointingHandCursor
        onClicked: root.viewFullAnimatedImage(url, title)
    }

    FilteredImageWarning {
        id: filter
        width: parent.width
        contentVisibiliy: frame.contentVisibility
        contentWarning: frame.contentWarning
        imageUrl: frame.url
    }
}
