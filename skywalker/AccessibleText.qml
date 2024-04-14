import QtQuick

Text {
    color: guiSettings.textColor
    Accessible.role: Accessible.StaticText
    Accessible.name: text
    Accessible.description: text

    GuiSettings {
        id: guiSettings
    }
}
