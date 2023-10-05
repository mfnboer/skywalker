import QtQuick
import QtQuick.Controls
import skywalker

Row {
    required property string authorName

    SvgImage {
        id: replyImg
        width: replyToText.height
        height: replyToText.height
        color: "grey"
        svg: svgOutline.reply
    }

    Text {
        id: replyToText
        width: parent.width - replyImg.width
        elide: Text.ElideRight
        color: "grey"
        font.pointSize: guiSettings.scaledFont(7/8)
        text: qsTr(`Reply to ${authorName}`)
    }
}
