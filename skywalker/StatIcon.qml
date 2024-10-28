import QtQuick
import QtQuick.Controls
import skywalker

Rectangle {
    property int statistic: -1
    property SvgImage svg
    property string iconColor: guiSettings.statsColor
    property var onClicked

    signal clicked()

    id: control
    height: statText.height
    width: statIcon.width + (statText.visible ? statText.width : 0)
    color: "transparent"

    Accessible.role: Accessible.Button
    Accessible.onPressAction: if (enabled) clicked()

    SkySvg {
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
        topPadding: 2
        bottomPadding: 2
        color: iconColor
        font.pointSize: guiSettigs.scaledFont(7/8)
        text: statistic
        visible: statistic >= 0
    }
    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: control.clicked()
    }

    GuiSettings {
        id: guiSettigs
    }
}
