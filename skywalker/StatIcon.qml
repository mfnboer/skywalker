import QtQuick
import QtQuick.Controls
import skywalker

Rectangle {
    property int statistic: -1
    property SvgImage svg
    property string iconColor: GuiSettings.statsColor
    property var onClicked

    signal clicked()

    id: control
    height: statIcon.height
    width: statIcon.width + (statText.active ? statText.width : 0)
    color: "transparent"

    Accessible.role: Accessible.Button
    Accessible.onPressAction: if (enabled) clicked()

    SkySvg {
        id: statIcon
        anchors.left: parent.left
        width: GuiSettings.statsHeight
        height: width
        color: iconColor
        svg: parent.svg
    }
    Loader {
        id: statText
        anchors.left: statIcon.right
        active: statistic >= 0

        sourceComponent: Text {
            topPadding: 2
            bottomPadding: 2
            color: iconColor
            font.pointSize: GuiSettings.scaledFont(7/8)
            text: statistic
        }
    }
    MouseArea {
        anchors.fill: parent
        onClicked: control.clicked()
    }

}
