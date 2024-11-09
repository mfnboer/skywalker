import QtQuick
import skywalker

SkySvg {
    x: parent.width - width / 2
    y: height - height / 2
    height: width
    color: GuiSettings.favoriteColor
    svg: SvgFilled.star

}
