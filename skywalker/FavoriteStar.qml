import QtQuick
import skywalker

SkySvg {
    x: parent.width - width / 2
    y: height - height / 2
    height: width
    color: guiSettings.favoriteColor
    svg: SvgFilled.star

}
