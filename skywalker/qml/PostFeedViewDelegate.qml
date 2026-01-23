import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker

Rectangle {
    readonly property int margin: 10
    required property string userDid
    property Skywalker skywalker: root.getSkywalker(userDid)
    readonly property int threadStyle: skywalker.getUserSettings().threadStyle
    readonly property string threadColor: skywalker.getUserSettings().threadColor

    required property int index
    required property basicprofile author
    required property string postUri
    required property string postCid
    required property string postText
    required property string postPlainText
    required property list<language> postLanguages
    required property string postIdentifiedLanguage
    required property string postTranslatedText
    required property date postIndexedDateTime
    required property double postIndexedSecondsAgo
    required property basicprofile postRepostedByAuthor
    required property string postReasonRepostUri;
    required property string postReasonRepostCid;
    required property bool postHasUnknownEmbed
    required property string postUnknownEmbedType
    required property list<imageview> postImages
    required property var postVideo // videoView
    required property var postExternal // externalview (var allows NULL)
    required property var postRecord // recordview
    required property var postRecordWithMedia // record_with_media_view
    required property int postType // QEnums::PostType
    required property int postFoldedType // QEnums::PostFoldedType
    required property int postThreadType // QEnums::ThreadPostType flags
    required property bool postIsPlaceHolder
    required property int postGapId;
    required property bool postHiddenPosts;
    required property bool postNotFound;
    required property bool postBlocked;
    required property blockedauthor postBlockedAuthor;
    required property bool postNotSupported;
    required property string postUnsupportedType;
    required property bool postIsReply
    required property bool postParentInThread
    required property basicprofile postReplyToAuthor
    required property string postReplyRootAuthorDid
    required property string postReplyRootUri
    required property string postReplyRootCid
    required property list<string> postMentionDids
    required property string postFeedContext
    required property int postReplyCount
    required property int postRepostCount
    required property int postLikeCount
    required property int postQuoteCount
    required property string postRepostUri
    required property string postLikeUri
    required property bool postLikeTransient
    required property bool postThreadMuted
    required property bool postReplyDisabled
    required property bool postEmbeddingDisabled
    required property bool postViewerStatePinned
    required property string postThreadgateUri
    required property int postReplyRestriction // QEnums::ReplyRestriction flags
    required property list<listviewbasic> postReplyRestrictionLists
    required property list<string> postHiddenReplies
    required property bool postIsHiddenReply
    required property bool postBookmarked
    required property bool postBookmarkTransient
    required property int postFeedback // QEnums::FeedbackType
    required property int postFeedbackTransient // QEnums::FeedbackType
    required property list<contentlabel> postLabels
    required property int postContentVisibility // QEnums::PostContentVisibility
    required property string postContentWarning
    required property basicprofile postContentLabeler
    required property int postMutedReason // QEnums::MutedPostReason
    required property string postHighlightColor
    required property bool postIsPinned
    required property bool postIsThread
    required property bool postIsThreadReply
    required property bool postLocallyDeleted
    required property int filteredPostHideReason // QEnums::HideReasonType
    required property string filteredPostHideDetail
    required property contentlabel filteredPostContentLabel
    required property bool endOfFeed
    property bool showRecord: true
    property bool unrollThread: false
    property var postThreadModel // provided when thread is unrolled
    property bool feedAcceptsInteractions: false
    property string feedDid: ""
    property bool swipeMode: false
    property int extraFooterHeight: 0
    property bool threadBarVisible: !swipeMode
    readonly property bool postBlockedByUser: postBlocked && postBlockedAuthor.viewer.valid && !postBlockedAuthor.viewer.blockedBy &&
            (postBlockedAuthor.viewer.blocking || !postBlockedAuthor.viewer.blockingByList.isNull())
    readonly property bool noPostRendering: postNotFound || postNotSupported || postLocallyDeleted || postBlocked || postBlockedByUser || postHiddenPosts || postFoldedType === QEnums.FOLDED_POST_FIRST
    readonly property int minGridHeight: noPostRendering ? 30 : 90
    readonly property int gridColumns: threadBarVisible ? 2 : 1
    readonly property int threadColumnWidth: threadBarVisible ? guiSettings.threadColumnWidth : 0
    readonly property int contentLeftMargin: threadBarVisible ? 0 : margin

    property int prevYCoord: 0
    property int lastDY: 0
    property int prevHeight: height
    property int prevAnchorY: 0
    property bool isAnchorItem: false
    property bool onScreen: false

    signal showHiddenReplies
    signal unfoldPosts
    signal activateSwipe(int imgIndex, var previewImg)
    signal addMorePosts(string uri)
    signal addOlderPosts

    id: postEntry
    // HACK
    // Setting the default size to 300 if the grid is not sized yet, seems to fix
    // positioning issued with viewPositionAtIndex
    height: postFoldedType === QEnums.FOLDED_POST_SUBSEQUENT ? 0 : (grid.height > minGridHeight ? grid.height : 300) + extraFooterHeight
    color: ((postThreadType & QEnums.THREAD_ENTRY) && !unrollThread) ? guiSettings.postHighLightColor : guiSettings.backgroundColor
    border.width: ((postThreadType & QEnums.THREAD_ENTRY) && !unrollThread) ? 1 : 0
    border.color: ((postThreadType & QEnums.THREAD_ENTRY) && !unrollThread) ? guiSettings.borderHighLightColor : guiSettings.borderColor
    visible: postFoldedType !== QEnums.FOLDED_POST_SUBSEQUENT

    Accessible.role: Accessible.Button
    Accessible.name: getSpeech()
    Accessible.onPressAction: performAccessiblePressAction()

    onIsAnchorItemChanged: prevAnchorY = y

    onYChanged: {
        if (prevYCoord != 0)
            lastDY = y - prevYCoord

        prevYCoord = y

        checkOnScreen()
    }

    // New items added at the top of the list sometimes push all items below downwards,
    // causing the list to scroll. To prevent that, we detect the downward movement and
    // scroll back (ideally Qt should not do push down)
    onHeightChanged: {
        const dh = height - prevHeight
        const relY = y - ListView.view.contentY
        const anchorIndex = ListView.view.getAnchorIndex()

        if (!ListView.view.flicking && index < anchorIndex && y !== 0 && dh != -lastDY) {
            ListView.view.contentY += dh
        }

        lastDY = 0
    }

    onOnScreenChanged: {
        if (onScreen) {
            if (feedAcceptsInteractions)
                postEntry.ListView.view.model.reportOnScreen(postUri)
        } else {
            cover()
        }
    }

    function cover() {
        if (feedAcceptsInteractions)
            postEntry.ListView.view.model.reportOffScreen(postUri, postFeedContext)

        postBody.movedOffScreen()
    }

    function closeMedia(mediaIndex, closeCb) {
        postBody.closeMedia(mediaIndex, closeCb)
    }

    GridLayout {
        id: grid
        columns: gridColumns
        width: parent.width
        rowSpacing: 0

        // BAR
        // Instead of using row spacing, these empty rectangles are used for white space.
        // This way we can color the background for threads.
        Rectangle {
            id: topLeftSpace
            Layout.leftMargin: 8 + (avatarImg.width - width) / 2
            Layout.preferredWidth: threadStyle === QEnums.THREAD_STYLE_BAR ? avatarImg.width : guiSettings.threadLineWidth
            Layout.preferredHeight: postEntry.margin * (!postParentInThread && (postType === QEnums.POST_REPLY || postType === QEnums.POST_LAST_REPLY) ?
                                        2 :
                                        ((postIsReply && (postThreadType & QEnums.THREAD_TOP) ? 4 : 1)))

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
                        return (postThreadType & QEnums.THREAD_TOP) ? "transparent" : getThreadEntryColor(threadColor)
                    } else if (postThreadType & QEnums.THREAD_TOP) {
                        return postIsReply ? guiSettings.threadStartColor(threadColor) : "transparent"
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
            visible: threadBarVisible

            Rectangle {
                y: postEntry.margin - (height / 2)
                width: parent.width
                height: 6
                color: guiSettings.threadMidColor(threadColor)
                visible: !postParentInThread && (postType === QEnums.POST_REPLY || postType === QEnums.POST_LAST_REPLY)
            }
        }
        Rectangle {
            Layout.leftMargin: contentLeftMargin
            Layout.preferredWidth: parent.width - threadColumnWidth - postEntry.margin * 2
            Layout.preferredHeight: topLeftSpace.visible ? topLeftSpace.height : postEntry.margin
            color: "transparent"

            Loader {
                width: parent.width
                height: parent.height
                active: postIsReply && (postThreadType & QEnums.THREAD_TOP)
                visible: status == Loader.Ready

                sourceComponent: AccessibleText {
                    width: parent.width
                    height: parent.height
                    elide: Text.ElideRight
                    color: guiSettings.linkColor
                    verticalAlignment: Text.AlignVCenter
                    text: qsTr("Read older...")

                    SkyMouseArea {
                        anchors.fill: parent
                        onClicked: addOlderPosts()
                    }
                }
            }
        }

        // BAR
        // Pinned post
        Loader {
            Layout.preferredWidth: threadColumnWidth
            Layout.fillHeight: true
            active: postIsPinned && !postLocallyDeleted && threadBarVisible
            visible: status == Loader.Ready
            sourceComponent: Rectangle {
                width: parent.width
                height: parent.height
                color: guiSettings.backgroundColor

                SkySvg {
                    anchors.right: parent.right
                    width: 18
                    height: width
                    color: Material.color(Material.Grey)
                    svg: SvgFilled.pin
                }
            }
        }
        Loader {
            Layout.leftMargin: contentLeftMargin
            Layout.fillWidth: true
            active: postIsPinned && !postLocallyDeleted
            visible: status == Loader.Ready
            sourceComponent: AccessibleText {
                width: parent.width
                elide: Text.ElideRight
                text: qsTr("Pinned post")
                color: Material.color(Material.Grey)
                font.bold: true
                font.pointSize: guiSettings.scaledFont(7/8)
            }
        }

        // BAR
        // Repost information
        Loader {
            Layout.preferredWidth: threadColumnWidth
            Layout.fillHeight: true
            active: !postRepostedByAuthor.isNull() && !postGapId && !postLocallyDeleted && threadBarVisible
            visible: status == Loader.Ready
            sourceComponent: Rectangle {
                width: parent.width
                height: parent.height
                color: guiSettings.backgroundColor

                SkySvg {
                    anchors.right: parent.right
                    width: 18
                    height: width
                    color: Material.color(Material.Grey)
                    svg: SvgOutline.repost
                }
            }
        }
        Loader {
            Layout.leftMargin: contentLeftMargin
            Layout.preferredWidth: parent.width - threadColumnWidth - postEntry.margin * 2
            active: !postRepostedByAuthor.isNull() && !postGapId && !postLocallyDeleted
            visible: status == Loader.Ready
            sourceComponent: SkyCleanedTextLine {
                id: repostedByText
                width: parent.width
                elide: Text.ElideRight
                plainText: qsTr(`Reposted by ${postRepostedByAuthor.name}`)
                color: Material.color(Material.Grey)
                font.bold: true
                font.pointSize: guiSettings.scaledFont(7/8)
                Accessible.ignored: true

                SkyMouseArea {
                    anchors.fill: parent
                    onClicked: skywalker.getDetailedProfile(postRepostedByAuthor.did)
                }
            }
        }

        // BAR
        // Author and content
        Rectangle {
            id: avatar
            Layout.preferredWidth: threadColumnWidth
            Layout.fillHeight: true
            color: "transparent"
            opacity: 0.9
            visible: threadBarVisible

            Rectangle {
                x: avatarImg.x + (avatarImg.width - width) / 2
                y: ((postType === QEnums.POST_ROOT && !postIsReply) || ((postThreadType & QEnums.THREAD_TOP) && !postIsReply)) ? avatarImg.y + avatarImg.height / 2 : 0
                width: threadStyle === QEnums.THREAD_STYLE_LINE ? guiSettings.threadLineWidth : avatarImg.width
                height: ((postType === QEnums.POST_LAST_REPLY) || (postThreadType & QEnums.THREAD_LEAF)) && postReplyCount === 0 && !unrollThread ? avatarImg.y + avatarImg.height / 2 - y : parent.height - y

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
                                    return getThreadEntryColor(threadColor)
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
                                return getThreadEndColor(threadColor)
                            case QEnums.POST_THREAD: {
                                if (postThreadType & QEnums.THREAD_ENTRY) {
                                    return getThreadEntryColor(threadColor)
                                } else if (postThreadType & QEnums.THREAD_PARENT) {
                                    return guiSettings.threadStartColor(threadColor)
                                } else if (postThreadType & QEnums.THREAD_LEAF) {
                                    return getThreadEndColor(threadColor)
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
                x: 8
                y: 5
                width: parent.width - 13
                userDid: postEntry.userDid
                author: postEntry.author
                showWarnedMedia: postEntry.filteredPostHideReason !== QEnums.HIDE_REASON_NONE
                visible: (!postIsPlaceHolder || postBlockedByUser) && !postLocallyDeleted && postFoldedType === QEnums.FOLDED_POST_NONE && (!unrollThread || postEntry.index == 0)

                onClicked: skywalker.getDetailedProfile(author.did)

                Accessible.role: Accessible.Button
                Accessible.name: qsTr(`show profile of ${author.name}`)
                Accessible.onPressAction: clicked()
            }

            // Mask to turn the threadbar into a dotted line
            Loader {
                width: parent.width
                active: postFoldedType === QEnums.FOLDED_POST_FIRST

                sourceComponent: Repeater {
                    model: Math.floor(avatar.height / 8)

                    Rectangle {
                        required property int index

                        y: 4 + index * 8
                        width: parent.width
                        height: 4
                        color: guiSettings.backgroundColor
                    }
                }
            }
        }
        Column {
            id: postColumn
            // Change from width to Layout.preferredWidth seems to solve the issue
            // where posts sometimes are too wide (like landscape mode) but makes
            // things very slow :-(
            // This seems fixed on Qt 6.7.3
            Layout.leftMargin: contentLeftMargin
            Layout.preferredWidth: parent.width - threadColumnWidth - postEntry.margin * 2
            visible: !postIsPlaceHolder && !postLocallyDeleted && postFoldedType === QEnums.FOLDED_POST_NONE

            Loader {
                width: parent.width
                active: threadBarVisible && (!unrollThread || postEntry.index == 0)
                visible: status == Loader.Ready

                sourceComponent: PostHeader {
                    width: parent.width
                    userDid: postEntry.userDid
                    author: postEntry.author
                    postIndexedSecondsAgo: postEntry.postIndexedSecondsAgo
                    filteredContentLabel: postEntry.filteredPostContentLabel
                }
            }

            Loader {
                width: parent.width
                active: !threadBarVisible && (!unrollThread || postEntry.index == 0)
                visible: status == Loader.Ready

                sourceComponent: PostHeaderWithAvatar {
                    width: parent.width
                    userDid: postEntry.userDid
                    author: postEntry.author
                    postIndexedSecondsAgo: postEntry.postIndexedSecondsAgo
                    filteredContentLabel: postEntry.filteredPostContentLabel
                }
            }

            // Reply to
            Loader {
                width: parent.width
                active: postIsReply && (!postParentInThread || postType === QEnums.POST_ROOT) && (postType !== QEnums.POST_THREAD || index === 0)
                visible: status == Loader.Ready
                sourceComponent: ReplyToRow {
                    width: parent.width
                    text: qsTr(`Reply to ${postReplyToAuthor.name}`)
                }
            }

            // Reply hidden by user
            Loader {
                width: parent.width
                active: postIsHiddenReply && postReplyRootAuthorDid === skywalker.getUserDid()
                visible: status == Loader.Ready
                sourceComponent: ReplyToRow {
                    width: parent.width
                    text: qsTr("Reply hidden by you")
                    svg: SvgOutline.hideVisibility
                }
            }

            // Hide reason (only when filtered posts are shown
            Loader {
                width: parent.width
                active: filteredPostHideReason !== QEnums.HIDE_REASON_NONE
                visible: status == Loader.Ready

                sourceComponent: Rectangle {
                    width: hideReasonRow.width
                    height: hideReasonRow.height
                    radius: 3
                    color: guiSettings.hideReasonLabelColor

                    Row {
                        id: hideReasonRow
                        width: parent.width
                        spacing: 10

                        SkySvg {
                            id: hideIcon
                            width: 20
                            height: 20
                            color: guiSettings.textColor
                            svg: SvgOutline.hideVisibility
                        }

                        AccessibleText {
                            width: parent.width - hideIcon.width
                            rightPadding: 10
                            anchors.verticalCenter: parent.verticalCenter
                            elide: Text.ElideRight
                            text: QEnums.hideReasonToString(filteredPostHideReason) + (filteredPostHideDetail ? `: ${filteredPostHideDetail}` : "")
                        }
                    }
                }
            }

            PostBody {
                id: postBody
                width: parent.width
                userDid: postEntry.userDid
                postAuthor: author
                postText: postEntry.postText
                postPlainText: postEntry.postPlainText
                postTranslatedText: postEntry.postTranslatedText
                postHasUnknownEmbed: postEntry.postHasUnknownEmbed
                postUnknownEmbedType: postEntry.postUnknownEmbedType
                postImages: postEntry.postImages
                postLanguageLabels: postLanguages
                postContentLabels: postLabels
                postContentVisibility: postEntry.postContentVisibility
                postContentWarning: postEntry.postContentWarning
                postContentLabeler: postEntry.postContentLabeler
                postMuted: postEntry.postMutedReason
                postIsThread: postEntry.postIsThread && !postEntry.unrollThread
                postIsThreadReply: postEntry.postIsThreadReply && !postEntry.unrollThread
                postVideo: postEntry.postVideo
                postExternal: postEntry.postExternal
                postRecord: postEntry.postRecord
                postRecordWithMedia: postEntry.postRecordWithMedia
                postDateTime: postEntry.postIndexedDateTime
                detailedView: ((postThreadType & QEnums.THREAD_ENTRY) && !postEntry.unrollThread) || (postEntry.unrollThread && postEntry.endOfFeed)
                initialShowMaxTextLines: postEntry.unrollThread ? maxTextLines : 25
                bodyBackgroundColor: postEntry.color.toString()
                borderColor: postEntry.border.color.toString()
                postHighlightColor: postEntry.postHighlightColor
                swipeMode: postEntry.swipeMode
                showRecord: postEntry.showRecord

                onActivateSwipe: (imgIndex, previewImg) => postEntry.activateSwipe(imgIndex, previewImg)
                onUnrollThread: {
                    if (!postEntry.unrollThread && !postEntry.postIsPlaceHolder && postEntry.postUri)
                        skywalker.getPostThread(postUri, QEnums.POST_THREAD_UNROLLED)
                }
            }

            // Reposts and likes in detailed view of post entry in thread view
            Loader {
                width: parent.width
                active: ((postThreadType & QEnums.THREAD_ENTRY) && !postEntry.unrollThread) || (postEntry.unrollThread && postEntry.endOfFeed)
                visible: status == Loader.Ready
                sourceComponent: Flow {
                    width: parent.width
                    topPadding: 10
                    bottomPadding: 5
                    spacing: 10

                    StatAuthors {
                        userDid: postEntry.userDid
                        atUri: postUri
                        count: postRepostCount
                        nameSingular: qsTr("repost")
                        namePlural: qsTr("reposts")
                        authorListType: QEnums.AUTHOR_LIST_REPOSTS
                        authorListHeader: qsTr("Reposted by")
                    }
                    StatQuotes {
                        userDid: postEntry.userDid
                        atUri: postUri
                        count: postQuoteCount
                    }
                    StatAuthors {
                        userDid: postEntry.userDid
                        atUri: postUri
                        count: postLikeCount
                        nameSingular: qsTr("like")
                        namePlural: qsTr("likes")
                        authorListType: QEnums.AUTHOR_LIST_LIKES
                        authorListHeader: qsTr("Liked by")
                    }
                }
            }

            // Stats
            Loader {
                width: parent.width
                height: guiSettings.postStatsHeight(feedAcceptsInteractions, 10)
                active: !unrollThread || endOfFeed
                asynchronous: true
                visible: active

                sourceComponent: PostStats {
                    id: postStats
                    topPadding: 10
                    skywalker: postEntry.skywalker
                    replyCount: postReplyCount
                    repostCount: postRepostCount + postQuoteCount
                    likeCount: postLikeCount
                    repostUri: postRepostUri
                    likeUri: postLikeUri
                    likeTransient: postLikeTransient
                    threadMuted: postThreadMuted
                    replyDisabled: postReplyDisabled
                    viewerStatePinned: postViewerStatePinned
                    replyRestriction: postReplyRestriction
                    isHiddenReply: postIsHiddenReply
                    isReply: postIsReply
                    replyRootAuthorDid: postReplyRootAuthorDid
                    replyRootUri: postReplyRootUri
                    authorIsUser: author.did === userDid
                    isBookmarked: postBookmarked
                    bookmarkTransient: postBookmarkTransient
                    feedback: postFeedback
                    feedbackTransient: postFeedbackTransient
                    isThread: postIsThread || postIsThreadReply
                    isQuotePost: Boolean(postRecord) || Boolean(postRecordWithMedia)
                    isUnrolledThread: postEntry.unrollThread
                    showViewThread: swipeMode
                    record: postRecord
                    recordWithMedia: postRecordWithMedia
                    feedAcceptsInteractions: postEntry.feedAcceptsInteractions

                    onReply: {
                        const lang = postLanguages.length > 0 ? postLanguages[0].shortCode : ""
                        root.composeReply(postUri, postCid,
                                          postEntry.unrollThread ? postThreadModel?.getFirstUnrolledPostText() : postText,
                                          postIndexedDateTime,
                                          author, postReplyRootUri, postReplyRootCid, lang,
                                          postMentionDids, "", "", feedDid, postFeedContext, userDid)
                    }

                    onReplyLongPress: (mouseEvent) => {
                        if (!root.isActiveUser(userDid))
                            return

                        const lang = postLanguages.length > 0 ? postLanguages[0].shortCode : ""
                        root.replyByNonActiveUser(
                                mouseEvent, postStats, postEntry.ListView.view,
                                postUri, postCid,
                                postEntry.unrollThread ? postThreadModel?.getFirstUnrolledPostText() : postText,
                                postIndexedDateTime,
                                author, postReplyRootUri, postReplyRootCid, lang,
                                postMentionDids)
                    }

                    onRepost: {
                        root.repost(postRepostUri, postUri, postCid,
                                    postReasonRepostUri, postReasonRepostCid,
                                    feedDid, postFeedContext,
                                    postEntry.unrollThread ? postThreadModel?.getFirstUnrolledPostText() : postText,
                                    postIndexedDateTime, author, postEmbeddingDisabled,
                                    postEntry.unrollThread ? postThreadModel?.getFirstUnrolledPostPlainText() : postPlainText,
                                    userDid)
                    }

                    function quote(quoteByDid = "") {
                        root.quotePost(postUri, postCid,
                            postEntry.unrollThread ? postThreadModel?.getFirstUnrolledPostText() : postText,
                            postIndexedDateTime, author, postEmbeddingDisabled,
                            feedDid, postFeedContext, quoteByDid)
                    }

                    onRepostLongPress: (mouseEvent) => {
                        if (!root.isActiveUser(userDid)) {
                            quote(userDid)
                            return
                        }

                        const actionDone = root.repostByNonActiveUser(
                                mouseEvent, postStats, postEntry.ListView.view, postUri, postCid,
                                postEntry.unrollThread ? postThreadModel?.getFirstUnrolledPostText() : postText,
                                postIndexedDateTime, author, postEmbeddingDisabled,
                                postReasonRepostUri, postReasonRepostCid)

                        if (!actionDone)
                            quote()
                    }

                    onLike: root.like(postLikeUri, postUri, postCid,
                                      postReasonRepostUri, postReasonRepostCid,
                                      feedDid, postFeedContext, userDid)

                    onLikeLongPress: (mouseEvent) => {
                        if (root.isActiveUser(userDid))
                            root.likeByNonActiveUser(mouseEvent, postStats, postEntry.ListView.view, postUri, postReasonRepostUri, postReasonRepostCid)
                    }

                    onBookmark: {
                        if (isBookmarked)
                            skywalker.getBookmarks().removeBookmark(postUri, postCid)
                        else
                            skywalker.getBookmarks().addBookmark(postUri, postCid)
                    }

                    onBookmarkLongPress: (mouseEvent) => {
                        if (root.isActiveUser(userDid))
                            root.bookmarkByNonActiveUser(mouseEvent, postStats, postEntry.ListView.view, postUri)
                    }

                    onViewThread: {
                        if (!postIsPlaceHolder && postUri)
                            skywalker.getPostThread(postUri)
                    }

                    onUnrollThread: {
                        if (!postIsPlaceHolder && postUri)
                            skywalker.getPostThread(postUri, QEnums.POST_THREAD_UNROLLED)
                    }

                    onQuoteChain: {
                        if (!postIsPlaceHolder && postUri)
                            root.viewQuoteChain(postUri, userDid)
                    }

                    onShare: skywalker.sharePost(postUri)
                    onMuteThread: root.muteThread(postIsReply ? postReplyRootUri : postUri, postThreadMuted, userDid)
                    onThreadgate: root.gateRestrictions(postThreadgateUri, postIsReply ? postReplyRootUri : postUri, postIsReply ? postReplyRootCid : postCid, postUri, postReplyRestriction, postReplyRestrictionLists, postHiddenReplies, userDid)
                    onHideReply: root.hidePostReply(postThreadgateUri, postReplyRootUri, postReplyRootCid, postUri, postReplyRestriction, postReplyRestrictionLists, postHiddenReplies, userDid)
                    onEditPost: root.composePostEdit(postEntry.ListView.view.model, postEntry.index)
                    onDeletePost: confirmDelete()
                    onCopyPostText: skywalker.copyPostTextToClipboard(postEntry.unrollThread ? postThreadModel?.getFullThreadPlainText() : postPlainText)
                    onReportPost: root.reportPost(postUri, postCid, postEntry.unrollThread ? postThreadModel?.getFirstUnrolledPostText() : postText, postIndexedDateTime, author, userDid)
                    onTranslatePost: root.translateText(postEntry.unrollThread ? postThreadModel?.getFullThreadPlainText() : postPlainText)
                    onInlineTranslatePost: model.translate(index)
                    onDetachQuote: (uri, detach) => root.detachQuote(uri, postUri, postCid, detach, userDid)
                    onPin: root.pinPost(postUri, postCid, userDid)
                    onUnpin: root.unpinPost(postCid, userDid)
                    onBlockAuthor: root.blockAuthor(author, userDid)
                    onShowEmojiNames: root.showEmojiNamesList(postEntry.unrollThread ? postThreadModel?.getFullThreadPlainText() : postPlainText)
                    onShowMoreLikeThis: root.showMoreLikeThis(feedDid, postUri, postCid, postFeedContext, userDid)
                    onShowLessLikeThis: root.showLessLikeThis(feedDid, postUri, postCid, postFeedContext, userDid)
                }
            }

            Loader {
                width: parent.width
                active: (postThreadType & QEnums.THREAD_LEAF) &&
                        !(postThreadType & QEnums.THREAD_ENTRY) &&
                        !unrollThread &&
                        postReplyCount > 0
                visible: status == Loader.Ready

                sourceComponent: AccessibleText {
                    topPadding: 10
                    bottomPadding: 10
                    width: parent.width
                    elide: Text.ElideRight
                    color: guiSettings.linkColor
                    text: qsTr("Read more...")

                    SkyMouseArea {
                        anchors.fill: parent
                        onClicked: addMorePosts(postUri)
                    }
                }
            }

            Loader {
                width: parent.width
                active: postType === QEnums.POST_THREAD &&
                        !(postThreadType & QEnums.THREAD_LEAF) &&
                        !(postThreadType & QEnums.THREAD_ENTRY) &&
                        !unrollThread &&
                        postReplyCount > 1
                visible: status == Loader.Ready

                sourceComponent: AccessibleText {
                    topPadding: 10
                    bottomPadding: 10
                    width: parent.width
                    elide: Text.ElideRight
                    color: guiSettings.linkColor
                    text: qsTr("Show more replies...")

                    SkyMouseArea {
                        anchors.fill: parent
                        onClicked: openPostThread()
                    }
                }
            }
        }

        // Folded Posts (Replaces the post content above)
        Loader {
            Layout.leftMargin: contentLeftMargin
            Layout.fillWidth: true
            active: postFoldedType === QEnums.FOLDED_POST_FIRST
            visible: status == Loader.Ready
            sourceComponent: AccessibleText {
                topPadding: 10
                bottomPadding: 10
                width: parent.width
                elide: Text.ElideRight
                color: guiSettings.linkColor
                text: qsTr("Unfold post thread")

                SkyMouseArea {
                    anchors.fill: parent
                    onClicked: unfoldPosts()
                }
            }
        }

        // Gap place holder
        Loader {
            Layout.columnSpan: gridColumns
            Layout.fillWidth: true
            active: postGapId > 0
            visible: status == Loader.Ready
            sourceComponent: AccessibleText {
                width: parent.width
                topPadding: 50
                bottomPadding: 50
                horizontalAlignment: Text.AlignHCenter
                elide: Text.ElideRight
                color: guiSettings.linkColor
                text: qsTr("Show more posts")

                SkyMouseArea {
                    anchors.fill: parent
                    onClicked: getGapPosts()
                }
            }
        }

        // Hidden posts
        Loader {
            Layout.columnSpan: gridColumns
            Layout.fillWidth: true
            active: postHiddenPosts
            visible: status == Loader.Ready
            sourceComponent: AccessibleText {
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                elide: Text.ElideRight
                color: guiSettings.linkColor
                text: qsTr("Show hidden replies")

                SkyMouseArea {
                    anchors.fill: parent
                    onClicked: showHiddenReplies()
                }
            }
        }

        // Place holder for NOT FOUND, NOT SUPPORTED, DELETED posts
        // NOTE: if a post is blocked locally, it also gets deleted locally
        Loader {
            Layout.columnSpan: gridColumns
            Layout.fillWidth: true
            active: (postNotFound || postNotSupported || postLocallyDeleted) && !postBlocked
            visible: status == Loader.Ready
            sourceComponent: AccessibleText {
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                elide: Text.ElideRight
                text: {
                    if (postNotFound)
                        return qsTr("🗑 Not found")
                    else if (postNotSupported)
                        return qsTr("⚠️ Not supported")
                    else if (postLocallyDeleted)
                        return qsTr("🗑 Deleted")
                    else
                        return "⚠️ Error"
                }
            }
        }

        // Place holder for BLOCKED posts where you (probably) did not block the author.
        // If there is no viewer state we do not know where the block comes from.
        Loader {
            Layout.columnSpan: gridColumns
            Layout.fillWidth: true
            active: postBlocked && !postBlockedByUser
            visible: status == Loader.Ready
            sourceComponent: AccessibleText {
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                elide: Text.ElideRight
                text: qsTr("🚫 Blocked")
            }
        }

        // Place holder for BLOCKED posts where you blocked the author.
        // Avatar is already displayed in the thread bar!
        Loader {
            Layout.fillWidth: true
            active: postBlockedByUser
            visible: status == Loader.Ready
            sourceComponent: Column {
                width: parent.width

                PostHeader {
                    width: parent.width
                    userDid: postEntry.userDid
                    author: postBlockedAuthor.author
                    postIndexedSecondsAgo: -1
                    visible: threadBarVisible
                }

                PostHeaderWithAvatar {
                    width: parent.width
                    userDid: postEntry.userDid
                    author: postBlockedAuthor.author
                    postIndexedSecondsAgo: -1
                    visible: !threadBarVisible
                }

                AccessibleText {
                    bottomPadding: 5
                    width: parent.width
                    elide: Text.ElideRight
                    text: {
                        if (postBlockedAuthor.blockingByListUri)
                            return qsTr("🚫 Blocked by list")
                        else if (postBlockedAuthor.viewer.blocking)
                            return qsTr("🚫 Blocked by you")
                        else
                            return qsTr("🚫 Blocked")
                    }
                }

                ListHeader {
                    width: parent.width
                    list: postBlockedAuthor.blockingByList
                    visible: !postBlockedAuthor.blockingByList.isNull()
                }
            }
        }

        Loader {
            Layout.columnSpan: gridColumns
            Layout.fillWidth: true
            active: postNotSupported
            visible: status == Loader.Ready
            sourceComponent: AccessibleText {
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.Wrap
                maximumLineCount: 2
                elide: Text.ElideRight
                color: Material.color(Material.Grey)
                font.pointSize: guiSettings.scaledFont(7/8)
                text: postUnsupportedType
            }
        }

        // BAR
        // Instead of using row spacing, these empty rectangles are used for white space.
        // This way we can color the background for threads.
        Rectangle {
            Layout.preferredWidth: threadColumnWidth
            Layout.preferredHeight: postEntry.margin
            color: "transparent"
            visible: threadBarVisible && !unrollThread

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
                            return getThreadEntryColor(threadColor)
                        }
                        if (postThreadType & QEnums.THREAD_LEAF) {
                            return getThreadEndColor(threadColor)
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
            Layout.leftMargin: contentLeftMargin
            Layout.preferredWidth: parent.width - threadColumnWidth - postEntry.margin * 2
            Layout.preferredHeight: postEntry.margin
            color: "transparent"
            visible: !unrollThread
        }

        // Post/Thread separator
        Rectangle {
            Layout.preferredWidth: parent.width
            Layout.columnSpan: gridColumns
            Layout.preferredHeight: 1
            color: postThreadType & QEnums.THREAD_ENTRY ? guiSettings.separatorHighLightColor : guiSettings.separatorColor
            visible: [QEnums.POST_STANDALONE, QEnums.POST_LAST_REPLY].includes(postType) ||
                ((postThreadType & QEnums.THREAD_LEAF) && !unrollThread)
        }

        // End of feed indication
        Loader {
            Layout.columnSpan: gridColumns
            Layout.fillWidth: true
            active: endOfFeed
            visible: status == Loader.Ready
            sourceComponent: AccessibleText {
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                topPadding: 10
                bottomPadding: 50
                elide: Text.ElideRight
                text: unrollThread ? qsTr("End of thread") : qsTr("End of feed")
                font.italic: true
            }
        }
    }

    SkyMouseArea {
        z: -2 // Let other mouse areas, e.g. images, get on top, -2 to allow records on top
        anchors.fill: parent
        enabled: !(postThreadType & QEnums.THREAD_ENTRY) && !unrollThread
        onClicked: {
            if (swipeMode)
                activateSwipe(0, null)
            else
                openPostThread()
        }
    }


    AccessibilityUtils {
        id: accessibilityUtils
    }

    function confirmDelete() {
        guiSettings.askYesNoQuestion(
                    postEntry,
                    qsTr("Do you really want to delete your post?"),
                    () => root.deletePost(postUri, postCid, userDid))
    }

    function openPostThread() {
        if (!(postThreadType & QEnums.THREAD_ENTRY) && !unrollThread)
        {
            if (!postIsPlaceHolder && postUri)
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

        return accessibilityUtils.getPostSpeech(postIndexedDateTime, author,
                unrollThread ? postThreadModel?.getFullThreadPlainText() : postPlainText,
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

    function getThreadEntryColor(color) {
        return guiSettings.threadEntryColor(color)
    }

    function getThreadEndColor(color) {
        return unrollThread ? guiSettings.threadMidColor(color) : guiSettings.threadEndColor(color)
    }

    function checkOnScreen() {
        const headerHeight = ListView.view.headerItem ? ListView.view.headerItem.height : 0
        const topY = ListView.view.contentY + headerHeight
        onScreen = (y + height > topY) && (y < ListView.view.contentY + ListView.view.height)
    }

    Component.onDestruction: {
        console.debug("Post destroyed:", index, "height:", height)

        if (feedAcceptsInteractions && onScreen)
            postEntry.ListView.view.model.reportOffScreen(postUri, postFeedContext)

        ListView.view.onContentMoved.disconnect(checkOnScreen)
    }

    Component.onCompleted: {
        console.debug("Post created:", index, "height:", height)

        ListView.view.enableOnScreenCheck = true
        checkOnScreen()

        ListView.view.onContentMoved.connect(checkOnScreen)
    }
}
