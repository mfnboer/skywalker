import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Rectangle {
    required property SvgImage svg
    required property string text
    required property var list

    signal linkActivated(string link)

    id: emptyListIndication
    width: parent.width
    height: visible ? noListsImage.height + noListsText.height : 0
    color: "transparent"
    visible: list.count === 0

    Accessible.role: Accessible.StaticText
    Accessible.name: text
    Accessible.description: Accessible.name

    SkySvg {
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
        width: parent.width
        horizontalAlignment: Text.AlignHCenter
        font.pointSize: GuiSettings.scaledFont(10/8)
        color: Material.color(Material.Grey)
        wrapMode: Text.Wrap
        elide: Text.ElideRight
        textFormat: Text.RichText
        text: emptyListIndication.text

        onLinkActivated: (link) => emptyListIndication.linkActivated(link)
    }

}
