import QtQuick
import QtQuick.Controls

Menu {
    width: 220
    modal: true
    topMargin:  guiSettings.headerMargin
    bottomMargin: guiSettings.footerMargin

    onAboutToShow: {
        background.color = guiSettings.menuColor
        root.enablePopupShield(true)
    }

    onAboutToHide: root.enablePopupShield(false)
}
