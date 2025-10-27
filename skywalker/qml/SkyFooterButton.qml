import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import skywalker

Rectangle {
    property SvgImage svg
    property int counter: 0
    property string counterBackgroundColor: guiSettings.badgeColor
    property string counterBorderColor: guiSettings.badgeBorderColor
    property string counterTextColor: guiSettings.badgeTextColor

    signal clicked

    id: button
    color: "transparent"

    Accessible.role: Accessible.PageTab
    Accessible.onPressAction: clicked()

    SkySvg {
        id: icon
        y: height + 5
        width: height
        height: button.height - 10
        anchors.horizontalCenter: button.horizontalCenter
        color: guiSettings.textColor
        svg: button.svg

        BadgeCounter {
            color: button.counterBackgroundColor
            border.color: button.counterBorderColor
            counterColor: button.counterTextColor
            counter: button.counter
        }
    }

    MouseArea {
        anchors.fill: button
        onClicked: button.clicked()
    }
}
