import QtQuick
import QtQuick.Controls
import skywalker

Item {
    required property int replyCount
    required property int repostCount
    required property int likeCount
    required property string repostUri
    required property string likeUri
    required property bool likeTransient
    required property bool threadMuted
    required property bool replyDisabled
    required property bool viewerStatePinned
    required property int replyRestriction // QEnums::ReplyRestriction flags
    required property bool isHiddenReply
    required property bool isReply
    required property string replyRootAuthorDid
    required property string replyRootUri
    required property bool authorIsUser
    required property bool isBookmarked
    required property bool bookmarkTransient
    required property bool isThread
    property bool isUnrolledThread: false
    property bool showViewThread: false
    property var record: null // recordview
    property var recordWithMedia: null // record_with_media_view
    property bool feedAcceptsInteractions: false
    property bool limitedStats: false
    property string color: guiSettings.statsColor
    property int topPadding: 0

    signal reply()
    signal repost()
    signal quotePost()
    signal like()
    signal likeLongPress(MouseEvent mouseEvent)
    signal viewThread()
    signal unrollThread()
    signal muteThread()
    signal bookmark()
    signal bookmarkLongPress(MouseEvent mouseEvent)
    signal share()
    signal threadgate()
    signal hideReply()
    signal deletePost()
    signal copyPostText()
    signal reportPost()
    signal translatePost()
    signal detachQuote(string uri, bool detach)
    signal pin()
    signal unpin()
    signal blockAuthor()
    signal showEmojiNames()
    signal showMoreLikeThis()
    signal showLessLikeThis()

    id: postStats
    height: replyIcon.height + topPadding

    StatIcon {
        id: replyIcon
        y: topPadding
        width: parent.width / 4
        iconColor: enabled ? postStats.color : guiSettings.disabledColor
        svg: SvgOutline.reply
        statistic: replyCount
        visible: !limitedStats
        enabled: !replyDisabled
        onClicked: reply()

        Accessible.name: (replyDisabled ? qsTr("reply not allowed") : qsTr("reply")) + statSpeech(replyCount, "reply", "replies")
    }
    StatIcon {
        id: repostIcon
        y: topPadding
        anchors.left: replyIcon.right
        width: parent.width / 4
        iconColor: repostUri ? guiSettings.likeColor : postStats.color
        svg: SvgOutline.repost
        statistic: repostCount
        visible: !limitedStats
        onClicked: repost()
        onPressAndHold: quotePost()

        Accessible.name: qsTr("repost") + statSpeech(repostCount, "repost", "reposts")
    }
    StatIcon {
        id: likeIcon
        y: topPadding
        anchors.left: !limitedStats ? repostIcon.right : parent.left
        width: parent.width / 4
        iconColor: likeUri ? guiSettings.likeColor : postStats.color
        svg: likeUri ? SvgFilled.like : SvgOutline.like
        statistic: likeCount
        onClicked: like()
        onPressAndHold: (mouseEvent) => likeLongPress(mouseEvent)

        Accessible.name: qsTr("like") + statSpeech(likeCount, "like", "likes")

        Loader {
            active: likeTransient || active

            BlinkingOpacity {
                target: likeIcon
                running: likeTransient
            }
        }
    }
    StatIcon {
        id: bookmarkIcon
        y: topPadding
        anchors.left: likeIcon.right
        width: parent.width / 8
        iconColor: isBookmarked ? guiSettings.buttonColor : postStats.color
        svg: isBookmarked ? SvgFilled.bookmark : SvgOutline.bookmark
        visible: !limitedStats
        onClicked: bookmark()
        onPressAndHold: (mouseEvent) => bookmarkLongPress(mouseEvent)

        Accessible.name: isBookmarked ? qsTr("remove bookmark") : qsTr("bookmark")

        Loader {
            active: bookmarkTransient || active

            sourceComponent: BlinkingOpacity {
                target: bookmarkIcon
                running: bookmarkTransient
            }
        }
    }
    StatIcon {
        id: moreIcon
        y: topPadding
        anchors.right : parent.right
        width: parent.width / 8
        iconColor: postStats.color
        svg: SvgOutline.moreVert
        onClicked: moreMenuLoader.open()

        Accessible.name: qsTr("more options")

        // PostStats is part of list item delegates.
        // Dynamicly loading the menu on demand improves list scrolling performance
        Loader {
            id: moreMenuLoader
            active: false

            function open() {
                active = true
            }

            onStatusChanged: {
                if (status == Loader.Ready)
                    item.open() // qmllint disable missing-property
            }

            sourceComponent: SkyMenu {
                id: moreMenu
                onAboutToShow: root.enablePopupShield(true)
                onAboutToHide: { root.enablePopupShield(false); parent.active = false }

                CloseMenuItem {
                    text: qsTr("<b>Post</b>")
                    Accessible.name: qsTr("close options menu")
                }
                AccessibleMenuItem {
                    text: qsTr("Translate")
                    onTriggered: translatePost()

                    MenuItemSvg { svg: SvgOutline.googleTranslate }
                }

                AccessibleMenuItem {
                    text: qsTr("Copy post text")
                    onTriggered: copyPostText()

                    MenuItemSvg { svg: SvgOutline.copy }
                }
                AccessibleMenuItem {
                    text: qsTr("Share")
                    onTriggered: share()

                    MenuItemSvg { svg: SvgOutline.share }
                }
                AccessibleMenuItem {
                    text: qsTr("Show more like this")
                    visible: feedAcceptsInteractions
                    onTriggered: showMoreLikeThis()

                    MenuItemSvg { svg: SvgOutline.sentimentSatisfied }
                }
                AccessibleMenuItem {
                    text: qsTr("Show less like this")
                    visible: feedAcceptsInteractions
                    onTriggered: showLessLikeThis()

                    MenuItemSvg { svg: SvgOutline.sentimentDissatisfied }
                }
                AccessibleMenuItem {
                    text: qsTr("View thread")
                    visible: showViewThread
                    onTriggered: viewThread()

                    MenuItemSvg { svg: SvgOutline.chat }
                }
                AccessibleMenuItem {
                    text: qsTr("Unroll thread")
                    visible: isThread && !isUnrolledThread
                    onTriggered: unrollThread()

                    MenuItemSvg { svg: SvgOutline.thread }
                }
                AccessibleMenuItem {
                    text: threadMuted ? qsTr("Unmute thread") : qsTr("Mute thread")
                    visible: !isReply || replyRootUri
                    onTriggered: muteThread()

                    MenuItemSvg { svg: threadMuted ? SvgOutline.notifications : SvgOutline.notificationsOff }
                }

                AccessibleMenuItem {
                    text: isHiddenReply ? qsTr("Unhide reply") : qsTr("Hide reply")
                    visible: isReply && !authorIsUser && isThreadFromUser()
                    onTriggered: hideReply()

                    MenuItemSvg { svg: isHiddenReply ? SvgOutline.visibility : SvgOutline.hideVisibility }
                }

                AccessibleMenuItem {
                    text: qsTr("Restrictions")
                    visible: authorIsUser
                    onTriggered: threadgate()

                    MenuItemSvg { svg: replyRestriction !== QEnums.REPLY_RESTRICTION_NONE ? SvgOutline.replyRestrictions : SvgOutline.noReplyRestrictions }
                }

                AccessibleMenuItem {
                    text: recordIsDetached() ? qsTr("Re-attach quote") : qsTr("Detach quote")
                    visible: hasOwnRecord()
                    onTriggered: detachQuote(getRecordPostUri(), !recordIsDetached())

                    MenuItemSvg { svg: recordIsDetached() ? SvgOutline.attach : SvgOutline.detach }
                }

                AccessibleMenuItem {
                    text: viewerStatePinned ? qsTr("Unpin from profile") : qsTr("Pin to profile")
                    visible: authorIsUser && !isUnrolledThread
                    onTriggered: {
                        if (viewerStatePinned)
                            unpin()
                        else
                            pin()
                    }

                    MenuItemSvg { svg: viewerStatePinned ? SvgFilled.unpin : SvgFilled.pin }
                }

                AccessibleMenuItem {
                    text: qsTr("Delete")
                    visible: authorIsUser && !isUnrolledThread
                    onTriggered: deletePost()

                    MenuItemSvg { svg: SvgOutline.delete }
                }
                AccessibleMenuItem {
                    text: qsTr("Report post")
                    visible: !authorIsUser && !isUnrolledThread
                    onTriggered: reportPost()

                    MenuItemSvg { svg: SvgOutline.report }
                }
                AccessibleMenuItem {
                    text: qsTr("Block author")
                    visible: !authorIsUser
                    onTriggered: blockAuthor()

                    MenuItemSvg { svg: SvgOutline.block }
                }
                AccessibleMenuItem {
                    text: qsTr("Emoji names")
                    onTriggered: showEmojiNames()

                    MenuItemSvg { svg: SvgOutline.emojiLanguage }
                }

                Component.onCompleted: background.color = guiSettings.menuColor
            }
        }
    }

    AccessibilityUtils {
        id: accessibilityUtils
    }


    function getRecordPostUri() {
        if (record)
            return record.detached ? record.detachedPostUri : record.postUri

        if (recordWithMedia)
            return recordWithMedia.record.detached ? recordWithMedia.record.detachedPostUri : recordWithMedia.record.postUri

        return ""
    }

    function recordIsDetached() {
        if (record)
            return record.detached

        if (recordWithMedia)
            return recordWithMedia.record.detached

        return false
    }

    function hasOwnRecord() {
        if (record)
            return record.detached ? guiSettings.isUserDid(record.detachedByDid) : guiSettings.isUserDid(record.author.did)

        if (recordWithMedia)
            return recordWithMedia.record.detached ? guiSettings.isUserDid(recordWithMedia.record.detachedByDid)  : guiSettings.isUserDid(recordWithMedia.record.author.did)

        return false
    }

    function isThreadFromUser() {
        if (!isReply)
            return authorIsUser

        return guiSettings.isUserDid(replyRootAuthorDid)
    }

    function statSpeech(stat, textSingular, textPlural) {
        return accessibilityUtils.statSpeech(stat, textSingular, textPlural)
    }
}
