import QtQuick
import QtQuick.Layouts

AccessibleText {
    id: globalContentFilters
    width: parent.width
    Layout.fillWidth: true
    padding: 10
    font.pointSize: guiSettings.scaledFont(9/8)
    font.bold: true
    color: guiSettings.textColor

    Rectangle {
        anchors.fill: parent
        z: parent.z - 1
        color: guiSettings.headerHighLightColor
    }

    GuiSettings {
        id: guiSettings
    }
}

