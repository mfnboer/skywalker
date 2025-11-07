import QtQuick
import QtQuick.Controls
import skywalker

Rectangle {
    property int statistic: -1
    property SvgImage svg
    property string iconColor: guiSettings.statsColor
    property bool blinking: false

    signal clicked
    signal pressAndHold(MouseEvent event)

    id: control
    height: statIcon.height
    width: statIcon.width + (statText.active ? statText.width : 0)
    color: "transparent"

    Accessible.role: Accessible.Button
    Accessible.onPressAction: if (enabled) emitClicked()

    SkySvg {
        id: statIcon
        anchors.left: parent.left
        width: guiSettings.statsIconHeight
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
            font.pointSize: guiSettings.scaledFont(7/8)
            text: statistic
        }
    }
    Loader {
        active: blinking || active

        sourceComponent: BlinkingOpacity {
            target: control
            running: blinking
        }
    }
    MouseArea {
        anchors.fill: parent
        onClicked: control.emitClicked()
        onPressAndHold: (mouseEvent) => control.emitPressAndHold(mouseEvent)
    }

    function emitClicked() {
        if (!blinking)
            control.clicked()
    }

    function emitPressAndHold(event) {
        if (!blinking)
            control.pressAndHold(event)
    }
}
