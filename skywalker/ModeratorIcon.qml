import QtQuick
import skywalker

SvgImage {
    x: parent.width - width * 0.8
    y: height - height * 0.2
    height: width
    color: guiSettings.moderatorIconColor
    outlineColor: guiSettings.backgroundColor
    svg: svgFilled.moderator

    GuiSettings {
        id: guiSettings
    }
}
