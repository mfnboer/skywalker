import QtQuick
import QtQuick.Controls

Drawer {
    dragMargin: 0
    modal: true

    onAboutToShow: {
        background.color = guiSettings.menuColor
        root.enablePopupShield(true)
    }

    onAboutToHide: root.enablePopupShield(false)
}
