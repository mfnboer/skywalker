import QtQuick
import QtQuick.Controls

Drawer {
    dragMargin: 0
    modal: true
    topPadding: 20
    bottomPadding: edge == Qt.BottomEdge ? guiSettings.footerMargin + 20 : 0

    onAboutToShow: {
        background.color = guiSettings.menuColor
        root.enablePopupShield(true)
    }

    onAboutToHide: root.enablePopupShield(false)

    Rectangle {
        y: -10 - height / 2
        anchors.horizontalCenter: parent.horizontalCenter
        width: 40
        height: 4
        radius: height / 2
        color: guiSettings.buttonColor
        visible: edge == Qt.BottomEdge
    }
}
