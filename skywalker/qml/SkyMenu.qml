import QtQuick
import QtQuick.Controls

Menu {
    property real menuWidth: 220

    width: guiSettings.scaleWidthToFont(menuWidth)
    modal: true
    topMargin:  guiSettings.headerMargin
    bottomMargin: guiSettings.footerMargin

    onAboutToShow: {
        background.color = guiSettings.menuColor
        root.enablePopupShield(true)
    }

    onAboutToHide: root.enablePopupShield(false)
}
