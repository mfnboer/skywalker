import QtQuick
import skywalker

SvgImage {
    x: parent.width - width / 2
    y: height - height / 2
    height: width
    color: guiSettings.favoriteColor
    svg: svgFilled.star

    GuiSettings {
        id: guiSettings
    }
}
