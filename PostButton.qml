import QtQuick
import QtQuick.Controls
import skywalker

SvgButton {
    property string initialText
    property var overrideOnClicked

    width: 70
    height: width
    iconColor: guiSettings.buttonTextColor
    Material.background: guiSettings.buttonColor
    opacity: 0.6
    imageMargin: 20
    svg: svgOutline.chat
    onClicked: {
        if (overrideOnClicked)
            overrideOnClicked()
        else
            root.composePost(initialText)
    }
}
