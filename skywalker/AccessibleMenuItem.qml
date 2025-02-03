import QtQuick
import QtQuick.Controls

MenuItem {
    property string textColor: guiSettings.textColor

    height: visible ? implicitHeight : 0
    Accessible.role: Accessible.MenuItem
    Accessible.name: text
    Accessible.description: Accessible.name
    Accessible.onPressAction: triggered()

    Component.onCompleted: contentItem.color = textColor
}
