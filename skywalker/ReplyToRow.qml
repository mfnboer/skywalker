import QtQuick
import QtQuick.Controls
import skywalker

Row {
    required property string authorName

    id: row

    SvgImage {
        id: replyImg
        width: 18 //replyToText.height
        height: width
        color: Material.color(Material.Grey)
        svg: svgOutline.reply
    }

    SkyCleanedText {
        id: replyToText
        anchors.verticalCenter: parent.verticalCenter
        width: parent.width - replyImg.width
        elide: Text.ElideRight
        color: Material.color(Material.Grey)
        font.pointSize: guiSettings.scaledFont(7/8)
        plainText: qsTr(`Reply to ${authorName}`)

        Accessible.ignored: true
    }
}
