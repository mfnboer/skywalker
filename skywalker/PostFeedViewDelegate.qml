import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Rectangle {
    readonly property int margin: 10
    readonly property int threadStyle: root.getSkywalker().getUserSettings().threadStyle
    readonly property string threadColor: root.getSkywalker().getUserSettings().threadColor

    required property basicprofile author
    required property string postUri
    required property string postCid
    required property string postText
    required property string postPlainText
    required property list<language> postLanguages
    required property date postIndexedDateTime
    required property basicprofile postRepostedByAuthor
    required property list<imageview> postImages
    required property var postExternal // externalview (var allows NULL)
    required property var postRecord // recordview
    required property var postRecordWithMedia // record_with_media_view
    required property int postType // QEnums::PostType
    required property int postThreadType // QEnums::ThreadPostType flags
    required property bool postIsPlaceHolder
    required property int postGapId;
    required property bool postNotFound;
    required property bool postBlocked;
    required property bool postNotSupported;
    required property string postUnsupportedType;
    required property bool postIsReply
    required property bool postParentInThread
    required property basicprofile postReplyToAuthor
    required property string postReplyRootUri
    required property string postReplyRootCid
    required property int postReplyCount
    required property int postRepostCount
    required property int postLikeCount
    required property string postRepostUri
    required property string postLikeUri
    required property bool postReplyDisabled
    required property int postReplyRestriction // QEnums::ReplyRestriction flags
    required property bool postBookmarked
    required property bool postBookmarkNotFound
    required property list<contentlabel> postLabels
    required property int postContentVisibility // QEnums::PostContentVisibility
    required property string postContentWarning
    required property int postMutedReason // QEnums::MutedPostReason
    required property string postHighlightColor
    required property bool postLocallyDeleted
    required property bool endOfFeed

    id: postEntry
    height: grid.height
    color: {
        if (postThreadType & QEnums.THREAD_ENTRY)
            return guiSettings.postHighLightColor
        else
            return guiSettings.backgroundColor
    }
    border.width: postThreadType & QEnums.THREAD_ENTRY ? 2 : 0
    border.color: guiSettings.borderColor

    Accessible.role: Accessible.Button
    Accessible.name: getSpeech()
    Accessible.onPressAction: performAccessiblePressAction()

    GridLayout {
        id: grid
        columns: 2
        width: parent.width
        rowSpacing: 0

        // Instead of using row spacing, these empty rectangles are used for white space.
        // This way we can color the background for threads.
        Rectangle {
            id: topLeftSpace
            Layout.leftMargin: 8 + (avatarImg.width - width) / 2
            width: threadStyle === QEnums.THREAD_STYLE_BAR ? avatarImg.width : guiSettings.threadLineWidth
            height: postEntry.margin * (!postParentInThread && (postType === QEnums.POST_REPLY || postType === QEnums.POST_LAST_REPLY) ? 2 : 1)

            color: {
                switch (postType) {
                case QEnums.POST_ROOT:
                    return postIsReply ? guiSettings.threadStartColor(threadColor) : "transparent"
                case QEnums.POST_REPLY:
                case QEnums.POST_LAST_REPLY:
                    return !postParentInThread ? "transparent" : guiSettings.threadMidColor(threadColor)
                case QEnums.POST_THREAD: {
                    if (postThreadType & QEnums.THREAD_FIRST_DIRECT_CHILD) {
                        return guiSettings.threadStartColor(threadColor)
                    } else if ((postThreadType & QEnums.THREAD_DIRECT_CHILD) ||
                               (postThreadType & QEnums.THREAD_ENTRY)){
                        return (postThreadType & QEnums.THREAD_TOP) ? "transparent" : guiSettings.threadEntryColor(threadColor)
                    } else if (postThreadType & QEnums.THREAD_TOP) {
                        return "transparent"
                    } else if (postThreadType & QEnums.THREAD_PARENT) {
                        return guiSettings.threadStartColor(threadColor)
                    }

                    return guiSettings.threadMidColor(threadColor)
                }
                default:
                    return "transparent"
                }
            }

            opacity: avatar.opacity

            Rectangle {
                y: postEntry.margin - (height / 2)
                width: parent.width
                height: 6
                color: guiSettings.threadMidColor(threadColor)
                visible: !postParentInThread && (postType === QEnums.POST_REPLY || postType === QEnums.POST_LAST_REPLY)
            }
        }
        Rectangle {
            width: parent.width - guiSettings.threadColumnWidth - postEntry.margin * 2
            Layout.preferredHeight: topLeftSpace.height
            color: "transparent"
        }

        // Repost information
        Rectangle {
            width: guiSettings.threadColumnWidth
            Layout.fillHeight: true
            color: guiSettings.backgroundColor
            visible: !postRepostedByAuthor.isNull() && !postGapId && !postLocallyDeleted

            SvgImage {
                anchors.right: parent.right
                width: repostedByText.height
                height: repostedByText.height
                color: Material.color(Material.Grey)
                svg: svgOutline.repost
            }
        }
        SkyCleanedText {
            id: repostedByText
            Layout.fillWidth: true
            elide: Text.ElideRight
            plainText: qsTr(`Reposted by ${postRepostedByAuthor.name}`)
            color: Material.color(Material.Grey)
            font.bold: true
            font.pointSize: guiSettings.scaledFont(7/8)
            visible: !postRepostedByAuthor.isNull() && !postGapId && !postLocallyDeleted

            Accessible.ignored: true

            MouseArea {
                anchors.fill: parent
                onClicked: skywalker.getDetailedProfile(postRepostedByAuthor.did)
            }
        }

        // Author and content
        Rectangle {
            id: avatar
            width: guiSettings.threadColumnWidth
            Layout.fillHeight: true
            color: "transparent"
            opacity: 0.9

            Rectangle {
                x: avatarImg.x + (avatarImg.width - width) / 2
                y: ((postType === QEnums.POST_ROOT && !postIsReply) || (postThreadType & QEnums.THREAD_TOP)) ? avatarImg.y + avatarImg.height / 2 : 0
                width: threadStyle === QEnums.THREAD_STYLE_LINE ? guiSettings.threadLineWidth : avatarImg.width
                height: ((postType === QEnums.POST_LAST_REPLY) || (postThreadType & QEnums.THREAD_LEAF)) && postReplyCount === 0 ? avatarImg.y + avatarImg.height / 2 - y : parent.height - y

                // Gradient is used display thread context.
                gradient: Gradient {
                    GradientStop {
                        position: 0.0
                        color: {
                            switch (postType) {
                            case QEnums.POST_ROOT:
                                return guiSettings.threadStartColor(threadColor)
                            case QEnums.POST_REPLY:
                            case QEnums.POST_LAST_REPLY:
                                return guiSettings.threadMidColor(threadColor)
                            case QEnums.POST_THREAD: {
                                if (postThreadType & QEnums.THREAD_ENTRY) {
                                    return guiSettings.threadEntryColor(threadColor)
                                } else if ((postThreadType & QEnums.THREAD_PARENT) ||
                                        (postThreadType & QEnums.THREAD_DIRECT_CHILD)) {
                                    return guiSettings.threadStartColor(threadColor)
                                }

                                return guiSettings.threadMidColor(threadColor)
                            }
                            default:
                                return guiSettings.backgroundColor
                            }
                        }
                    }
                    GradientStop {
                        position: 1.0
                        color: {
                            switch (postType) {
                            case QEnums.POST_STANDALONE:
                                return guiSettings.backgroundColor
                            case QEnums.POST_LAST_REPLY:
                                return guiSettings.threadEndColor(threadColor)
                            case QEnums.POST_THREAD: {
                                if (postThreadType & QEnums.THREAD_ENTRY) {
                                    return guiSettings.threadEntryColor(threadColor)
                                } else if (postThreadType & QEnums.THREAD_PARENT) {
                                    return guiSettings.threadStartColor(threadColor)
                                } else if (postThreadType & QEnums.THREAD_LEAF) {
                                    return guiSettings.threadEndColor(threadColor)
                                }

                                return guiSettings.threadMidColor(threadColor)
                            }
                            default:
                                return guiSettings.threadMidColor(threadColor)
                            }
                        }
                    }
                }
            }

            Avatar {
                id: avatarImg
                x: avatar.x + 8
                y: postHeader.y + 5 // For some reason "avatar.y + 5" does not work when it is a repost
                width: parent.width - 13
                avatarUrl: author.avatarUrl
                isModerator: author.associated.isLabeler
                visible: !postIsPlaceHolder && !postLocallyDeleted

                onClicked: skywalker.getDetailedProfile(author.did)

                Accessible.role: Accessible.Button
                Accessible.name: qsTr(`show profile of ${author.name}`)
                Accessible.onPressAction: clicked()
            }
        }
        Column {
            id: postColumn
            // Change from width to Layout.preferredWidth seems to solve the issue
            // where posts sometimes are too wide (like landscape mode) but makes
            // things very slow :-(
            width: parent.width - guiSettings.threadColumnWidth - postEntry.margin * 2
            visible: !postIsPlaceHolder && !postLocallyDeleted

            PostHeader {
                id: postHeader
                width: parent.width
                authorName: author.name
                authorHandle: author.handle
                postThreadType: postEntry.postThreadType
                postIndexedSecondsAgo: (new Date() - postEntry.postIndexedDateTime) / 1000
            }

            // Reply to
            ReplyToRow {
                width: parent.width
                authorName: postReplyToAuthor.name
                visible: postIsReply && (!postParentInThread || postType === QEnums.POST_ROOT)
            }

            PostBody {
                id: postBody
                width: parent.width
                postAuthor: author
                postText: postEntry.postText
                postPlainText: postEntry.postPlainText
                postImages: postEntry.postImages
                postLanguageLabels: postLanguages
                postContentLabels: postLabels
                postContentVisibility: postEntry.postContentVisibility
                postContentWarning: postEntry.postContentWarning
                postMuted: postEntry.postMutedReason
                postExternal: postEntry.postExternal
                postRecord: postEntry.postRecord
                postRecordWithMedia: postEntry.postRecordWithMedia
                postDateTime: postEntry.postIndexedDateTime
                detailedView: postThreadType & QEnums.THREAD_ENTRY
                ellipsisBackgroundColor: postEntry.color
                postHighlightColor: postEntry.postHighlightColor
            }

            // Reposts and likes in detailed view of post entry in thread view
            Row {
                width: parent.width
                topPadding: 10
                bottomPadding: 5
                visible: postThreadType & QEnums.THREAD_ENTRY

                StatAuthors {
                    rightPadding: 30
                    atUri: postUri
                    count: postRepostCount
                    nameSingular: qsTr("repost")
                    namePlural: qsTr("reposts")
                    authorListType: QEnums.AUTHOR_LIST_REPOSTS
                    authorListHeader: qsTr("Reposted by")
                }
                StatAuthors {
                    atUri: postUri
                    count: postLikeCount
                    nameSingular: qsTr("like")
                    namePlural: qsTr("likes")
                    authorListType: QEnums.AUTHOR_LIST_LIKES
                    authorListHeader: qsTr("Liked by")
                }
            }

            // Stats
            PostStats {
                width: parent.width
                topPadding: 10
                replyCount: postReplyCount
                repostCount: postRepostCount
                likeCount: postLikeCount
                repostUri: postRepostUri
                likeUri: postLikeUri
                replyDisabled: postReplyDisabled
                authorIsUser: isUser(author)
                isBookmarked: postBookmarked
                bookmarkNotFound: postBookmarkNotFound

                onReply: {
                    const lang = postLanguages.length > 0 ? postLanguages[0].shortCode : ""
                    root.composeReply(postUri, postCid, postText, postIndexedDateTime,
                                      author, postReplyRootUri, postReplyRootCid, lang)
                }

                onRepost: {
                    root.repost(postRepostUri, postUri, postCid, postText,
                                postIndexedDateTime, author)
                }

                onLike: root.like(postLikeUri, postUri, postCid)

                onBookmark: {
                    if (isBookmarked) {
                        skywalker.bookmarks.removeBookmark(postUri)
                    }
                    else {
                        const bookmarked = skywalker.bookmarks.addBookmark(postUri)

                        if (!bookmarked)
                            skywalker.showStatusMessage(qsTr("Your bookmarks are full!"), QEnums.STATUS_LEVEL_ERROR)
                    }
                }

                onShare: skywalker.sharePost(postUri)
                onDeletePost: confirmDelete()
                onCopyPostText: skywalker.copyPostTextToClipboard(postPlainText)
                onReportPost: root.reportPost(postUri, postCid, postText, postIndexedDateTime, author)
                onTranslatePost: root.translateText(postPlainText)
            }
        }

        // Gap place holder
        Text {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
            color: guiSettings.linkColor
            text: qsTr("Show more posts")
            visible: postGapId > 0

            MouseArea {
                anchors.fill: parent
                onClicked: getGapPosts()
            }
        }

        // NOT FOUND place holder
        Text {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
            color: guiSettings.textColor
            text: qsTr("NOT FOUND")
            visible: postNotFound
        }

        // BLOCKED place holder
        Text {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
            color: guiSettings.textColor
            text: qsTr("BLOCKED")
            visible: postBlocked
        }

        // NOT SUPPORTED place holder
        Text {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
            color: guiSettings.textColor
            text: qsTr("NOT SUPPORTED")
            visible: postNotSupported
        }
        Text {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
            maximumLineCount: 2
            elide: Text.ElideRight
            color: Material.color(Material.Grey)
            font.pointSize: guiSettings.scaledFont(7/8)
            text: postUnsupportedType
            visible: postNotSupported
        }

        // Locally deleted
        Text {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
            color: guiSettings.textColor
            text: qsTr("DELETED")
            visible: postLocallyDeleted
        }

        // Instead of using row spacing, these empty rectangles are used for white space.
        // This way we can color the background for threads.
        Rectangle {
            width: guiSettings.threadColumnWidth
            height: postEntry.margin
            color: "transparent"

            Rectangle {
                x: 8 + (avatarImg.width - width) / 2
                width: threadStyle === QEnums.THREAD_STYLE_BAR ? avatarImg.width : guiSettings.threadLineWidth
                height: parent.height
                opacity: avatar.opacity
                visible: !((postType === QEnums.POST_LAST_REPLY) || (postThreadType & QEnums.THREAD_LEAF))

                color: {
                    switch (postType) {
                    case QEnums.POST_ROOT:
                    case QEnums.POST_REPLY:
                        return guiSettings.threadMidColor(threadColor)
                    case QEnums.POST_THREAD: {
                        if (postThreadType & QEnums.THREAD_ENTRY)  {
                            return guiSettings.threadEntryColor(threadColor)
                        }
                        if (postThreadType & QEnums.THREAD_LEAF) {
                            return guiSettings.backgroundColor
                        } else if (postThreadType & QEnums.THREAD_PARENT)  {
                            return guiSettings.threadStartColor(threadColor)
                        }

                        return guiSettings.threadMidColor(threadColor)
                    }
                    default:
                        return guiSettings.backgroundColor
                    }
                }
            }
        }
        Rectangle {
            width: parent.width - guiSettings.threadColumnWidth - postEntry.margin * 2
            height: postEntry.margin
            color: "transparent"
        }

        // Post/Thread separator
        Rectangle {
            width: parent.width
            Layout.columnSpan: 2
            Layout.preferredHeight: 1
            color: guiSettings.separatorColor
            visible: [QEnums.POST_STANDALONE, QEnums.POST_LAST_REPLY].includes(postType) ||
                (postThreadType & QEnums.THREAD_LEAF)
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
            visible: endOfFeed
        }
    }

    MouseArea {
        z: -2 // Let other mouse areas, e.g. images, get on top, -2 to allow records on top
        anchors.fill: parent
        enabled: !(postThreadType & QEnums.THREAD_ENTRY) && !postBookmarkNotFound
        onClicked: openPostThread()
    }

    UnicodeFonts {
        id: unicodeFonts
    }

    AccessibilityUtils {
        id: accessibilityUtils
    }

    GuiSettings {
        id: guiSettings
    }

    function confirmDelete() {
        guiSettings.askYesNoQuestion(
                    postEntry,
                    qsTr("Do you really want to your post?"),
                    () => root.deletePost(postUri, postCid))
    }

    function openPostThread() {
        if (!(postThreadType & QEnums.THREAD_ENTRY) && !postBookmarkNotFound)
        {
            if (postUri)
                skywalker.getPostThread(postUri)
        }
    }

    function getGapPosts() {
        if (!skywalker.getTimelineInProgress)
            skywalker.getTimelineForGap(postGapId, 3, true)
    }

    function performAccessiblePressAction() {
        if (postLocallyDeleted)
            return

        if (postIsPlaceHolder) {
            if (postGapId > 0)
                getGapPosts()

            return
        }

        if (postBody.postVisible())
            openPostThread()
    }

    function getSpeech() {
        if (postLocallyDeleted)
            return qsTr("deleted post")

        if (postIsPlaceHolder) {
            if (postGapId > 0)
                return qsTr("show more more posts")

            return accessibilityUtils.getPostNotAvailableSpeech(
                    postNotFound, postBlocked, postNotSupported)
        }

        if (!postBody.postVisible())
            return getHiddenPostSpeech()

        return accessibilityUtils.getPostSpeech(postIndexedDateTime, author, postPlainText,
                postImages, postExternal, postRecord, postRecordWithMedia,
                postRepostedByAuthor, postIsReply, postReplyToAuthor)
    }

    function getHiddenPostSpeech() {
        if (postContentVisibility === QEnums.CONTENT_VISIBILITY_HIDE_POST)
            return postContentWarning

        if (postBody.mutePost)
            return postBody.getMuteText()

        return postContentWarning
    }

    function isUser(author) {
        return skywalker.getUserDid() === author.did
    }
}
