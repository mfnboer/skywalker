import QtQuick
import QtQuick.Controls
import skywalker

Item {
    required property SvgImage svg
    required property string text
    property string color: enabled ? guiSettings.menuTextColor : guiSettings.menuDisabledColor
    property string svgColor: color
    property var popup

    signal clicked

    id: button
    width: parent.width
    height: visible ? label.height + 20 : 0

    SkySvg {
        id: icon
        x: 10
        y: (svg.offsetByHeight ? height : 0) + 5
        height: label.height + 10
        width: height
        svg: button.svg
        color: button.svgColor
    }

    Text {
        id: label
        y: 10
        anchors.left: icon.right
        anchors.right: parent.right
        leftPadding: 10
        rightPadding: 10
        color: button.color
        elide: Text.ElideRight
        text: button.text
    }

    MouseArea {
        anchors.fill: parent
        enabled: button.enabled
        onClicked: {
            button.clicked()

            if (popup)
                popup.close()
        }
    }
}
