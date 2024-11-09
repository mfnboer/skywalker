import QtQuick

Rectangle {
    required property int counter

    x: parent.width - 17
    y: -parent.y + 6
    width: Math.max(counterText.width + 10, height)
    height: 20
    radius: 8
    color: GuiSettings.badgeColor
    border.color: GuiSettings.badgeBorderColor
    border.width: 2
    visible: counter > 0

    Text {
        id: counterText
        anchors.centerIn: parent
        font.bold: true
        font.pointSize: GuiSettings.scaledFont(6/8)
        color: GuiSettings.badgeTextColor
        text: counter
    }

}
