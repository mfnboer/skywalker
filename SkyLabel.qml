import QtQuick
import QtQuick.Controls

Label {
    property string backgroudColor: guiSettings.labelColor

    padding: 3
    background: Rectangle { color: backgroudColor }
    font.pointSize: guiSettings.labelFontSize

    GuiSettings {
        id: guiSettings
    }
}
