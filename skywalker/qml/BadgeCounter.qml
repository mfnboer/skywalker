import QtQuick

Rectangle {
    required property int counter
    property string counterColor: guiSettings.badgeTextColor

    x: parent.width - 17
    y: -parent.y + 6
    width: Math.max(counterText.width + 10, height)
    height: 20
    radius: 8
    color: guiSettings.badgeColor
    border.color: guiSettings.badgeBorderColor
    border.width: 2
    visible: counter > 0

    Text {
        id: counterText
        anchors.centerIn: parent
        font.bold: true
        font.pointSize: guiSettings.scaledFont(6/8)
        color: counterColor
        text: counter
    }

}
