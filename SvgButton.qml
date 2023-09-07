import QtQuick
import QtQuick.Controls
import skywalker

RoundButton {
    id: button
    property string iconColor
    property svgimage svg

    Material.background: "black"
    opacity: 0.7

    SvgImage {
        width: parent.width - 20
        height: parent.height - 20
        x: parent.x + 10
        y: parent.y + height + 10
        svg: button.svg
        color: button.iconColor
    }
}
