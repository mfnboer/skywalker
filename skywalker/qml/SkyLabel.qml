import QtQuick
import QtQuick.Controls

Label {
    property string backgroundColor: guiSettings.labelColor
    property double backgroundOpacity: 1.0
    property double labelFontHeight: guiSettings.labelFontHeight
    property double labelHeight: guiSettings.labelHeight

    topPadding: (labelHeight - labelFontHeight) / 2
    bottomPadding: topPadding
    leftPadding: 2
    rightPadding: 2
    background: Rectangle { color: backgroundColor; radius: 2; opacity: backgroundOpacity }
    font.pointSize: guiSettings.labelFontSize

    Accessible.role: Accessible.StaticText
    Accessible.name: text
}
