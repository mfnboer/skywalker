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

    Text {
        id: replyToText
        anchors.verticalCenter: parent.verticalCenter
        width: parent.width - replyImg.width
        textFormat: Text.RichText
        elide: Text.ElideRight
        color: Material.color(Material.Grey)
        font.pointSize: guiSettings.scaledFont(7/8)
        text: qsTr(`Reply to ${(unicodeFonts.toCleanedHtml(authorName))}`)
    }

    UnicodeFonts {
        id: unicodeFonts
    }
}
