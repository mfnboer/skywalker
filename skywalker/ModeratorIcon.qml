import QtQuick
import skywalker

SkySvg {
    x: parent.width - width * 0.8
    y: height - height * 0.2
    height: width
    color: GuiSettings.moderatorIconColor
    outlineColor: GuiSettings.backgroundColor
    svg: SvgFilled.moderator

}
