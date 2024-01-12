import QtQuick
import skywalker

SvgImage {
    y: height + 5
    anchors.rightMargin: 10
    anchors.right: parent.right
    width: height
    height: parent.height - 10
    color: guiSettings.textColor
    visible: parent.enabled

    GuiSettings {
        id: guiSettings
    }
}
