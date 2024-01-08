import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Rectangle {
    required property svgimage svg
    required property string text
    required property var list

    id: emptyListIndication
    width: parent.width
    height: visible ? noListsImage.height + noListsText.height : 0
    color: "transparent"
    visible: list.count === 0

    SvgImage {
        id: noListsImage
        width: 150
        height: 150
        anchors.horizontalCenter: parent.horizontalCenter
        color: Material.color(Material.Grey)
        svg: emptyListIndication.svg
    }
    Text {
        id: noListsText
        y: noListsImage.y
        anchors.horizontalCenter: parent.horizontalCenter
        font.pointSize: guiSettings.scaledFont(10/8)
        color: Material.color(Material.Grey)
        elide: Text.ElideRight
        text: emptyListIndication.text
    }

    GuiSettings {
        id: guiSettings
    }
}
