import QtQuick
import QtQuick.Controls

MenuItem {
    property string textColor: enabled ? guiSettings.textColor : guiSettings.disabledColor

    height: visible ? implicitHeight : 0
    Accessible.role: Accessible.MenuItem
    Accessible.name: text
    Accessible.description: Accessible.name
    Accessible.onPressAction: triggered()

    onTextColorChanged: contentItem.color = textColor

    Component.onCompleted: contentItem.color = textColor
}
