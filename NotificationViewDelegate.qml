import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Rectangle {
    property int margin: 8
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
    required property list<string> notificationReasonPostLabels
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
    required property int notificationPostRepostCount
    required property int notificationPostLikeCount
    required property int notificationPostReplyCount
    required property bool notificationPostNotFound
    required property list<string> notificationPostLabels
    required property int notificationPostContentVisibility // QEnums::PostContentVisibility
    required property string notificationPostContentWarning
    required property basicprofile replyToAuthor
    required property bool endOfList

    id: notification
    width: grid.width
    height: grid.height
    color: notificationIsRead ? "transparent" : guiSettings.postHighLightColor

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
            }
            SvgImage {
                x: parent.x + 14
                y: height + 5
                width: parent.width - 19
                height: width
                color: "palevioletred"
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
            Rectangle {
                x: parent.x + 14
                y: parent.y + 5
                width: parent.width - 19
                height: width
                radius: height / 2
                color: "blue"
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
                postText: notificationPostText
                postPlainText: notificationPostPlainText
                postImages: notificationPostImages
                postContentLabels: notificationPostLabels
                postContentVisibility: notificationPostContentVisibility
                postContentWarning: notificationPostContentWarning
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
                visible: !notificationPostNotFound
                authorIsUser: false

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

                onShare: skywalker.sharePost(notificationPostUri, notificationAuthor.handle)
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
                }
                Repeater {
                    model: Math.min(notificationOtherAuthors.length, 4)

                    Avatar {
                        required property int index

                        width: authorAvatar.width
                        height: width
                        avatarUrl: notificationOtherAuthors[index].avatarUrl

                        onClicked: skywalker.getDetailedProfile(notificationOtherAuthors[index].did)
                    }
                }
                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text: `+${(notificationOtherAuthors.length - 4)}`
                    visible: notificationOtherAuthors.length > 4
                }
            }

            RowLayout {
                width: parent.width

                Text {
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                    text: {
                        `<b>${notificationAuthor.name}</b> ` +
                        (notificationOtherAuthors.length > 0 ?
                            (notificationOtherAuthors.length > 1 ?
                                qsTr(`and ${(notificationOtherAuthors.length)} others `) :
                                qsTr(`and <b>${(notificationOtherAuthors[0].name)}</b> `)) :
                            "") +
                        reasonText()
                    }
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
                postDateTime: notificationReasonPostTimestamp
                postExternal: notificationReasonPostExternal
                postRecord: notificationReasonPostRecord
                postRecordWithMedia: notificationReasonPostRecordWithMedia
                visible: showPostForAggregatableReason()
            }
        }

        // Separator
        Rectangle {
            width: parent.width
            Layout.columnSpan: 2
            Layout.preferredHeight: 1
            Layout.fillWidth: true
            color: "lightgrey"
        }

        // End of feed indication
        Text {
            width: parent.width
            Layout.columnSpan: 2
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            topPadding: 10
            elide: Text.ElideRight
            color: Material.foreground
            text: qsTr("End of feed")
            font.italic: true
            visible: endOfList
        }
    }

    MouseArea {
        z: -2 // Let other mouse areas, e.g. images, get on top, -2 to allow records on top
        anchors.fill: parent
        onClicked: {
            console.debug("POST CLICKED:", notificationPostUri)
            if (notificationPostUri)
                skywalker.getPostThread(notificationPostUri)
        }
    }

    GuiSettings {
        id: guiSettings
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
        default:
            return "UNKNOW REASON: " + notificationReason
        }
    }
}
