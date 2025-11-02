import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker

Rectangle {
    required property basicprofile owner
    property Skywalker skywalker: root.getSkywalker(owner.did)
    property int margin: 10
    required property int index
    required property basicprofile notificationAuthor
    required property list<basicprofile> notificationOtherAuthors
    required property list<basicprofile> notificationAllAuthors
    required property int notificationReason // QEnums::NotificationReason
    required property string notificationReasonRaw
    required property bool notificationIsAggregatable
    required property string notificationReasonSubjectUri
    required property string notificationReasonSubjectCid
    required property string notificationReasonPostText
    required property string notificationReasonPostPlainText
    required property basicprofile notificationReasonPostAuthor
    required property bool notificationReasonPostIsReply
    required property basicprofile notificationReasonPostReplyToAuthor
    required property bool notificationReasonPostHasUnknownEmbed
    required property string notificationReasonPostUnknownEmbedType
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
    required property double notificationSecondsAgo
    required property bool notificationIsRead
    required property string notificationPostUri
    required property string notificationCid
    required property basicprofile notificationPostAuthor
    required property string notificationPostText
    required property string notificationPostPlainText
    required property date notificationPostTimestamp
    required property bool notificationPostHasUnknownEmbed
    required property string notificationPostUnknownEmbedType
    required property list<imageview> notificationPostImages
    required property var notificationPostVideo
    required property var notificationPostExternal // externalview (var allows NULL)
    required property var notificationPostRecord // recordview
    required property var notificationPostRecordWithMedia // record_with_media_view
    required property string notificationPostReplyRootAuthorDid
    required property string notificationPostReplyRootUri
    required property string notificationPostReplyRootCid
    required property list<string> notificationPostMentionDids
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
    required property bool notificationPostBookmarkTransient
    required property bool notificationPostNotFound
    required property bool notificationPostBlocked
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
            Layout.preferredWidth: guiSettings.threadColumnWidth
            Layout.fillHeight: true
            color: "transparent"

            MouseArea {
                anchors.fill: parent
                onClicked: showAuthorList()
                enabled: notificationIsAggregatable
            }

            Avatar {
                id: avatarImg
                x: 8
                y: 10
                width: parent.width - 13
                userDid: owner.did
                author: notificationAuthor
                visible: showAvatarAsIcon()

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
                svg: SvgFilled.like
                visible: [QEnums.NOTIFICATION_REASON_LIKE, QEnums.NOTIFICATION_REASON_LIKE_VIA_REPOST].includes(notificationReason)
            }
            SkySvg {
                x: parent.x + 14
                y: height + 5
                width: parent.width - 19
                height: width
                color: guiSettings.textColor
                svg: SvgOutline.repost
                visible: [QEnums.NOTIFICATION_REASON_REPOST, QEnums.NOTIFICATION_REASON_REPOST_VIA_REPOST].includes(notificationReason)
            }
            SkySvg {
                x: parent.x + 14
                y: height + 5
                width: parent.width - 19
                height: width
                color: guiSettings.avatarDefaultColor
                svg: SvgFilled.notificationsActive
                visible: [QEnums.NOTIFICATION_REASON_SUBSCRIBED_POST].includes(notificationReason)
            }
            SkySvg {
                x: parent.x + 14
                y: height + 5
                width: parent.width - 19
                height: width
                color: guiSettings.textColor
                svg: SvgOutline.inviteCode
                visible: notificationReason === QEnums.NOTIFICATION_REASON_INVITE_CODE_USED
            }
            SkySvg {
                x: parent.x + 14
                y: height + 5
                width: parent.width - 19
                height: width
                color: guiSettings.moderatorIconColor
                outlineColor: guiSettings.textColor
                svg: SvgFilled.moderator
                visible: notificationReason === QEnums.NOTIFICATION_REASON_NEW_LABELS
            }
            SkySvg {
                x: parent.x + 14
                y: height + 5
                width: parent.width - 19
                height: width
                color: guiSettings.starterpackColor
                svg: SvgOutline.starterpack
                visible: notificationReason === QEnums.NOTIFICATION_REASON_STARTERPACK_JOINED
            }
            Image {
                x: parent.x + 14
                y: 5
                width: parent.width - 19
                height: width
                fillMode: Image.PreserveAspectFit
                source: "/images/verified_check.svg"
                asynchronous: true
                visible: [QEnums.NOTIFICATION_REASON_VERIFIED, QEnums.NOTIFICATION_REASON_UNVERIFIED].includes(notificationReason)
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
                    svg: SvgFilled.newFollower
                }
            }
        }

        Loader {
            id: postLoader
            Layout.preferredWidth: notification.width - guiSettings.threadColumnWidth - notification.margin * 2
            active: showPost()
            visible: status == Loader.Ready

            function movedOffScreen() {
                if (item)
                    item.movedOffScreen()
            }

            sourceComponent: Column {
                id: postColumn
                width: postLoader.width
                topPadding: 5

                function movedOffScreen() {
                    postBody.movedOffScreen()
                }

                PostHeader {
                    id: postHeader
                    width: parent.width
                    userDid: owner.did
                    author: notificationPostAuthor
                    postIndexedSecondsAgo: notificationSecondsAgo
                    visible: showAvatarAsIcon()
                }
                PostHeaderWithAvatar {
                    width: parent.width
                    userDid: owner.did
                    author: notificationPostAuthor
                    postIndexedSecondsAgo: notificationSecondsAgo
                    visible: !showAvatarAsIcon()
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
                    userDid: owner.did
                    postAuthor: notificationAuthor
                    postText: notificationPostBlocked ? qsTr("🚫 Blocked") : notificationPostText
                    postPlainText: notificationPostBlocked ? "" : notificationPostPlainText
                    postHasUnknownEmbed: notificationPostHasUnknownEmbed
                    postUnknownEmbedType: notificationPostUnknownEmbedType
                    postImages: notificationPostImages
                    postLanguageLabels: notificationPostLanguages
                    postContentLabels: notificationPostLabels
                    postContentVisibility: notificationPostContentVisibility
                    postContentWarning: notificationPostContentWarning
                    postMuted: notificationPostMutedReason
                    postIsThread: false
                    postIsThreadReply: false
                    postVideo: notificationPostVideo
                    postExternal: notificationPostExternal
                    postRecord: notificationPostRecord
                    postRecordWithMedia: notificationPostRecordWithMedia
                    postDateTime: notificationPostTimestamp
                    bodyBackgroundColor: notification.color
                    borderColor: notificationIsRead ? guiSettings.borderColor : guiSettings.borderHighLightColor
                }

                Loader {
                    active: true
                    width: parent.width
                    height: guiSettings.statsHeight + 10
                    asynchronous: true

                    sourceComponent: PostStats {
                        id: postStats
                        width: parent.width
                        topPadding: 10
                        skywalker: notification.skywalker
                        replyCount: notificationPostReplyCount
                        repostCount: notificationPostRepostCount + notificationPostQuoteCount
                        likeCount: notificationPostLikeCount
                        repostUri: notificationPostRepostUri
                        likeUri: notificationPostLikeUri
                        likeTransient: notificationPostLikeTransient
                        threadMuted: notificationPostThreadMuted
                        replyDisabled: notificationPostReplyDisabled
                        viewerStatePinned: notificationPostViewerStatePinned
                        replyRestriction: notificationPostReplyRestriction
                        isHiddenReply: notificationPostIsHiddenReply
                        isReply: notificationPostIsReply
                        replyRootAuthorDid: notificationPostReplyRootAuthorDid
                        replyRootUri: notificationPostReplyRootUri
                        visible: !notificationPostNotFound && !notificationPostBlocked
                        authorIsUser: false
                        isBookmarked: notificationPostBookmarked
                        bookmarkTransient: notificationPostBookmarkTransient
                        isThread: false
                        record: notificationPostRecord
                        recordWithMedia: notificationPostRecordWithMedia

                        function replyToNotification(postByDid = "") {
                            const lang = notificationPostLanguages.length > 0 ?
                                           notificationPostLanguages[0].shortCode : ""

                            root.composeReply(notificationPostUri, notificationCid, notificationPostText,
                                              notificationPostTimestamp, notificationAuthor,
                                              notificationPostReplyRootUri, notificationPostReplyRootCid,
                                              lang, notificationPostMentionDids, "", "",
                                              postByDid)
                        }

                        onReply: replyToNotification(notification.owner.did)

                        onReplyLongPress: (mouseEvent) => {
                            if (!root.isActiveUser(owner.did))
                                return

                            const lang = notificationPostLanguages.length > 0 ?
                                            notificationPostLanguages[0].shortCode : ""

                            root.replyByNonActiveUser(
                                    mouseEvent, postStats, notification.ListView.view,
                                    notificationPostUri, notificationCid, notificationPostText,
                                    notificationPostTimestamp, notificationAuthor,
                                    notificationPostReplyRootUri, notificationPostReplyRootCid,
                                    lang, notificationPostMentionDids)
                        }

                        function repostNotification(nonActiveUserDid = "") {
                            root.repost(notificationPostRepostUri, notificationPostUri, notificationCid,
                                        "", "", notificationPostText, notificationPostTimestamp,
                                        notificationAuthor, notificationPostEmbeddingDisabled, notificationPostPlainText,
                                        nonActiveUserDid)
                        }

                        onRepost: repostNotification(notification.owner.did)

                        function quoteNotification(nonActiveUserDid = "") {
                            root.quotePost(notificationPostUri, notificationCid,
                                    notificationPostText, notificationPostTimestamp,
                                    notificationAuthor, notificationPostEmbeddingDisabled,
                                    nonActiveUserDid)
                        }

                        onRepostLongPress: (mouseEvent) => {
                            if (!root.isActiveUser(owner.did)) {
                                quoteNotification(notification.owner.did)
                                return
                            }

                            const actionDone = root.repostByNonActiveUser(
                                    mouseEvent, postStats, notification.ListView.view,
                                    notificationPostUri, notificationCid,
                                    notificationPostText, notificationPostTimestamp,
                                    notificationAuthor, notificationPostEmbeddingDisabled)

                            if (!actionDone)
                                quoteNotification()
                        }

                        onLike: {
                            root.like(notificationPostLikeUri, notificationPostUri, notificationCid,
                            "", "", "", "", notification.owner.did)
                        }

                        onLikeLongPress: (mouseEvent) => {
                            if (!root.isActiveUser(owner.did))
                                return

                            root.likeByNonActiveUser(mouseEvent, postStats, notification.ListView.view,
                                                     notificationPostUri)
                        }

                        onBookmark: {
                            if (isBookmarked)
                                skywalker.getBookmarks().removeBookmark(notificationPostUri, notificationCid)
                            else
                                skywalker.getBookmarks().addBookmark(notificationPostUri, notificationCid)
                        }

                        onBookmarkLongPress: (mouseEvent) => {
                            if (!root.isActiveUser(owner.did))
                                return

                            root.bookmarkByNonActiveUser(mouseEvent, postStats, notification.ListView.view, notificationPostUri)
                        }

                        onShare: skywalker.sharePost(notificationPostUri)
                        onMuteThread: root.muteThread(notificationPostIsReply ? notificationPostReplyRootUri : notificationPostUri, notificationPostThreadMuted, owner.did)
                        onThreadgate: root.gateRestrictions(notificationPostThreadgateUri, notificationPostIsReply ? notificationPostReplyRootUri : notificationPostUri, notificationPostIsReply ? notificationPostReplyRootCid : notificationCid, notificationPostUri, notificationPostReplyRestriction, notificationPostReplyRestrictionLists, notificationPostHiddenReplies, owner.did)
                        onHideReply: root.hidePostReply(notificationPostThreadgateUri, notificationPostReplyRootUri, notificationPostReplyRootCid, notificationPostUri, notificationPostReplyRestriction, notificationPostReplyRestrictionLists, notificationPostHiddenReplies, owner.did)
                        onCopyPostText: skywalker.copyPostTextToClipboard(notificationPostPlainText)
                        onReportPost: root.reportPost(notificationPostUri, notificationCid, notificationPostText, notificationPostTimestamp, notificationAuthor, owner.did)
                        onTranslatePost: root.translateText(notificationPostPlainText)
                        onDetachQuote: (uri, detach) => root.detachQuote(uri, notificationPostUri, notificationCid, detach, owner.did)
                        onPin: root.pinPost(notificationPostUri, notificationCid, owner.did)
                        onUnpin: root.unpinPost(notificationCid, owner.did)
                        onBlockAuthor: root.blockAuthor(notificationPostAuthor, owner.did)
                        onShowEmojiNames: root.showEmojiNamesList(notificationPostPlainText)
                    }
                }
            }
        }

        Loader {
            id: aggregatableLoader
            Layout.preferredWidth: notification.width - guiSettings.threadColumnWidth - notification.margin * 2
            active: notificationIsAggregatable
            visible: status == Loader.Ready

            function movedOffScreen() {
                if (item)
                    item.movedOffScreen()
            }

            sourceComponent: Column {
                width: aggregatableLoader.width
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
                        userDid: owner.did
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
                            userDid: owner.did
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
                    DurationLabel {
                        Layout.alignment: Qt.AlignTop
                        durationSeconds: (new Date() - notificationTimestamp) / 1000
                    }
                }

                PostHeaderWithAvatar {
                    width: parent.width
                    userDid: owner.did
                    author: notificationReasonPostAuthor
                    postIndexedSecondsAgo: -1
                    visible: [QEnums.NOTIFICATION_REASON_LIKE_VIA_REPOST,
                              QEnums.NOTIFICATION_REASON_REPOST_VIA_REPOST].includes(notificationReason)
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
                    userDid: owner.did
                    postAuthor: notificationReasonPostAuthor
                    postText: {
                        if (notificationReasonPostLocallyDeleted)
                            return qsTr("🗑 Deleted")
                        else if (notificationReasonPostNotFound)
                            return qsTr("🗑 Not found")

                        return notificationReasonPostText
                    }
                    postPlainText: !notificationReasonPostLocallyDeleted && !notificationReasonPostNotFound ?
                                       notificationReasonPostPlainText : ""
                    postHasUnknownEmbed: notificationReasonPostHasUnknownEmbed
                    postUnknownEmbedType: notificationReasonPostUnknownEmbedType
                    postImages: notificationReasonPostImages
                    postLanguageLabels: notificationReasonPostLanguages
                    postContentLabels: notificationReasonPostLabels
                    postContentVisibility: QEnums.CONTENT_VISIBILITY_SHOW // User's own post
                    postContentWarning: ""
                    postMuted: QEnums.MUTED_POST_NONE
                    postIsThread: false
                    postIsThreadReply: false
                    postDateTime: notificationReasonPostTimestamp
                    postVideo: notificationReasonPostVideo
                    postExternal: notificationReasonPostExternal
                    postRecord: notificationReasonPostRecord
                    postRecordWithMedia: notificationReasonPostRecordWithMedia
                    bodyBackgroundColor: notification.color
                    borderColor: notificationIsRead ? guiSettings.borderColor : guiSettings.borderHighLightColor
                    visible: showPostForAggregatableReason()
                }
            }
        }

        Loader {
            id: inviteCodeLoader
            Layout.preferredWidth: parent.width - guiSettings.threadColumnWidth - notification.margin * 2
            active: notificationReason === QEnums.NOTIFICATION_REASON_INVITE_CODE_USED
            visible: status == Loader.Ready
            sourceComponent: Column {
                width: parent.width
                topPadding: 5

                RowLayout {
                    width: parent.width
                    Avatar {
                        id: usedByAvatar
                        Layout.preferredWidth: 34
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
                        `<b>${(UnicodeFonts.toCleanedHtml(notificationInviteCodeUsedBy.name))}</b> ` +
                        qsTr("used your invite code") + ": " + notificationInviteCode
                    }
                }
            }
        }

        // Separator
        Rectangle {
            Layout.preferredWidth: parent.width
            Layout.preferredHeight: 1
            Layout.columnSpan: 2
            color: notification.notificationIsRead ? guiSettings.separatorColor : guiSettings.separatorHighLightColor
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

    function openNotification() {
        if (notificationPostUri)
            skywalker.getPostThread(notificationPostUri)
        else if (notificationInviteCode)
            skywalker.getDetailedProfile(notificationInviteCodeUsedBy.did)
        else if (notificationIsAggregatable)
            showAuthorList()
    }

    function showAuthorList() {
        let title = "Users:"

        switch (notificationReason) {
        case QEnums.NOTIFICATION_REASON_LIKE:
        case QEnums.NOTIFICATION_REASON_LIKE_VIA_REPOST:
            title = qsTr("Liked by")
            break
        case QEnums.NOTIFICATION_REASON_FOLLOW:
            title = qsTr("New followers")
            break
        case QEnums.NOTIFICATION_REASON_REPOST:
        case QEnums.NOTIFICATION_REASON_REPOST_VIA_REPOST:
            title = qsTr("Reposted by")
            break
        case QEnums.NOTIFICATION_REASON_VERIFIED:
            title = qsTr("Verified by")
            break
        case QEnums.NOTIFICATION_REASON_UNVERIFIED:
            title = qsTr("Unverified by")
            break
        case QEnums.NOTIFICATION_REASON_NEW_LABELS:
            title = qsTr("Labelers")
            break
        }

        root.viewSimpleAuthorList(title, notificationAllAuthors, owner.did)
    }

    function showAvatarAsIcon() {
        let reasons = [QEnums.NOTIFICATION_REASON_MENTION,
                       QEnums.NOTIFICATION_REASON_REPLY,
                       QEnums.NOTIFICATION_REASON_QUOTE]
        return reasons.includes(notificationReason)
    }

    function showPost() {
        let reasons = [QEnums.NOTIFICATION_REASON_MENTION,
                       QEnums.NOTIFICATION_REASON_REPLY,
                       QEnums.NOTIFICATION_REASON_QUOTE,
                       QEnums.NOTIFICATION_REASON_SUBSCRIBED_POST]
        return reasons.includes(notificationReason)
    }

    function showPostForAggregatableReason() {
        let reasons = [QEnums.NOTIFICATION_REASON_LIKE,
                       QEnums.NOTIFICATION_REASON_LIKE_VIA_REPOST,
                       QEnums.NOTIFICATION_REASON_REPOST,
                       QEnums.NOTIFICATION_REASON_REPOST_VIA_REPOST]
        return reasons.includes(notificationReason)
    }

    function reasonText() {
        switch (notificationReason) {
        case QEnums.NOTIFICATION_REASON_LIKE:
            return qsTr("liked your post")
        case QEnums.NOTIFICATION_REASON_LIKE_VIA_REPOST:
            return qsTr("liked your repost")
        case QEnums.NOTIFICATION_REASON_FOLLOW:
            return qsTr("started following you")
        case QEnums.NOTIFICATION_REASON_REPOST:
            return qsTr("reposted your post")
        case QEnums.NOTIFICATION_REASON_REPOST_VIA_REPOST:
            return qsTr("reposted your repost")
        case QEnums.NOTIFICATION_REASON_MENTION:
            return qsTr("mentioned you")
        case QEnums.NOTIFICATION_REASON_REPLY:
            return qsTr("replied to you")
        case QEnums.NOTIFICATION_REASON_QUOTE:
            return qsTr("quoted you")
        case QEnums.NOTIFICATION_REASON_STARTERPACK_JOINED:
            return qsTr("joined via starter pack")
        case QEnums.NOTIFICATION_REASON_VERIFIED:
            return qsTr("verified you")
        case QEnums.NOTIFICATION_REASON_UNVERIFIED:
            return qsTr("deleted your verification")
        case QEnums.NOTIFICATION_REASON_NEW_LABELS:
            return qsTr("published new labels. Visit the labeler profile to see which labels are new.")
        case QEnums.NOTIFICATION_REASON_SUBSCRIBED_POST:
            return qsTr("posted")
        case QEnums.NOTIFICATION_REASON_UNKNOWN:
            return qsTr(`unknown notification: ${notificationReasonRaw}`)
        default:
            return "UNKNOW REASON: " + notificationReason
        }
    }

    function authorsAndReasonText() {
        return `<b>${(UnicodeFonts.toCleanedHtml(notificationAuthor.name))}</b> ` +
            (notificationOtherAuthors.length > 0 ?
                (notificationOtherAuthors.length > 1 ?
                    qsTr(`and ${(notificationOtherAuthors.length)} others `) :
                    qsTr(`and <b>${(UnicodeFonts.toCleanedHtml(notificationOtherAuthors[0].name))}</b> `)) :
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
        const reason = UnicodeFonts.toPlainText(authorsAndReasonText())
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
        if (notificationIsAggregatable)
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
