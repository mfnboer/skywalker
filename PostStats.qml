import QtQuick
import skywalker

Row {
    required property int replyCount
    required property int repostCount
    required property int likeCount
    required property string repostUri
    required property string likeUri

    signal reply()
    signal repost()
    signal like()
    signal more()

    StatIcon {
        width: parent.width / 4
        iconColor: "grey"
        svg: svgOutline.reply
        statistic: replyCount
        onClicked: reply()
    }
    StatIcon {
        width: parent.width / 4
        iconColor: repostUri ? "palevioletred" : "grey"
        svg: svgOutline.repost
        statistic: repostCount
        onClicked: repost()
    }
    StatIcon {
        width: parent.width / 4
        iconColor: likeUri ? "palevioletred" : "grey"
        svg: likeUri ? svgFilled.like : svgOutline.like
        statistic: likeCount
        onClicked: like()
    }
    StatIcon {
        width: parent.width / 4
        iconColor: "grey"
        svg: svgOutline.moreVert
        onClicked: more()
    }
}
