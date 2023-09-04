import QtQuick
import QtQuick.Controls
import skywalker

Rectangle {
    property int statistic: -1
    property svgimage svg
    property string iconColor

    height: statText.height
    color: "transparent"

    SvgImage {
        id: statIcon
        anchors.left: parent.left
        width: statText.height
        height: statText.height
        color: iconColor
        svg: parent.svg
    }
    Text {
        id: statText
        anchors.left: statIcon.right
        color: iconColor
        text: statistic
        visible: statistic >= 0
    }
    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
    }
}
