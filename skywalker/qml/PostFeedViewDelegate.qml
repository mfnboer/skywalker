﻿import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker

Rectangle {
    readonly property int margin: 10
    readonly property int threadStyle: root.getSkywalker().getUserSettings().threadStyle
    readonly property string threadColor: root.getSkywalker().getUserSettings().threadColor

    required property int index
    required property basicprofile author
    required property string postUri
    required property string postCid
    required property string postText
    required property string postPlainText
    required property list<language> postLanguages
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
    required property list<contentlabel> postLabels
    required property int postContentVisibility // QEnums::PostContentVisibility
    required property string postContentWarning
    required property int postMutedReason // QEnums::MutedPostReason
    required property string postHighlightColor
    required property bool postIsPinned
    required property bool postIsThread
    required property bool postIsThreadReply
    required property bool postLocallyDeleted
    required property bool endOfFeed
    property bool unrollThread: false
    property var postThreadModel // provided when thread is unrolled
    property bool feedAcceptsInteractions: false
    property string feedDid: ""
    property bool swipeMode: false
    property int extraFooterHeight: 0
    property bool threadBarVisible: !swipeMode
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
    signal activateSwipe
    signal addMorePosts(string uri)
    signal addOlderPosts

    id: postEntry
    // HACK
    // Setting the default size to 300 if the grid is not sized yet, seems to fix
    // positioning issued with viewPositionAtIndex
    height: postFoldedType === QEnums.FOLDED_POST_SUBSEQUENT ? 0 : (grid.height > 30 ? grid.height : 300) + extraFooterHeight
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
        if (!onScreen)
            cover()
    }

    function cover() {
        postBody.movedOffScreen()
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

                    MouseArea {
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

                MouseArea {
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
                author: postEntry.author
                visible: !postIsPlaceHolder && !postLocallyDeleted && postFoldedType === QEnums.FOLDED_POST_NONE && (!unrollThread || postEntry.index == 0)

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
                    author: postEntry.author
                    postIndexedSecondsAgo: postEntry.postIndexedSecondsAgo
                }
            }
            Loader {
                width: parent.width
                active: !threadBarVisible && (!unrollThread || postEntry.index == 0)
                visible: status == Loader.Ready

                sourceComponent: PostHeaderWithAvatar {
                    width: parent.width
                    author: postEntry.author
                    postIndexedSecondsAgo: postEntry.postIndexedSecondsAgo
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
                active: postIsHiddenReply && guiSettings.isUserDid(postReplyRootAuthorDid)
                visible: status == Loader.Ready
                sourceComponent: ReplyToRow {
                    width: parent.width
                    text: qsTr("Reply hidden by you")
                    svg: SvgOutline.hideVisibility
                }
            }

            PostBody {
                id: postBody
                width: parent.width
                postAuthor: author
                postText: postEntry.postText
                postPlainText: postEntry.postPlainText
                postHasUnknownEmbed: postEntry.postHasUnknownEmbed
                postUnknownEmbedType: postEntry.postUnknownEmbedType
                postImages: postEntry.postImages
                postLanguageLabels: postLanguages
                postContentLabels: postLabels
                postContentVisibility: postEntry.postContentVisibility
                postContentWarning: postEntry.postContentWarning
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

                onActivateSwipe: postEntry.activateSwipe()
                onUnrollThread: {
                    console.debug("UNROLL!!!")
                    if (!postEntry.unrollThread && !postEntry.postIsPlaceHolder && postEntry.postUri)
                        skywalker.getPostThread(postUri, true)
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
                        atUri: postUri
                        count: postRepostCount
                        nameSingular: qsTr("repost")
                        namePlural: qsTr("reposts")
                        authorListType: QEnums.AUTHOR_LIST_REPOSTS
                        authorListHeader: qsTr("Reposted by")
                    }
                    StatQuotes {
                        atUri: postUri
                        count: postQuoteCount
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
            }

            // Stats
            Loader {
                active: !unrollThread || endOfFeed
                width: parent.width
                height: guiSettings.statsHeight + 10
                asynchronous: true
                visible: active

                sourceComponent: PostStats {
                    width: parent.width
                    topPadding: 10
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
                    authorIsUser: guiSettings.isUser(author)
                    isBookmarked: postBookmarked
                    bookmarkTransient: postBookmarkTransient
                    isThread: postIsThread || postIsThreadReply
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
                                          postMentionDids)
                    }

                    onRepost: {
                        root.repost(postRepostUri, postUri, postCid, postReasonRepostUri, postReasonRepostCid,
                                    postEntry.unrollThread ? postThreadModel?.getFirstUnrolledPostText() : postText,
                                    postIndexedDateTime, author, postEmbeddingDisabled,
                                    postEntry.unrollThread ? postThreadModel?.getFirstUnrolledPostPlainText() : postPlainText)
                    }

                    onQuotePost: {
                        root.quotePost(postUri, postCid,
                                       postEntry.unrollThread ? postThreadModel?.getFirstUnrolledPostText() : postText,
                                       postIndexedDateTime, author, postEmbeddingDisabled)
                    }

                    onLike: root.like(postLikeUri, postUri, postCid, postReasonRepostUri, postReasonRepostCid)

                    onBookmark: {
                        if (isBookmarked)
                            skywalker.getBookmarks().removeBookmark(postUri, postCid)
                        else
                            skywalker.getBookmarks().addBookmark(postUri, postCid)
                    }

                    onViewThread: {
                        if (!postIsPlaceHolder && postUri)
                            skywalker.getPostThread(postUri)
                    }

                    onUnrollThread: {
                        if (!postIsPlaceHolder && postUri)
                            skywalker.getPostThread(postUri, true)
                    }

                    onShare: skywalker.sharePost(postUri)
                    onMuteThread: root.muteThread(postIsReply ? postReplyRootUri : postUri, postThreadMuted)
                    onThreadgate: root.gateRestrictions(postThreadgateUri, postIsReply ? postReplyRootUri : postUri, postIsReply ? postReplyRootCid : postCid, postUri, postReplyRestriction, postReplyRestrictionLists, postHiddenReplies)
                    onHideReply: root.hidePostReply(postThreadgateUri, postReplyRootUri, postReplyRootCid, postUri, postReplyRestriction, postReplyRestrictionLists, postHiddenReplies)
                    onDeletePost: confirmDelete()
                    onCopyPostText: skywalker.copyPostTextToClipboard(postEntry.unrollThread ? postThreadModel?.getFullThreadPlainText() : postPlainText)
                    onReportPost: root.reportPost(postUri, postCid, postEntry.unrollThread ? postThreadModel?.getFirstUnrolledPostText() : postText, postIndexedDateTime, author)
                    onTranslatePost: root.translateText(postEntry.unrollThread ? postThreadModel?.getFullThreadPlainText() : postPlainText)
                    onDetachQuote: (uri, detach) => root.detachQuote(uri, postUri, postCid, detach)
                    onPin: root.pinPost(postUri, postCid)
                    onUnpin: root.unpinPost(postCid)
                    onBlockAuthor: root.blockAuthor(author)
                    onShowEmojiNames: root.showEmojiNamesList(postEntry.unrollThread ? postThreadModel?.getFullThreadPlainText() : postPlainText)
                    onShowMoreLikeThis: root.showMoreLikeThis(feedDid, postUri, postFeedContext)
                    onShowLessLikeThis: root.showLessLikeThis(feedDid, postUri, postFeedContext)
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

                    MouseArea {
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

                    MouseArea {
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
            sourceComponent: Text {
                topPadding: 10
                bottomPadding: 10
                width: parent.width
                elide: Text.ElideRight
                color: guiSettings.linkColor
                text: qsTr("Unfold post thread")

                MouseArea {
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
            sourceComponent: Text {
                width: parent.width
                topPadding: 50
                bottomPadding: 50
                horizontalAlignment: Text.AlignHCenter
                elide: Text.ElideRight
                color: guiSettings.linkColor
                text: qsTr("Show more posts")

                MouseArea {
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
            sourceComponent: Text {
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                elide: Text.ElideRight
                color: guiSettings.linkColor
                text: qsTr("Show hidden replies")

                MouseArea {
                    anchors.fill: parent
                    onClicked: showHiddenReplies()
                }
            }
        }

        // Place holder for NOT FOUND, BLOCKED, NOT SUPPORTED, DELETED posts
        Loader {
            Layout.columnSpan: gridColumns
            Layout.fillWidth: true
            active: postNotFound || postBlocked || postNotSupported || postLocallyDeleted
            visible: status == Loader.Ready
            sourceComponent: Text {
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                elide: Text.ElideRight
                color: guiSettings.textColor
                text: {
                    if (postNotFound)
                        return qsTr("🗑 Not found")
                    else if (postBlocked)
                        return qsTr("🚫 Blocked")
                    else if (postNotSupported)
                        return qsTr("⚠️ Not supported")
                    else if (postLocallyDeleted)
                        return qsTr("🗑 Deleted")
                    else
                        return "⚠️ Error"
                }
            }
        }

        Loader {
            Layout.columnSpan: gridColumns
            Layout.fillWidth: true
            active: postNotSupported
            visible: status == Loader.Ready
            sourceComponent: Text {
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
            sourceComponent: Text {
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                topPadding: 10
                bottomPadding: 50
                elide: Text.ElideRight
                color: guiSettings.textColor
                text: unrollThread ? qsTr("End of thread") : qsTr("End of feed")
                font.italic: true
            }
        }
    }

    MouseArea {
        z: -2 // Let other mouse areas, e.g. images, get on top, -2 to allow records on top
        anchors.fill: parent
        enabled: !(postThreadType & QEnums.THREAD_ENTRY) && !unrollThread
        onClicked: {
            if (swipeMode)
                activateSwipe()
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
                    () => root.deletePost(postUri, postCid))
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

    Component.onCompleted: {
        ListView.view.enableOnScreenCheck = true
        checkOnScreen()
    }
}
