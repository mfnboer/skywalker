import QtQuick

Text {
    textFormat: Text.PlainText
    font.pointSize: guiSettings.scaledFont(1)
    elide: Text.ElideRight
    color: guiSettings.textColor

    Accessible.role: Accessible.StaticText
    Accessible.name: text
    Accessible.description: text
}
