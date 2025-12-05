import QtQuick
import QtQuick.Layouts

AccessibleText {
    id: globalContentFilters
    width: parent.width
    Layout.fillWidth: true
    padding: 10
    font.pointSize: guiSettings.scaledFont(9/8)
    font.bold: true
    elide: Text.ElideRight

    Rectangle {
        anchors.fill: parent
        z: parent.z - 1
        color: guiSettings.headerHighLightColor
    }
}

