import QtQuick
import QtQuick.Controls
import skywalker

Row {
    required property string text
    property svgimage svg: svgOutline.reply

    id: row
    width: parent.width
    spacing: 5

    SkySvg {
        id: replyImg
        width: parent.visible ? replyToText.height : 0
        height: width
        color: Material.color(Material.Grey)
        svg: row.svg
    }

    SkyCleanedText {
        id: replyToText
        width: parent.width - implicitHeight
        anchors.verticalCenter: parent.verticalCenter
        elide: Text.ElideRight
        color: Material.color(Material.Grey)
        font.pointSize: guiSettings.scaledFont(7/8)
        plainText: row.text

        Accessible.ignored: true
    }
}
