import QtQuick
import QtQuick.Controls.Material

SkyButton {
    width: 40
    height: width
    padding: 0
    Material.background: "transparent"
    font.pointSize: guiSettings.scaledFont(2)
    textColor: guiSettings.textColor
    onClicked: console.debug("CLICKED:", text)
}
