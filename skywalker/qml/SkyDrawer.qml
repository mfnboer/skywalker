import QtQuick
import QtQuick.Controls

Drawer {
    dragMargin: 0
    modal: true

    onAboutToShow: root.enablePopupShield(true)
    onAboutToHide: root.enablePopupShield(false)

    Component.onCompleted: background.color = guiSettings.menuColor
}
