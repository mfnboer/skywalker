import QtQuick
import QtQuick.Controls
import skywalker

Item {
    required property Skywalker skywalker
    required property int replyCount
    required property int repostCount
    required property int likeCount
    required property string repostUri
    required property bool repostTransient
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
    required property bool isQuotePost
    required property double indexedSecondsAgo
    property UserSettings userSettings: skywalker.getUserSettings()
    property int feedback: QEnums.FEEDBACK_NONE
    property int feedbackTransient: QEnums.FEEDBACK_NONE
    property bool isUnrolledThread: false
    property bool showViewThread: false
    property var record: null // recordview
    property var recordWithMedia: null // record_with_media_view
    property bool feedAcceptsInteractions: false
    property bool limitedStats: false
    property string color: guiSettings.statsColor
    property int topPadding: 0

    signal reply()
    signal replyLongPress(MouseEvent mouseEvent)
    signal repost(MouseEvent mouseEvent)
    signal repostLongPress(MouseEvent mouseEvent)
    signal like()
    signal likeLongPress(MouseEvent mouseEvent)
    signal viewThread()
    signal unrollThread()
    signal quoteChain()
    signal muteThread()
    signal bookmark()
    signal bookmarkLongPress(MouseEvent mouseEvent)
    signal share()
    signal threadgate()
    signal hideReply()
    signal editPost()
    signal deletePost()
    signal copyPostText()
    signal reportPost()
    signal translatePost()
    signal detachQuote(string uri, bool detach)
    signal pin()
    signal unpin()
    signal muteAuthor()
    signal blockAuthor()
    signal showEmojiNames()
    signal showMoreLikeThis()
    signal showLessLikeThis()
    signal viewAlsoLiked()

    id: postStats
    height: replyIcon.height + topPadding + (showMoreIconLoader.item ? showMoreIconLoader.item.height + topPadding : 0)

    StatIcon {
        id: replyIcon
        y: topPadding
        width: parent.width / 4
        iconColor: enabled ? postStats.color : guiSettings.statsDisabledColor
        svg: SvgOutline.reply
        statistic: replyCount
        visible: !limitedStats
        enabled: !replyDisabled
        onClicked: reply()
        onPressAndHold: (mouseEvent) => replyLongPress(mouseEvent)

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
        blinking: repostTransient
        visible: !limitedStats
        onClicked: (mouseEvent) => repost(mouseEvent)
        onPressAndHold: (mouseEvent) => repostLongPress(mouseEvent)

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
        blinking: likeTransient
        onClicked: like()
        onPressAndHold: (mouseEvent) => likeLongPress(mouseEvent)

        Accessible.name: qsTr("like") + statSpeech(likeCount, "like", "likes")
    }
    StatIcon {
        id: bookmarkIcon
        y: topPadding
        anchors.left: likeIcon.right
        width: parent.width / 8
        iconColor: isBookmarked ? guiSettings.buttonColor : postStats.color
        svg: isBookmarked ? SvgFilled.bookmark : SvgOutline.bookmark
        blinking: bookmarkTransient
        visible: !limitedStats
        onClicked: bookmark()
        onPressAndHold: (mouseEvent) => bookmarkLongPress(mouseEvent)

        Accessible.name: isBookmarked ? qsTr("remove bookmark") : qsTr("bookmark")
    }
    StatIcon {
        id: moreIcon
        y: topPadding
        anchors.right : parent.right
        width: parent.width / 8
        iconColor: postStats.color
        svg: SvgOutline.moreVert
        visible: !limitedStats
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
                onClosed: parent.active = false

                SkyMenuButton {
                    text: qsTr("Translate")
                    svg: SvgOutline.googleTranslate
                    popup: moreMenu
                    onClicked: translatePost()
                }
                SkyMenuButton {
                    text: qsTr("Copy post text")
                    svg: SvgOutline.copy
                    popup: moreMenu
                    onClicked: copyPostText()
                }
                SkyMenuButton {
                    text: qsTr("Share")
                    svg: SvgOutline.share
                    popup: moreMenu
                    onClicked: share()
                }
                SkyMenuButton {
                    color: feedback === QEnums.FEEDBACK_NONE ? guiSettings.textColor : guiSettings.disabledColor
                    text: qsTr("Show more like this")
                    svg: feedback === QEnums.FEEDBACK_MORE_LIKE_THIS ? SvgFilled.thumbUp : SvgOutline.thumbUp
                    popup: moreMenu
                    visible: feedAcceptsInteractions
                    onClicked: emitShowMoreLikeThis()
                }
                SkyMenuButton {
                    color: feedback === QEnums.FEEDBACK_NONE ? guiSettings.textColor : guiSettings.disabledColor
                    text: qsTr("Show less like this")
                    svg: feedback === QEnums.FEEDBACK_LESS_LIKE_THIS ? SvgFilled.thumbDown : SvgOutline.thumbDown
                    popup: moreMenu
                    visible: feedAcceptsInteractions
                    onClicked: emitShowLessLikeThis()
                }
                SkyMenuButton {
                    text: qsTr("Also liked")
                    svg: SvgOutline.like
                    popup: moreMenu
                    visible: likeCount > 0 && !isReply && indexedSecondsAgo < 30 * 24 * 3600
                    onClicked: viewAlsoLiked()
                }
                SkyMenuButton {
                    text: qsTr("View thread")
                    svg: SvgOutline.chat
                    popup: moreMenu
                    visible: showViewThread
                    onClicked: viewThread()
                }
                SkyMenuButton {
                    text: qsTr("Unroll thread")
                    svg: SvgOutline.thread
                    popup: moreMenu
                    visible: isThread && !isUnrolledThread
                    onClicked: unrollThread()
                }
                SkyMenuButton {
                    text: qsTr("Unwrap quote chain")
                    svg: SvgFilled.quote
                    popup: moreMenu
                    visible: isQuotePost
                    onClicked: quoteChain()
                }
                SkyMenuButton {
                    text: threadMuted ? qsTr("Unmute thread") : qsTr("Mute thread")
                    svg: threadMuted ? SvgOutline.notifications : SvgOutline.notificationsOff
                    popup: moreMenu
                    visible: !isReply || replyRootUri
                    onClicked: muteThread()
                }

                SkyMenuButton {
                    text: isHiddenReply ? qsTr("Unhide reply") : qsTr("Hide reply")
                    svg: isHiddenReply ? SvgOutline.visibility : SvgOutline.hideVisibility
                    popup: moreMenu
                    visible: isReply && !authorIsUser && isThreadFromUser()
                    onClicked: hideReply()
                }

                SkyMenuButton {
                    text: qsTr("Restrictions")
                    svg: replyRestriction !== QEnums.REPLY_RESTRICTION_NONE ? SvgOutline.replyRestrictions : SvgOutline.noReplyRestrictions
                    popup: moreMenu
                    visible: authorIsUser
                    onClicked: threadgate()
                }

                SkyMenuButton {
                    text: recordIsDetached() ? qsTr("Re-attach quote") : qsTr("Detach quote")
                    svg: recordIsDetached() ? SvgOutline.attach : SvgOutline.detach
                    popup: moreMenu
                    visible: hasOwnRecord()
                    onClicked: detachQuote(getRecordPostUri(), !recordIsDetached())
                }

                SkyMenuButton {
                    text: viewerStatePinned ? qsTr("Unpin from profile") : qsTr("Pin to profile")
                    svg: viewerStatePinned ? SvgFilled.unpin : SvgFilled.pin
                    popup: moreMenu
                    visible: authorIsUser && !isUnrolledThread
                    onClicked: {
                        if (viewerStatePinned)
                            unpin()
                        else
                            pin()
                    }
                }

                // Only allow the active user to edit posts to avoid too much complexity
                SkyMenuButton {
                    text: qsTr("Edit")
                    svg: SvgOutline.edit
                    popup: moreMenu
                    visible: authorIsUser && !isUnrolledThread && root.isActiveUser(skywalker.getUserDid())
                    onClicked: editPost()
                }
                SkyMenuButton {
                    text: qsTr("Delete")
                    svg: SvgOutline.delete
                    popup: moreMenu
                    visible: authorIsUser && !isUnrolledThread
                    onClicked: deletePost()
                }
                SkyMenuButton {
                    text: qsTr("Report post")
                    svg: SvgOutline.report
                    popup: moreMenu
                    visible: !authorIsUser && !isUnrolledThread
                    onClicked: reportPost()
                }
                SkyMenuButton {
                    text: qsTr("Mute author")
                    svg: SvgOutline.mute
                    popup: moreMenu
                    visible: !authorIsUser
                    onClicked: muteAuthor()
                }
                SkyMenuButton {
                    text: qsTr("Block author")
                    svg: SvgOutline.block
                    popup: moreMenu
                    visible: !authorIsUser
                    onClicked: blockAuthor()
                }
                SkyMenuButton {
                    text: qsTr("Emoji names")
                    svg: SvgOutline.emojiLanguage
                    popup: moreMenu
                    onClicked: showEmojiNames()
                }
            }
        }
    }

    Loader {
        id: showMoreIconLoader
        anchors.topMargin: topPadding
        anchors.top: likeIcon.bottom
        anchors.left: likeIcon.left
        width: parent.width / 4
        active: feedAcceptsInteractions && !limitedStats && userSettings.showFeedbackButtons

        sourceComponent: StatIcon {
            id: showMoreIcon
            iconColor: postStats.color
            svg: feedback === QEnums.FEEDBACK_MORE_LIKE_THIS ? SvgFilled.thumbUp : SvgOutline.thumbUp
            blinking: feedbackTransient === QEnums.FEEDBACK_MORE_LIKE_THIS
            Accessible.name: qsTr("Show more like this")
            onClicked: emitShowMoreLikeThis()
            onPressAndHold: showFeedbackNotice(true)
        }
    }

    Loader {
        id: showLessIconLoader
        anchors.topMargin: topPadding
        anchors.top: moreIcon.bottom
        anchors.left: moreIcon.left
        width: parent.width / 8
        active: feedAcceptsInteractions && !limitedStats && userSettings.showFeedbackButtons

        sourceComponent: StatIcon {
            id: showLessIcon
            iconColor: postStats.color
            svg: feedback === QEnums.FEEDBACK_LESS_LIKE_THIS ? SvgFilled.thumbDown : SvgOutline.thumbDown
            blinking: feedbackTransient === QEnums.FEEDBACK_LESS_LIKE_THIS
            Accessible.name: qsTr("Show more less this")
            onClicked: emitShowLessLikeThis()
            onPressAndHold: showFeedbackNotice(true)
        }
    }

    AccessibilityUtils {
        id: accessibilityUtils
    }

    function emitShowMoreLikeThis() {
        if (showFeedbackNotice())
            return

        if (feedback === QEnums.FEEDBACK_NONE)
            showMoreLikeThis()
        else
            skywalker.showStatusMessage(qsTr("Feedback sent"), QEnums.STATUS_LEVEL_INFO, 1)
    }

    function emitShowLessLikeThis() {
        if (showFeedbackNotice())
            return

        if (feedback === QEnums.FEEDBACK_NONE)
            showLessLikeThis()
        else
            skywalker.showStatusMessage(qsTr("Feedback sent"), QEnums.STATUS_LEVEL_INFO, 1)
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
        const userDid = skywalker.getUserDid()

        if (record)
            return record.detached ? record.detachedByDid === userDid : record.author.did === userDid

        if (recordWithMedia)
            return recordWithMedia.record.detached ? recordWithMedia.record.detachedByDid === userDid  : recordWithMedia.record.author.did === userDid

        return false
    }

    function isThreadFromUser() {
        if (!isReply)
            return authorIsUser

        return replyRootAuthorDid === skywalker.getUserDid()
    }

    function statSpeech(stat, textSingular, textPlural) {
        return accessibilityUtils.statSpeech(stat, textSingular, textPlural)
    }

    function showFeedbackNotice(unconditional = false) {
        if (!unconditional) {
            if (!userSettings.getShowFeedbackNotice())
                return false

            userSettings.setShowFeedbackNotice(false)
        }

        let msg = qsTr("With the feedback buttons (thumb up/down) you give feedback to the creator of the feed.<br><br>" +
                       "👍 = you like more posts like this<br><br>" +
                       "👎 = you prefer less posts like this<br><br>" +
                       "Not all feeds support feedback. These buttons are only shown for feeds that support it.<br><br>" +
                       "You can also give feedback via the menu button (⋮). If you don't want the feedback buttons, you can disable them in the appearance " +
                       `<a href="settings" style="color: ${guiSettings.linkColor}; text-decoration: none">Settings</a>.`)

        if (!unconditional)
            msg += qsTr("<br><br>This message will not be shown again.")

        guiSettings.notice(root, msg, "", () => {}, (link) => { root.editSettings() })
        return true
    }
}
