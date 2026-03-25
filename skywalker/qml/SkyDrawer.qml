import QtQuick
import QtQuick.Controls

Drawer {
    dragMargin: 0
    modal: true
    topPadding: 20
    bottomPadding: edge == Qt.BottomEdge ? guiSettings.footerMargin + 20 : 0

    onAboutToShow: {
        background.color = guiSettings.menuColor
    }

    Rectangle {
        y: -topPadding / 2 - height / 2
        anchors.horizontalCenter: parent.horizontalCenter
        width: 40
        height: 4
        radius: height / 2
        color: guiSettings.separatorHighLightColor
        visible: edge == Qt.BottomEdge
    }

    Rectangle {
        x: -leftPadding / 2 - width / 2
        anchors.verticalCenter: parent.verticalCenter
        width: 4
        height: 40
        radius: width / 2
        color: guiSettings.separatorHighLightColor
        visible: edge == Qt.RightEdge
    }

    Rectangle {
        anchors.right: parent.right
        anchors.rightMargin: -rightPadding / 2 - width / 2
        anchors.verticalCenter: parent.verticalCenter
        width: 4
        height: 40
        radius: width / 2
        color: guiSettings.separatorHighLightColor
        visible: edge == Qt.LeftEdge
    }
}
