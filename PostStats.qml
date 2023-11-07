import QtQuick
import QtQuick.Controls
import skywalker

Row {
    required property int replyCount
    required property int repostCount
    required property int likeCount
    required property string repostUri
    required property string likeUri
    required property bool authorIsUser
    required property bool isBookmarked

    signal reply()
    signal repost()
    signal like()
    signal bookmark()
    signal share()
    signal deletePost()
    signal copyPostText()

    StatIcon {
        width: parent.width / 5
        iconColor: Material.color(Material.Grey)
        svg: svgOutline.reply
        statistic: replyCount
        onClicked: reply()
    }
    StatIcon {
        width: parent.width / 5
        iconColor: repostUri ? "palevioletred" : Material.color(Material.Grey)
        svg: svgOutline.repost
        statistic: repostCount
        onClicked: repost()
    }
    StatIcon {
        width: parent.width / 5
        iconColor: likeUri ? "palevioletred" : Material.color(Material.Grey)
        svg: likeUri ? svgFilled.like : svgOutline.like
        statistic: likeCount
        onClicked: like()
    }
    StatIcon {
        width: parent.width / 5
        iconColor: isBookmarked ? guiSettings.buttonColor : Material.color(Material.Grey)
        svg: isBookmarked ? svgFilled.bookmark : svgOutline.bookmark
        onClicked: bookmark()
    }
    StatIcon {
        width: parent.width / 5
        iconColor: Material.color(Material.Grey)
        svg: svgOutline.moreVert
        onClicked: moreMenu.open()

        Menu {
            id: moreMenu

            MenuItem {
                text: qsTr("Copy post text")
                onTriggered: copyPostText()

                MenuItemSvg {
                    svg: svgOutline.copy
                }
            }
            MenuItem {
                text: qsTr("Share")
                onTriggered: share()

                MenuItemSvg {
                    svg: svgOutline.share
                }
            }
            MenuItem {
                text: qsTr("Delete")
                enabled: authorIsUser
                onTriggered: deletePost()

                MenuItemSvg {
                    svg: svgOutline.delete
                    visible: parent.enabled
                }
            }
        }
    }

    GuiSettings {
        id: guiSettings
    }
}
