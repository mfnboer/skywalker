import QtQuick
import QtQuick.Controls
import skywalker

RoundButton {
    id: button
    required property string iconColor
    required property svgimage svg
    property int imageMargin: 10

    Material.background: "black"
    opacity: 0.7

    SvgImage {
        width: button.width - 2 * button.imageMargin
        height: button.height - 2 * button.imageMargin
        x: button.imageMargin
        y: height + button.imageMargin
        svg: button.svg
        color: button.iconColor
    }
}
