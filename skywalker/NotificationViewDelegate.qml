import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Rectangle {
    property int margin: 10
    required property int index
    required property basicprofile notificationAuthor
    required property list<basicprofile> notificationOtherAuthors
    required property list<basicprofile> notificationAllAuthors
    required property int notificationReason // QEnums::NotificationReason
    required property string notificationReasonSubjectUri
    required property string notificationReasonSubjectCid
    required property string notificationReasonPostText
    required property string notificationReasonPostPlainText
    required property bool notificationReasonPostIsReply
    required property basicprofile notificationReasonPostReplyToAuthor
    required property list<imageview> notificationReasonPostImages
    required property var notificationReasonPostVideo
    required property var notificationReasonPostExternal // externalview (var allows NULL)
    required property var notificationReasonPostRecord // recordview
    required property var notificationReasonPostRecordWithMedia // record_with_media_view
    required property date notificationReasonPostTimestamp
    required property bool notificationReasonPostNotFound
    required property list<language> notificationReasonPostLanguages
    required property list<contentlabel> notificationReasonPostLabels
    required property bool notificationReasonPostLocallyDeleted
    required property date notificationTimestamp
    required property bool notificationIsRead
    required property string notificationPostUri
    required property string notificationCid
    required property basicprofile notificationPostAuthor
    required property string notificationPostText
    required property string notificationPostPlainText
    required property date notificationPostTimestamp
    required property list<imageview> notificationPostImages
    required property var notificationPostVideo
    required property var notificationPostExternal // externalview (var allows NULL)
    required property var notificationPostRecord // recordview
    required property var notificationPostRecordWithMedia // record_with_media_view
    required property string notificationPostReplyRootAuthorDid
    required property string notificationPostReplyRootUri
    required property string notificationPostReplyRootCid
    required property string notificationPostRepostUri
    required property string notificationPostLikeUri
    required property bool notificationPostLikeTransient
    required property bool notificationPostThreadMuted
    required property bool notificationPostReplyDisabled
    required property bool notificationPostEmbeddingDisabled
    required property bool notificationPostViewerStatePinned
    required property string notificationPostThreadgateUri
    required property int  notificationPostReplyRestriction // QEnums::ReplyRestriction flags
    required property list<listviewbasic> notificationPostReplyRestrictionLists
    required property list<string> notificationPostHiddenReplies
    required property bool notificationPostIsHiddenReply
    required property int notificationPostRepostCount
    required property int notificationPostLikeCount
    required property int notificationPostQuoteCount
    required property int notificationPostReplyCount
    required property bool notificationPostBookmarked
    required property bool notificationPostNotFound
    required property list<language> notificationPostLanguages
    required property list<contentlabel> notificationPostLabels
    required property int notificationPostContentVisibility // QEnums::PostContentVisibility
    required property string notificationPostContentWarning
    required property int notificationPostMutedReason // QEnums::MutedPostReason
    required property bool notificationPostIsReply
    required property basicprofile replyToAuthor
    required property string notificationInviteCode
    required property basicprofile notificationInviteCodeUsedBy
    required property bool endOfList
    property bool onScreen: false

    id: notification
    height: grid.height
    color: notificationIsRead ? guiSettings.backgroundColor : guiSettings.postHighLightColor

    Accessible.role: Accessible.Button
    Accessible.name: getSpeech()
    Accessible.onPressAction: openNotification()

    onYChanged: checkOnScreen()

    onOnScreenChanged: {
        if (!onScreen)
            cover()
    }

    function cover() {
        postLoader.movedOffScreen()
        aggregatableLoader.movedOffScreen()
    }

    GridLayout {
        id: grid
        columns: 2
        width: parent.width
        rowSpacing: 5

        // Author and content
        Rectangle {
            id: avatar
            width: guiSettings.threadColumnWidth
            Layout.fillHeight: true
            color: "transparent"

            MouseArea {
                anchors.fill: parent
                onClicked: showAuthorList()
                enabled: isAggregatableReason()
            }

            Avatar {
                id: avatarImg
                x: 8
                y: 10
                width: parent.width - 13
                height: width
                author: notificationAuthor
                visible: showPost()

                onClicked: skywalker.getDetailedProfile(notificationAuthor.did)

                Accessible.role: Accessible.Button
                Accessible.name: qsTr(`show profile of ${notificationAuthor.name}`)
                Accessible.onPressAction: clicked()
            }
            SkySvg {
                x: parent.x + 14
                y: height + 5
                width: parent.width - 19
                height: width
                color: guiSettings.likeColor
                svg: svgFilled.like
                visible: notificationReason === QEnums.NOTIFICATION_REASON_LIKE
            }
            SkySvg {
                x: parent.x + 14
                y: height + 5
                width: parent.width - 19
                height: width
                color: guiSettings.textColor
                svg: svgOutline.repost
                visible: notificationReason === QEnums.NOTIFICATION_REASON_REPOST
            }
            SkySvg {
                x: parent.x + 14
                y: height + 5
                width: parent.width - 19
                height: width
                color: guiSettings.textColor
                svg: svgOutline.inviteCode
                visible: notificationReason === QEnums.NOTIFICATION_REASON_INVITE_CODE_USED
            }
            SkySvg {
                x: parent.x + 14
                y: height + 5
                width: parent.width - 19
                height: width
                color: guiSettings.moderatorIconColor
                outlineColor: guiSettings.textColor
                svg: svgFilled.moderator
                visible: notificationReason === QEnums.NOTIFICATION_REASON_NEW_LABELS
            }
            Rectangle {
                x: parent.x + 14
                y: parent.y + 5
                width: parent.width - 19
                height: width
                radius: height / 2
                color: guiSettings.avatarDefaultColor
                visible: notificationReason === QEnums.NOTIFICATION_REASON_FOLLOW

                SkySvg {
                    x: 5
                    y: height + 5
                    width: parent.width - 10
                    height: width
                    color: "white"
                    svg: svgFilled.newFollower
                }
            }
        }

        Loader {
            id: postLoader
            width: parent.width - guiSettings.threadColumnWidth - notification.margin * 2
            active: showPost()
            visible: status == Loader.Ready

            function movedOffScreen() {
                if (item)
                    item.movedOffScreen()
            }

            sourceComponent: Column {
                id: postColumn
                width: parent.width
                topPadding: 5

                function movedOffScreen() {
                    postBody.movedOffScreen()
                }

                PostHeader {
                    id: postHeader
                    width: parent.width
                    Layout.fillWidth: true
                    author: notificationPostAuthor
                    postThreadType: QEnums.THREAD_NONE
                    postIndexedSecondsAgo: (new Date() - notificationTimestamp) / 1000
                }

                // Reply to
                ReplyToRow {
                    width: parent.width
                    text: qsTr(`Reply to ${replyToAuthor.name}`)
                    visible: notificationPostIsReply
                }

                // Reply hidden by user
                ReplyToRow {
                    width: parent.width
                    text: qsTr("Reply hidden by you")
                    visible: notificationPostIsHiddenReply && notificationPostReplyRootAuthorDid === skywalker.getUserDid()
                }

                PostBody {
                    id: postBody
                    width: parent.width
                    Layout.fillWidth: true
                    postAuthor: notificationAuthor
                    postText: notificationPostText
                    postPlainText: notificationPostPlainText
                    postImages: notificationPostImages
                    postLanguageLabels: notificationPostLanguages
                    postContentLabels: notificationPostLabels
                    postContentVisibility: notificationPostContentVisibility
                    postContentWarning: notificationPostContentWarning
                    postMuted: notificationPostMutedReason
                    postVideo: notificationPostVideo
                    postExternal: notificationPostExternal
                    postRecord: notificationPostRecord
                    postRecordWithMedia: notificationPostRecordWithMedia
                    postDateTime: notificationPostTimestamp
                    bodyBackgroundColor: notification.color
                }

                PostStats {
                    width: parent.width
                    topPadding: 10
                    replyCount: notificationPostReplyCount
                    repostCount: notificationPostRepostCount + notificationPostQuoteCount
                    likeCount: notificationPostLikeCount
                    repostUri: notificationPostRepostUri
                    likeUri: notificationPostLikeUri
                    likeTransient: notificationPostLikeTransient
                    threadMuted: notificationPostThreadMuted
                    replyDisabled: notificationPostReplyDisabled
                    embeddingDisabled: notificationPostEmbeddingDisabled
                    viewerStatePinned: notificationPostViewerStatePinned
                    replyRestriction: notificationPostReplyRestriction
                    isHiddenReply: notificationPostIsHiddenReply
                    isReply: notificationPostIsReply
                    replyRootAuthorDid: notificationPostReplyRootAuthorDid
                    replyRootUri: notificationPostReplyRootUri
                    visible: !notificationPostNotFound
                    authorIsUser: false
                    isBookmarked: notificationPostBookmarked
                    bookmarkNotFound: false
                    record: notificationPostRecord
                    recordWithMedia: notificationPostRecordWithMedia

                    onReply: {
                        const lang = notificationPostLanguages.length > 0 ?
                                       notificationPostLanguages[0].shortCode : ""

                        root.composeReply(notificationPostUri, notificationCid, notificationPostText,
                                          notificationPostTimestamp, notificationAuthor,
                                          notificationPostReplyRootUri, notificationPostReplyRootCid,
                                          lang)
                    }

                    onRepost: {
                        root.repost(notificationPostRepostUri, notificationPostUri, notificationCid,
                                    notificationPostText, notificationPostTimestamp,
                                    notificationAuthor, notificationPostEmbeddingDisabled)
                    }

                    onLike: root.like(notificationPostLikeUri, notificationPostUri, notificationCid)

                    onBookmark: {
                        if (isBookmarked) {
                            skywalker.bookmarks.removeBookmark(notificationPostUri)
                        }
                        else {
                            const bookmarked = skywalker.bookmarks.addBookmark(notificationPostUri)

                            if (!bookmarked)
                                skywalker.showStatusMessage(qsTr("Your bookmarks are full!"), QEnums.STATUS_LEVEL_ERROR)
                        }
                    }

                    onShare: skywalker.sharePost(notificationPostUri)
                    onMuteThread: root.muteThread(notificationPostIsReply ? notificationPostReplyRootUri : notificationPostUri, notificationPostThreadMuted)
                    onThreadgate: root.gateRestrictions(notificationPostThreadgateUri, notificationPostIsReply ? notificationPostReplyRootUri : notificationPostUri, notificationPostIsReply ? notificationPostReplyRootCid : notificationCid, notificationPostUri, notificationPostReplyRestriction, notificationPostReplyRestrictionLists, notificationPostHiddenReplies)
                    onHideReply: root.hidePostReply(notificationPostThreadgateUri, notificationPostReplyRootUri, notificationPostReplyRootCid, notificationPostUri, notificationPostReplyRestriction, notificationPostReplyRestrictionLists, notificationPostHiddenReplies)
                    onCopyPostText: skywalker.copyPostTextToClipboard(notificationPostPlainText)
                    onReportPost: root.reportPost(notificationPostUri, notificationCid, notificationPostText, notificationPostTimestamp, notificationAuthor)
                    onTranslatePost: root.translateText(notificationPostPlainText)
                    onDetachQuote: (uri, detach) => root.detachQuote(uri, notificationPostUri, notificationCid, detach)
                    onPin: root.pinPost(notificationPostUri, notificationCid)
                    onUnpin: root.unpinPost(notificationCid)
                }
            }
        }

        Loader {
            id: aggregatableLoader
            width: parent.width - guiSettings.threadColumnWidth - notification.margin * 2
            active: isAggregatableReason()
            visible: status == Loader.Ready

            function movedOffScreen() {
                if (item)
                    item.movedOffScreen()
            }

            sourceComponent: Column {
                width: parent.width
                topPadding: 5

                function movedOffScreen() {
                    reasonPostBody.movedOffScreen()
                }

                Row {
                    width: parent.width
                    spacing: 5

                    Avatar {
                        id: authorAvatar
                        width: 34
                        height: width
                        author: notificationAuthor

                        onClicked: skywalker.getDetailedProfile(notificationAuthor.did)

                        Accessible.role: Accessible.Button
                        Accessible.name: qsTr(`show profile of ${notificationAuthor.name}`)
                        Accessible.onPressAction: clicked()
                    }
                    Repeater {
                        model: Math.min(notificationOtherAuthors.length, 4)

                        Avatar {
                            required property int index

                            width: authorAvatar.width
                            height: width
                            author: notificationOtherAuthors[index]

                            onClicked: skywalker.getDetailedProfile(notificationOtherAuthors[index].did)

                            Accessible.role: Accessible.Button
                            Accessible.name: qsTr(`show profile of ${notificationOtherAuthors[index].name}`)
                            Accessible.onPressAction: clicked()
                        }
                    }
                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        color: guiSettings.textColor
                        text: `+${(notificationOtherAuthors.length - 4)}`
                        visible: notificationOtherAuthors.length > 4

                        MouseArea {
                            anchors.fill: parent
                            onClicked: showAuthorList()
                        }
                    }
                }

                RowLayout {
                    width: parent.width

                    Text {
                        Layout.fillWidth: true
                        textFormat: Text.RichText
                        wrapMode: Text.Wrap
                        color: guiSettings.textColor
                        text: authorsAndReasonText()
                    }
                    Text {
                        Layout.fillHeight: true
                        text: guiSettings.durationToString((new Date() - notificationTimestamp) / 1000)
                        font.pointSize: guiSettings.scaledFont(7/8)
                        color: Material.color(Material.Grey)
                    }
                }

                // Reply to
                ReplyToRow {
                    width: parent.width
                    text: qsTr(`Reply to ${notificationReasonPostReplyToAuthor.name}`)
                    visible: showPostForAggregatableReason() && notificationReasonPostIsReply
                }

                PostBody {
                    id: reasonPostBody
                    topPadding: 5
                    width: parent.width
                    postAuthor: skywalker.getUser()
                    postText: {
                        if (notificationReasonPostLocallyDeleted)
                            return "DELETED"
                        else if (notificationReasonPostNotFound)
                            return "NOT FOUND"

                        return notificationReasonPostText
                    }
                    postPlainText: !notificationReasonPostLocallyDeleted && !notificationReasonPostNotFound ?
                                       notificationReasonPostPlainText : ""
                    postImages: notificationReasonPostImages
                    postLanguageLabels: notificationReasonPostLanguages
                    postContentLabels: notificationReasonPostLabels
                    postContentVisibility: QEnums.CONTENT_VISIBILITY_SHOW // User's own post
                    postContentWarning: ""
                    postMuted: QEnums.MUTED_POST_NONE
                    postDateTime: notificationReasonPostTimestamp
                    postVideo: notificationReasonPostVideo
                    postExternal: notificationReasonPostExternal
                    postRecord: notificationReasonPostRecord
                    postRecordWithMedia: notificationReasonPostRecordWithMedia
                    bodyBackgroundColor: notification.color
                    visible: showPostForAggregatableReason()
                }
            }
        }

        Loader {
            id: inviteCodeLoader
            width: parent.width - guiSettings.threadColumnWidth - notification.margin * 2
            active: notificationReason === QEnums.NOTIFICATION_REASON_INVITE_CODE_USED
            visible: status == Loader.Ready
            sourceComponent: Column {
                width: parent.width
                topPadding: 5

                RowLayout {
                    width: parent.width
                    Avatar {
                        id: usedByAvatar
                        width: 34
                        height: width
                        author: notificationInviteCodeUsedBy

                        onClicked: skywalker.getDetailedProfile(notificationInviteCodeUsedBy.did)

                        Accessible.role: Accessible.Button
                        Accessible.name: qsTr(`show profile of ${notificationInviteCodeUsedBy.name}`)
                        Accessible.onPressAction: clicked()
                    }

                    SkyButton {
                        Layout.alignment: Qt.AlignRight
                        text: qsTr("Dismiss")
                        onClicked: {
                            console.debug("DISMISS:", index)
                            skywalker.notificationListModel.dismissInviteCodeUsageNotification(index)
                        }
                    }
                }

                Text {
                    width: parent.width
                    textFormat: Text.RichText
                    wrapMode: Text.Wrap
                    color: guiSettings.textColor
                    text: {
                        `<b>${(unicodeFonts.toCleanedHtml(notificationInviteCodeUsedBy.name))}</b> ` +
                        qsTr("used your invite code") + ": " + notificationInviteCode
                    }
                }
            }
        }

        // Separator
        Rectangle {
            width: parent.width
            Layout.preferredHeight: 1
            Layout.columnSpan: 2
            color: guiSettings.separatorColor
        }

        // End of feed indication
        Text {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            topPadding: 10
            elide: Text.ElideRight
            color: guiSettings.textColor
            text: qsTr("End of feed")
            font.italic: true
            visible: endOfList
        }
    }

    MouseArea {
        z: -2 // Let other mouse areas, e.g. images, get on top, -2 to allow records on top
        anchors.fill: parent
        onClicked: openNotification()
    }

    AccessibilityUtils {
        id: accessibilityUtils
    }

    UnicodeFonts {
        id: unicodeFonts
    }

    GuiSettings {
        id: guiSettings
    }

    function openNotification() {
        if (notificationPostUri)
            skywalker.getPostThread(notificationPostUri)
        else if (notificationInviteCode)
            skywalker.getDetailedProfile(notificationInviteCodeUsedBy.did)
        else if (isAggregatableReason)
            showAuthorList()
    }

    function showAuthorList() {
        let title = "Users:"

        switch (notificationReason) {
        case QEnums.NOTIFICATION_REASON_LIKE:
            title = qsTr("Liked by")
            break
        case QEnums.NOTIFICATION_REASON_FOLLOW:
            title = qsTr("New followers")
            break
        case QEnums.NOTIFICATION_REASON_REPOST:
            title = qsTr("Reposted by")
            break
        case QEnums.NOTIFICATION_REASON_NEW_LABELS:
            title = qsTr("Labelers")
            break
        }

        root.viewSimpleAuthorList(title, notificationAllAuthors)
    }

    function showPost() {
        let reasons = [QEnums.NOTIFICATION_REASON_MENTION,
                       QEnums.NOTIFICATION_REASON_REPLY,
                       QEnums.NOTIFICATION_REASON_QUOTE]
        return reasons.includes(notificationReason)
    }

    function isAggregatableReason() {
        let reasons = [QEnums.NOTIFICATION_REASON_LIKE,
                       QEnums.NOTIFICATION_REASON_FOLLOW,
                       QEnums.NOTIFICATION_REASON_REPOST,
                       QEnums.NOTIFICATION_REASON_NEW_LABELS]
        return reasons.includes(notificationReason)
    }

    function showPostForAggregatableReason() {
        let reasons = [QEnums.NOTIFICATION_REASON_LIKE,
                       QEnums.NOTIFICATION_REASON_REPOST]
        return reasons.includes(notificationReason)
    }

    function reasonText() {
        switch (notificationReason) {
        case QEnums.NOTIFICATION_REASON_LIKE:
            return qsTr("liked your post")
        case QEnums.NOTIFICATION_REASON_FOLLOW:
            return qsTr("started following you")
        case QEnums.NOTIFICATION_REASON_REPOST:
            return qsTr("reposted your post")
        case QEnums.NOTIFICATION_REASON_MENTION:
            return qsTr("mentioned you")
        case QEnums.NOTIFICATION_REASON_REPLY:
            return qsTr("replied to you")
        case QEnums.NOTIFICATION_REASON_QUOTE:
            return qsTr("quoted you")
        case QEnums.NOTIFICATION_REASON_NEW_LABELS:
            return qsTr("published new labels. Visit the labeler profile to see which labels are new.")
        default:
            return "UNKNOW REASON: " + notificationReason
        }
    }

    function authorsAndReasonText() {
        return `<b>${(unicodeFonts.toCleanedHtml(notificationAuthor.name))}</b> ` +
            (notificationOtherAuthors.length > 0 ?
                (notificationOtherAuthors.length > 1 ?
                    qsTr(`and ${(notificationOtherAuthors.length)} others `) :
                    qsTr(`and <b>${(unicodeFonts.toCleanedHtml(notificationOtherAuthors[0].name))}</b> `)) :
                "") +
            reasonText()
    }

    function getReasonPostSpeech() {
        if (notificationReasonPostLocallyDeleted)
            return qsTr("deleted post")

        if (notificationReasonPostNotFound)
            return qsTr("not found")

        return accessibilityUtils.getPostSpeech(notificationReasonPostTimestamp,
                skywalker.getUser(), notificationReasonPostPlainText, notificationReasonPostImages,
                notificationReasonPostExternal, notificationReasonPostRecord,
                notificationReasonPostRecordWithMedia,
                accessibilityUtils.nullAuthor,
                notificationReasonPostIsReply, notificationReasonPostReplyToAuthor)
    }

    function getAggregatableSpeech() {
        const reason = unicodeFonts.toPlainText(authorsAndReasonText())
        const time = accessibilityUtils.getTimeSpeech(notificationTimestamp)
        let speech = `${time} ${reason}`

        if (showPostForAggregatableReason()) {
            const postSpeech = getReasonPostSpeech()
            speech += `\n\n${postSpeech}`
        }

        return speech
    }

    function getPostSpeech() {
        const reason = reasonText()
        const time = accessibilityUtils.getTimeSpeech(notificationTimestamp)
        const postSpeech = accessibilityUtils.getPostSpeech(notificationPostTimestamp,
                notificationAuthor, notificationPostPlainText, notificationPostImages,
                notificationPostExternal, notificationPostRecord,
                notificationPostRecordWithMedia, accessibilityUtils.nullAuthor,
                notificationPostIsReply, replyToAuthor)

        let speech = `${time} ${notificationAuthor.name} ${reason}\n\n${postSpeech}`
        return speech
    }

    function getSpeech() {
        if (isAggregatableReason())
            return getAggregatableSpeech()

        if (showPost())
            return getPostSpeech()
    }

    function checkOnScreen() {
        const headerHeight = ListView.view.headerItem ? ListView.view.headerItem.height : 0
        const topY = ListView.view.contentY + headerHeight
        onScreen = (y + height > topY) && (y < ListView.view.contentY + ListView.view.height)
    }

    Component.onCompleted: {
        ListView.view.enableOnScreenCheck = true
        checkOnScreen()
    }
}
