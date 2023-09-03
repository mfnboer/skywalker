import QtQuick
import skywalker

Rectangle {
    property int statistic: -1
    property svgimage svg

    height: statText.height

    SvgImage {
        id: statIcon
        anchors.left: parent.left
        width: statText.height
        height: statText.height
        svg: parent.svg
    }
    Text {
        id: statText
        anchors.left: statIcon.right
        text: statistic
        visible: statistic >= 0
    }
    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
    }
}
