import QtQuick
import QtQuick.Controls.Material
import skywalker

Row {
    required property string text
    property SvgImage svg: SvgOutline.reply

    id: row
    width: parent.width
    spacing: 5

    SkySvg {
        id: replyImg
        width: parent.visible ? guiSettings.appFontHeight * 7/8 : 0
        height: width
        color: Material.color(Material.Grey)
        svg: row.svg
    }

    SkyCleanedTextLine {
        id: replyToText
        width: parent.width - replyImg.width
        anchors.verticalCenter: parent.verticalCenter
        elide: Text.ElideRight
        color: Material.color(Material.Grey)
        font.pointSize: guiSettings.scaledFont(7/8)
        plainText: row.text

        Accessible.ignored: true
    }

}
