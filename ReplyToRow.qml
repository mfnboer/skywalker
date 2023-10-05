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
        color: "grey"
        svg: svgOutline.reply
    }

    Text {
        id: replyToText
        anchors.verticalCenter: parent.verticalCenter
        width: parent.width - replyImg.width
        elide: Text.ElideRight
        color: "grey"
        font.pointSize: guiSettings.scaledFont(7/8)
        text: qsTr(`Reply to ${authorName}`)
    }
}
