import QtQuick
import QtQuick.Controls
import skywalker

SvgButton {
    property var overrideOnClicked

    width: 70
    height: width
    opacity: 0.6
    imageMargin: 20
    svg: svgOutline.chat
    onClicked: {
        if (overrideOnClicked)
            overrideOnClicked()
        else
            root.composePost()
    }
}
