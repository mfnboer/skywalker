import QtQuick
import skywalker

Text {
    color: guiSettings.textColor
    Accessible.role: Accessible.StaticText
    Accessible.name: text
    Accessible.description: text
}
