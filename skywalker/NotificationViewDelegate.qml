import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Rectangle {
    property int margin: 8
    required property int index
    required property int viewWidth
    required property basicprofile notificationAuthor
    required property list<basicprofile> notificationOtherAuthors
    required property int notificationReason // QEnums::NotificationReason
    required property string notificationReasonSubjectUri
    required property string notificationReasonSubjectCid
    required property string notificationReasonPostText
    required property string notificationReasonPostPlainText
    required property list<imageview> notificationReasonPostImages
    required property var notificationReasonPostExternal // externalview (var allows NULL)
    required property var notificationReasonPostRecord // recordview
    required property var notificationReasonPostRecordWithMedia // record_with_media_view
    required property date notificationReasonPostTimestamp
    required property bool notificationReasonPostNotFound
    required property list<contentlabel> notificationReasonPostLabels
    required property bool notificationReasonPostLocallyDeleted
    required property date notificationTimestamp
    required property bool notificationIsRead
    required property string notificationPostUri
    required property string notificationCid
    required property string notificationPostText
    required property string notificationPostPlainText
    required property date notificationPostTimestamp
    required property list<imageview> notificationPostImages
    required property var notificationPostExternal // externalview (var allows NULL)
    required property var notificationPostRecord // recordview
    required property var notificationPostRecordWithMedia // record_with_media_view
    required property string notificationPostReplyRootUri
    required property string notificationPostReplyRootCid
    required property string notificationPostRepostUri
    required property string notificationPostLikeUri
    required property bool notificationPostReplyDisabled
    required property int notificationPostReplyRestriction // QEnums::ReplyRestriction
    required property int notificationPostRepostCount
    required property int notificationPostLikeCount
    required property int notificationPostReplyCount
    required property bool notificationPostBookmarked
    required property bool notificationPostNotFound
    required property list<contentlabel> notificationPostLabels
    required property int notificationPostContentVisibility // QEnums::PostContentVisibility
    required property string notificationPostContentWarning
    required property int notificationPostMutedReason // QEnums::MutedPostReason
    required property basicprofile replyToAuthor
    required property string notificationInviteCode
    required property basicprofile notificationInviteCodeUsedBy
    required property bool endOfList

    id: notification
    width: grid.width
    height: grid.height
    color: notificationIsRead ? "transparent" : guiSettings.postHighLightColor

    Accessible.role: Accessible.Button
    Accessible.name: getSpeech()
    Accessible.onPressAction: openNotification()

    GridLayout {
        id: grid
        columns: 2
        width: viewWidth
        rowSpacing: 5

        // Author and content
        Rectangle {
            id: avatar
            width: guiSettings.threadBarWidth * 5
            height: avatarImg.height + 5
            Layout.fillHeight: true
            color: "transparent"

            Avatar {
                id: avatarImg
                x: parent.x + 8
                y: postHeader.y + 5
                width: parent.width - 13
                height: width
                avatarUrl: notificationAuthor.avatarUrl
                visible: showPost()

                onClicked: skywalker.getDetailedProfile(notificationAuthor.did)

                Accessible.role: Accessible.Button
                Accessible.name: qsTr(`show profile of ${notificationAuthor.name}`)
                Accessible.onPressAction: clicked()
            }
            SvgImage {
                x: parent.x + 14
                y: height + 5
                width: parent.width - 19
                height: width
                color: guiSettings.likeColor
                svg: svgFilled.like
                visible: notificationReason === QEnums.NOTIFICATION_REASON_LIKE
            }
            SvgImage {
                x: parent.x + 14
                y: height + 5
                width: parent.width - 19
                height: width
                color: guiSettings.textColor
                svg: svgOutline.repost
                visible: notificationReason === QEnums.NOTIFICATION_REASON_REPOST
            }
            SvgImage {
                x: parent.x + 14
                y: height + 5
                width: parent.width - 19
                height: width
                color: guiSettings.textColor
                svg: svgOutline.inviteCode
                visible: notificationReason === QEnums.NOTIFICATION_REASON_INVITE_CODE_USED
            }
            Rectangle {
                x: parent.x + 14
                y: parent.y + 5
                width: parent.width - 19
                height: width
                radius: height / 2
                color: guiSettings.avatarDefaultColor
                visible: notificationReason === QEnums.NOTIFICATION_REASON_FOLLOW

                SvgImage {
                    x: 5
                    y: height + 5
                    width: parent.width - 10
                    height: width
                    color: "white"
                    svg: svgFilled.newFollower
                }
            }
        }

        Column {
            id: postColumn
            width: parent.width - avatar.width - notification.margin * 2
            visible: showPost()
            topPadding: 5

            PostHeader {
                id: postHeader
                width: parent.width
                Layout.fillWidth: true
                authorName: notificationAuthor.name
                authorHandle: notificationAuthor.handle
                postThreadType: QEnums.THREAD_NONE
                postIndexedSecondsAgo: (new Date() - notificationTimestamp) / 1000
            }

            // Reply to
            ReplyToRow {
                width: parent.width
                authorName: replyToAuthor.name
                visible: notificationReason === QEnums.NOTIFICATION_REASON_REPLY
            }

            PostBody {
                id: postBody
                width: parent.width
                Layout.fillWidth: true
                postAuthor: notificationAuthor
                postText: notificationPostText
                postPlainText: notificationPostPlainText
                postImages: notificationPostImages
                postContentLabels: notificationPostLabels
                postContentVisibility: notificationPostContentVisibility
                postContentWarning: notificationPostContentWarning
                postMuted: notificationPostMutedReason
                postExternal: notificationPostExternal
                postRecord: notificationPostRecord
                postRecordWithMedia: notificationPostRecordWithMedia
                postDateTime: notificationPostTimestamp
            }

            PostStats {
                width: parent.width
                topPadding: 10
                replyCount: notificationPostReplyCount
                repostCount: notificationPostRepostCount
                likeCount: notificationPostLikeCount
                repostUri: notificationPostRepostUri
                likeUri: notificationPostLikeUri
                replyDisabled: notificationPostReplyDisabled
                visible: !notificationPostNotFound
                authorIsUser: false
                isBookmarked: notificationPostBookmarked
                bookmarkNotFound: false

                onReply: {
                    root.composeReply(notificationPostUri, notificationCid, notificationPostText,
                                      notificationPostTimestamp, notificationAuthor,
                                      notificationPostReplyRootUri, notificationPostReplyRootCid)
                }

                onRepost: {
                    root.repost(notificationPostRepostUri, notificationPostUri, notificationCid,
                                notificationPostText, notificationPostTimestamp,
                                notificationAuthor)
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

                onShare: skywalker.sharePost(notificationPostUri, notificationAuthor)
                onCopyPostText: skywalker.copyPostTextToClipboard(notificationPostPlainText)
                onReportPost: root.reportPost(notificationPostUri, notificationCid, notificationPostText, notificationPostTimestamp, notificationAuthor)
                onTranslatePost: root.translateText(notificationPostPlainText)
            }
        }
        Column {
            width: parent.width - avatar.width - notification.margin * 2
            topPadding: 5
            visible: isAggregatableReason()

            Row {
                spacing: 5
                Avatar {
                    id: authorAvatar
                    width: 34
                    height: width
                    avatarUrl: notificationAuthor.avatarUrl

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
                        avatarUrl: notificationOtherAuthors[index].avatarUrl

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
                }
            }

            RowLayout {
                width: parent.width

                Text {
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                    color: guiSettings.textColor
                    text: authorsAndReasonText()
                }
                Text {
                    text: guiSettings.durationToString((new Date() - notificationTimestamp) / 1000)
                    font.pointSize: guiSettings.scaledFont(7/8)
                    color: Material.color(Material.Grey)
                }
            }

            PostBody {
                topPadding: 5
                width: parent.width
                Layout.fillWidth: true
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
                postContentLabels: notificationReasonPostLabels
                postContentVisibility: QEnums.CONTENT_VISIBILITY_SHOW // User's own post
                postContentWarning: ""
                postMuted: QEnums.MUTED_POST_NONE
                postDateTime: notificationReasonPostTimestamp
                postExternal: notificationReasonPostExternal
                postRecord: notificationReasonPostRecord
                postRecordWithMedia: notificationReasonPostRecordWithMedia
                visible: showPostForAggregatableReason()
            }
        }
        Column {
            width: parent.width - avatar.width - notification.margin * 2
            topPadding: 5
            visible: notificationReason === QEnums.NOTIFICATION_REASON_INVITE_CODE_USED

            RowLayout {
                width: parent.width
                Avatar {
                    id: usedByAvatar
                    width: 34
                    height: width
                    avatarUrl: notificationInviteCodeUsedBy.avatarUrl

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
                wrapMode: Text.Wrap
                color: guiSettings.textColor
                text: {
                    `<b>${notificationInviteCodeUsedBy.name}</b> ` +
                    qsTr("used your invite code") + ": " + notificationInviteCode
                }
            }
        }

        // Separator
        Rectangle {
            width: parent.width
            Layout.columnSpan: 2
            Layout.preferredHeight: 1
            Layout.fillWidth: true
            color: guiSettings.separatorColor
        }

        // End of feed indication
        Text {
            width: parent.width
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
                       QEnums.NOTIFICATION_REASON_REPOST]
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
        default:
            return "UNKNOW REASON: " + notificationReason
        }
    }

    function authorsAndReasonText() {
        return `<b>${notificationAuthor.name}</b> ` +
            (notificationOtherAuthors.length > 0 ?
                (notificationOtherAuthors.length > 1 ?
                    qsTr(`and ${(notificationOtherAuthors.length)} others `) :
                    qsTr(`and <b>${(notificationOtherAuthors[0].name)}</b> `)) :
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
                accessibilityUtils.nullAuthor, false, accessibilityUtils.nullAuthor)
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
                notificationReason === QEnums.NOTIFICATION_REASON_REPLY, replyToAuthor)

        let speech = `${time} ${notificationAuthor.name} ${reason}\n\n${postSpeech}`
        return speech
    }

    function getSpeech() {
        if (isAggregatableReason())
            return getAggregatableSpeech()

        if (showPost())
            return getPostSpeech()
    }
}
