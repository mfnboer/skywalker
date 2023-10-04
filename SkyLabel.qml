import QtQuick
import QtQuick.Controls

Label {
    property string backgroudColor: guiSettings.labelColor

    padding: 3
    background: Rectangle { color: backgroudColor; radius: 2 }
    font.pointSize: guiSettings.labelFontSize

    GuiSettings {
        id: guiSettings
    }
}
