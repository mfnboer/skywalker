import QtQuick
import QtQuick.Controls

MenuItem {
    height: visible ? implicitHeight : 0
    Accessible.role: Accessible.MenuItem
    Accessible.name: text
    Accessible.description: Accessible.name
    Accessible.onPressAction: triggered()

    Component.onCompleted: contentItem.color = guiSettings.textColor
}
