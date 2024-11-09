import QtQuick
import QtQuick.Layouts

AccessibleText {
    id: globalContentFilters
    width: parent.width
    Layout.fillWidth: true
    padding: 10
    font.pointSize: GuiSettings.scaledFont(9/8)
    font.bold: true
    color: GuiSettings.textColor

    Rectangle {
        anchors.fill: parent
        z: parent.z - 1
        color: GuiSettings.headerHighLightColor
    }

}

