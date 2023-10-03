import QtQuick
import QtQuick.Controls
import skywalker

SvgButton {
    property string initialText

    width: 70
    height: width
    iconColor: guiSettings.buttonTextColor
    Material.background: guiSettings.buttonColor
    opacity: 0.6
    imageMargin: 20
    svg: svgOutline.chat
    onClicked: root.composePost(initialText)
}
