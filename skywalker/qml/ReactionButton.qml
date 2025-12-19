import QtQuick
import QtQuick.Controls.Material
import skywalker

SkyButton {
    width: 40
    height: width
    padding: 0
    Material.background: "transparent"
    font.pointSize: guiSettings.scaledFont(2)
    font.family: UnicodeFonts.getEmojiFontFamily()
    textColor: guiSettings.textColor
}
