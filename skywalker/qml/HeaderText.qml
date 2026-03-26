import QtQuick
import QtQuick.Layouts

AccessibleText {
    id: globalContentFilters
    width: parent.width
    Layout.fillWidth: true
    font.pointSize: guiSettings.scaledFont(9/8)
    font.bold: true
    elide: Text.ElideRight
}

