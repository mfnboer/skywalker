import QtQuick
import QtQuick.Controls

Label {
    property string backgroundColor: GuiSettings.labelColor
    property double backgroundOpacity: 1.0

    topPadding: (GuiSettings.labelHeight - GuiSettings.labelFontHeight) / 2
    bottomPadding: topPadding
    leftPadding: 2
    rightPadding: 2
    background: Rectangle { color: backgroundColor; radius: 2; opacity: backgroundOpacity }
    font.pointSize: GuiSettings.labelFontSize

    Accessible.role: Accessible.StaticText
    Accessible.name: text

}
