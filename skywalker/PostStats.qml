import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Column {
    required property int replyCount
    required property int repostCount
    required property int likeCount
    required property string repostUri
    required property string likeUri
    required property bool replyDisabled
    required property bool authorIsUser
    required property bool isBookmarked
    required property bool bookmarkNotFound

    signal reply()
    signal repost()
    signal like()
    signal bookmark()
    signal share()
    signal deletePost()
    signal copyPostText()
    signal reportPost()
    signal translatePost()

    Row {
        width: parent.width

        StatIcon {
            width: parent.width / 5
            iconColor: enabled ? guiSettings.statsColor : guiSettings.disabledColor
            svg: svgOutline.reply
            statistic: replyCount
            visible: !bookmarkNotFound
            enabled: !replyDisabled
            onClicked: reply()

            Accessible.name: (replyDisabled ? qsTr("reply not allowed") : qsTr("reply")) + statSpeech(replyCount, "reply", "replies")
        }
        StatIcon {
            width: parent.width / 5
            iconColor: repostUri ? guiSettings.likeColor : guiSettings.statsColor
            svg: svgOutline.repost
            statistic: repostCount
            visible: !bookmarkNotFound
            onClicked: repost()

            Accessible.name: qsTr("repost") + statSpeech(repostCount, "repost", "reposts")
        }
        StatIcon {
            width: parent.width / 5
            iconColor: likeUri ? guiSettings.likeColor : guiSettings.statsColor
            svg: likeUri ? svgFilled.like : svgOutline.like
            statistic: likeCount
            visible: !bookmarkNotFound
            onClicked: like()

            Accessible.name: qsTr("like") + statSpeech(likeCount, "like", "likes")
        }
        StatIcon {
            width: parent.width / 5
            iconColor: isBookmarked ? guiSettings.buttonColor : guiSettings.statsColor
            svg: isBookmarked ? svgFilled.bookmark : svgOutline.bookmark
            onClicked: bookmark()

            Accessible.name: isBookmarked ? qsTr("remove bookmark") : qsTr("bookmark")
        }
        StatIcon {
            width: parent.width / 5
            svg: svgOutline.moreVert
            visible: !bookmarkNotFound
            onClicked: moreMenu.open()

            Accessible.name: qsTr("more options")

            Menu {
                id: moreMenu
                modal: true

                onAboutToShow: root.enablePopupShield(true)
                onAboutToHide: root.enablePopupShield(false)

                CloseMenuItem {
                    text: qsTr("<b>Post</b>")
                    Accessible.name: qsTr("close more options menu")
                }
                AccessibleMenuItem {
                    text: qsTr("Translate")
                    onTriggered: translatePost()

                    MenuItemSvg { svg: svgOutline.googleTranslate }
                }

                AccessibleMenuItem {
                    text: qsTr("Copy post text")
                    onTriggered: copyPostText()

                    MenuItemSvg { svg: svgOutline.copy }
                }
                AccessibleMenuItem {
                    text: qsTr("Share")
                    onTriggered: share()

                    MenuItemSvg { svg: svgOutline.share }
                }
                AccessibleMenuItem {
                    text: qsTr("Delete")
                    enabled: authorIsUser
                    onTriggered: deletePost()

                    MenuItemSvg { svg: svgOutline.delete }
                }
                AccessibleMenuItem {
                    text: qsTr("Report post")
                    onTriggered: reportPost()

                    MenuItemSvg { svg: svgOutline.report }
                }
            }
        }
    }

    AccessibilityUtils {
        id: accessibilityUtils
    }

    GuiSettings {
        id: guiSettings
    }

    function statSpeech(stat, textSingular, textPlural) {
        return accessibilityUtils.statSpeech(stat, textSingular, textPlural)
    }
}
