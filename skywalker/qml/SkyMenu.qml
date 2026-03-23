import QtQuick
import QtQuick.Controls

Menu {
    property real menuWidth: 220

    width: guiSettings.scaleWidthToFont(menuWidth)
    modal: true
    topMargin:  guiSettings.headerMargin
    bottomMargin: guiSettings.footerMargin

    enter: Transition {
        NumberAnimation { property: "scale"; from: 0; to: 1; easing.type: Easing.InOutQuad; duration: 100 }
    }

    exit: Transition {
        NumberAnimation { property: "scale"; from: 1; to: 0; easing.type: Easing.InOutQuad; duration: 100 }
    }

    delegate: AccessibleMenuItem {
        width: undefined
    }

    onAboutToShow: {
        background.color = guiSettings.menuColor
    }
}
