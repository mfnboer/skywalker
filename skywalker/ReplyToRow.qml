import QtQuick
import QtQuick.Controls
import skywalker

Row {
    required property string authorName

    id: row
    width: parent.width

    SvgImage {
        id: replyImg
        width: parent.visible ? replyToText.height : 0
        height: width
        color: Material.color(Material.Grey)
        svg: svgOutline.reply
    }

    SkyCleanedText {
        id: replyToText
        width: parent.width - implicitHeight
        anchors.verticalCenter: parent.verticalCenter
        elide: Text.ElideRight
        color: Material.color(Material.Grey)
        font.pointSize: guiSettings.scaledFont(7/8)
        plainText: qsTr(`Reply to ${authorName}`)

        Accessible.ignored: true
    }
}
