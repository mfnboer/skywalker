import QtQuick
import skywalker

SkySvg {
    x: parent.width - width * 0.8
    y: height - height * 0.2
    height: width
    color: guiSettings.moderatorIconColor
    outlineColor: guiSettings.backgroundColor
    svg: SvgFilled.moderator
}
