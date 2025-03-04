import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Rectangle {
    required property SvgImage svg
    required property string text
    required property var list

    signal linkActivated(string link)
    signal retry()

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
        svg: Boolean(list.error) ? SvgOutline.warning : emptyListIndication.svg
    }
    Text {
        id: noListsText
        y: noListsImage.y
        width: parent.width
        leftPadding: 10
        rightPadding: 10
        horizontalAlignment: Text.AlignHCenter
        font.pointSize: guiSettings.scaledFont(10/8)
        color: Material.color(Material.Grey)
        wrapMode: Text.Wrap
        elide: Text.ElideRight
        textFormat: Text.RichText
        text: Boolean(list.error) ? list.error : emptyListIndication.text

        onLinkActivated: (link) => emptyListIndication.linkActivated(link)
    }
    Text {
        anchors.top: noListsText.bottom
        anchors.topMargin: 10
        width: parent.width
        leftPadding: 10
        rightPadding: 10
        horizontalAlignment: Text.AlignHCenter
        textFormat: Text.RichText
        text: qsTr(`<a href="link" style="color: ${guiSettings.linkColor}; text-decoration: none">Retry</a>`)
        visible: Boolean(list.error)
        onLinkActivated: emptyListIndication.retry()
    }
}
