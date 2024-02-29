import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Rectangle {
    property int margin: 8
    required property int viewWidth

    required property basicprofile author
    required property string postUri
    required property string postCid
    required property string postText
    required property string postPlainText
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
    required property bool postLocallyDeleted
    required property bool endOfFeed

    id: postEntry
    width: grid.width
    height: grid.height
    color: {
        if (postThreadType & QEnums.THREAD_ENTRY)
            return guiSettings.postHighLightColor
        else
            return "transparent"
    }
    border.width: postThreadType & QEnums.THREAD_ENTRY ? 2 : 0
    border.color: guiSettings.borderColor

    Accessible.role: Accessible.Button
    Accessible.name: getSpeech()
    Accessible.onPressAction: performAccessiblePressAction()

    GridLayout {
        id: grid
        columns: 2
        width: viewWidth
        rowSpacing: 0

        // Instead of using row spacing, these empty rectangles are used for white space.
        // This way we can color the background for threads.
        RowLayout {
            id: topLeftSpace
            width: avatar.width
            height: postEntry.margin * (postIsReply && !postParentInThread ? 2 : 1)
            spacing: 0

            Repeater {
                model: guiSettings.threadBarWidth

                Rectangle {
                    required property int index

                    width: avatar.width / guiSettings.threadBarWidth
                    Layout.preferredHeight: topLeftSpace.height
                    color: {
                        switch (postType) {
                        case QEnums.POST_REPLY:
                        case QEnums.POST_LAST_REPLY:
                            return !postParentInThread && index % 2 === 0 ? "transparent" : guiSettings.threadMidColor
                        case QEnums.POST_THREAD: {
                            if (postThreadType & QEnums.THREAD_FIRST_DIRECT_CHILD) {
                                return guiSettings.threadStartColor
                            } else if ((postThreadType & QEnums.THREAD_DIRECT_CHILD) ||
                                       (postThreadType & QEnums.THREAD_ENTRY)){
                                return guiSettings.threadEntryColor
                            } else if (postThreadType & QEnums.THREAD_TOP) {
                                return "transparent"
                            } else if (postThreadType & QEnums.THREAD_PARENT) {
                                return guiSettings.threadStartColor
                            }

                            return guiSettings.threadMidColor
                        }
                        default:
                            return "transparent"
                        }
                    }
                    opacity: avatar.opacity
                }
            }
        }
        Rectangle {
            width: parent.width - avatar.width - postEntry.margin * 2
            Layout.preferredHeight: topLeftSpace.height
            Layout.fillWidth: true
            color: "transparent"
        }

        // Repost information
        Rectangle {
            width: avatar.width
            height: repostedByText.height
            color: "transparent"
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
            width: parent.width - avatar.width - postEntry.margin * 2
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
            width: guiSettings.threadBarWidth * 5
            Layout.fillHeight: true
            opacity: 0.9

            // Gradient is used display thread context.
            gradient: Gradient {
                GradientStop {
                    position: 0.0
                    color: {
                        switch (postType) {
                        case QEnums.POST_ROOT:
                            return guiSettings.threadStartColor
                        case QEnums.POST_REPLY:
                        case QEnums.POST_LAST_REPLY:
                            return guiSettings.threadMidColor
                        case QEnums.POST_THREAD: {
                            if (postThreadType & QEnums.THREAD_ENTRY) {
                                return guiSettings.threadEntryColor
                            } else if ((postThreadType & QEnums.THREAD_PARENT) ||
                                    (postThreadType & QEnums.THREAD_DIRECT_CHILD)) {
                                return guiSettings.threadStartColor
                            }

                            return guiSettings.threadMidColor
                        }
                        default:
                            return "transparent"
                        }
                    }
                }
                GradientStop {
                    position: 1.0
                    color: {
                        switch (postType) {
                        case QEnums.POST_STANDALONE:
                            return "transparent"
                        case QEnums.POST_LAST_REPLY:
                            return guiSettings.threadEndColor
                        case QEnums.POST_THREAD: {
                            if (postThreadType & QEnums.THREAD_ENTRY) {
                                return guiSettings.threadEntryColor
                            } else if (postThreadType & QEnums.THREAD_PARENT) {
                                return guiSettings.threadStartColor
                            } else if (postThreadType & QEnums.THREAD_LEAF) {
                                return guiSettings.threadEndColor
                            }

                            return guiSettings.threadMidColor
                        }
                        default:
                            return guiSettings.threadMidColor
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
                visible: !postIsPlaceHolder && !postLocallyDeleted

                onClicked: skywalker.getDetailedProfile(author.did)

                Accessible.role: Accessible.Button
                Accessible.name: qsTr(`show profile of ${author.name}`)
                Accessible.onPressAction: clicked()
            }
        }
        Column {
            id: postColumn
            // Changfrom width to Layout.preferredWidth seems to solve the issue
            // where posts sometimes are too wide (like landscape mode) but makes
            // things very slow :-(
            width: parent.width - avatar.width - postEntry.margin * 2
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
                postContentLabels: postLabels
                postContentVisibility: postEntry.postContentVisibility
                postContentWarning: postEntry.postContentWarning
                postMuted: postEntry.postMutedReason
                postExternal: postEntry.postExternal
                postRecord: postEntry.postRecord
                postRecordWithMedia: postEntry.postRecordWithMedia
                postDateTime: postEntry.postIndexedDateTime
                detailedView: postThreadType & QEnums.THREAD_ENTRY
            }

            // Reposts and likes in detailed view of post entry in thread view
            Row {
                width: parent.width
                topPadding: 10
                bottomPadding: 5
                visible: postThreadType & QEnums.THREAD_ENTRY

                Text {
                    rightPadding: 30
                    color: guiSettings.linkColor
                    textFormat: Text.StyledText
                    text: postRepostCount > 1 ? qsTr(`<b>${postRepostCount}</b> reposts`) : qsTr(`<b>${postRepostCount}</b> repost`)
                    visible: postRepostCount

                    Accessible.role: Accessible.Link
                    Accessible.name: unicodeFonts.toPlainText(text)
                    Accessible.onPressAction: showReposts()

                    MouseArea {
                        anchors.fill: parent
                        onClicked: parent.showReposts()
                    }

                    function showReposts() {
                        let modelId = skywalker.createAuthorListModel(
                                QEnums.AUTHOR_LIST_REPOSTS, postUri)
                        root.viewAuthorList(modelId, qsTr("Reposted by"));
                    }
                }
                Text {
                    color: guiSettings.linkColor
                    textFormat: Text.StyledText
                    text: postLikeCount > 1 ? qsTr(`<b>${postLikeCount}</b> likes`) : qsTr(`<b>${postLikeCount}</b> like`)
                    visible: postLikeCount

                    Accessible.role: Accessible.Link
                    Accessible.name: unicodeFonts.toPlainText(text)
                    Accessible.onPressAction: showLikes()

                    MouseArea {
                        anchors.fill: parent
                        onClicked: parent.showLikes()
                    }

                    function showLikes() {
                        let modelId = skywalker.createAuthorListModel(
                                QEnums.AUTHOR_LIST_LIKES, postUri)
                        root.viewAuthorList(modelId, qsTr("Liked by"));
                    }
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
                    root.composeReply(postUri, postCid, postText, postIndexedDateTime,
                                      author, postReplyRootUri, postReplyRootCid)
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
                onDeletePost: root.deletePost(postUri, postCid)
                onCopyPostText: skywalker.copyPostTextToClipboard(postPlainText)
                onReportPost: root.reportPost(postUri, postCid, postText, postIndexedDateTime, author)
                onTranslatePost: root.translateText(postPlainText)
            }
        }

        // Gap place holder
        Text {
            width: parent.width
            Layout.columnSpan: 2
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
            textFormat: Text.StyledText
            text: `<a href=\"showMore\" style=\"color: ${guiSettings.linkColor};\">` + qsTr("Show more posts") + "</a>"
            visible: postGapId > 0

            onLinkActivated: getGapPosts()

            MouseArea {
                anchors.fill: parent
                cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
                acceptedButtons: Qt.NoButton
            }
        }

        // NOT FOUND place holder
        Text {
            width: parent.width
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
            width: parent.width
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
            width: parent.width
            Layout.columnSpan: 2
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
            color: guiSettings.textColor
            text: qsTr("NOT SUPPORTED")
            visible: postNotSupported
        }
        Text {
            width: parent.width
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
            width: parent.width
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
            width: avatar.width
            height: postEntry.margin
            color: {
                switch (postType) {
                case QEnums.POST_ROOT:
                case QEnums.POST_REPLY:
                    return guiSettings.threadMidColor
                case QEnums.POST_THREAD: {
                    if (postThreadType & QEnums.THREAD_ENTRY)  {
                        return guiSettings.threadEntryColor
                    }
                    if (postThreadType & QEnums.THREAD_LEAF) {
                        return "transparent"
                    } else if (postThreadType & QEnums.THREAD_PARENT)  {
                        return guiSettings.threadStartColor
                    }

                    return guiSettings.threadMidColor
                }
                default:
                    return "transparent"
                }
            }
            opacity: avatar.opacity
        }
        Rectangle {
            width: parent.width - avatar.width - postEntry.margin * 2
            height: postEntry.margin
            Layout.fillWidth: true
            color: "transparent"
        }

        // Post/Thread separator
        Rectangle {
            width: parent.width
            Layout.columnSpan: 2
            Layout.preferredHeight: 1
            Layout.fillWidth: true
            color: guiSettings.separatorColor
            visible: [QEnums.POST_STANDALONE, QEnums.POST_LAST_REPLY].includes(postType) ||
                (postThreadType & QEnums.THREAD_LEAF)
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

    function openPostThread() {
        if (!(postThreadType & QEnums.THREAD_ENTRY) && !postBookmarkNotFound)
        {
            if (postUri)
                skywalker.getPostThread(postUri)
        }
    }

    function getGapPosts() {
        if (!skywalker.getTimelineInProgress)
            skywalker.getTimelineForGap(postGapId)
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
