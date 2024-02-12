import QtQuick
import QtQuick.Controls

MenuItem {
    Accessible.role: Accessible.MenuItem
    Accessible.name: text
    Accessible.description: Accessible.name
    Accessible.onPressAction: triggered()
}
