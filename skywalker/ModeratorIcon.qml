import QtQuick
import skywalker

SvgImage {
    x: parent.width - width * 0.9
    y: height - height * 0.1
    height: width
    color: guiSettings.favoriteColor
    svg: svgFilled.moderator

    GuiSettings {
        id: guiSettings
    }
}
