import QtQuick
import QtQuick.Controls.Material
import skywalker

Rectangle {
    required property int index
    required property string userDid
    property Skywalker skywalker: root.getSkywalker(userDid)
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
    required property int postFeedback // QEnums::FeedbackType
    required property int postFeedbackTransient // QEnums::FeedbackType
    required property list<contentlabel> postLabels
    required property int postContentVisibility // QEnums::PostContentVisibility
    required property string postContentWarning
    required property int postMutedReason // QEnums::MutedPostReason
    required property string postHighlightColor
    required property bool postIsThread
    required property bool postIsThreadReply
    required property bool postIsPinned
    required property bool postLocallyDeleted
    required property bool endOfFeed
    property bool feedAcceptsInteractions: false
    property string feedDid: ""

    readonly property bool isLeftCell: index % GridView.view.columns == 0
    readonly property bool isRightCell: index % GridView.view.columns == GridView.view.columns - 1
    readonly property int leftMargin: isLeftCell ? 0 : (isRightCell ? GridView.view.spacing : GridView.view.spacing / GridView.view.columns)
    readonly property int rightMargin: isRightCell ? 0 : (isLeftCell ? GridView.view.spacing : GridView.view.spacing / GridView.view.columns)
    readonly property int bottomMargin: GridView.view.spacing

    property var videoItem: postVideo ? videoLoader.item : null
    property var imageItem: postImages.length > 0 ? imageLoader.item : null
    property var mediaItem: videoItem ? videoItem : imageItem
    property bool onScreen: false

    signal activateSwipe

    id: page
    color: guiSettings.backgroundColor

    onYChanged: checkOnScreen()

    onOnScreenChanged: {
        if (feedAcceptsInteractions)
        {
            if (onScreen)
                GridView.view.model.reportOnScreen(postUri)
            else
                GridView.view.model.reportOffScreen(postUri, postFeedContext)
        }
    }

    Rectangle {
        id: mediaRect
        x: leftMargin
        width: parent.width - leftMargin - rightMargin
        height: parent.height - bottomMargin
        color: "transparent"

        Loader {
            id: videoLoader
            active: Boolean(postVideo)

            sourceComponent: VideoView {
                id: video
                width: mediaRect.width
                height: mediaRect.height
                maxHeight: mediaRect.height
                videoView: postVideo
                contentVisibility: postContentVisibility
                contentWarning: postContentWarning
                autoLoad: false // avoid loading tens of videos
                autoPlay: false
                swipeMode: true
                tileMode: true

                onActivateSwipe: page.activateSwipe()
            }
        }

        Loader {
            id: imageLoader
            active: postImages.length > 0

            sourceComponent: ImageAutoRetry {
                property alias contentFilter: filter

                width: mediaRect.width
                height: mediaRect.height
                fillMode: Image.PreserveAspectCrop
                source: filter.getImage(0).thumbUrl
                sourceSize.width: width * Screen.devicePixelRatio
                sourceSize.height: height * Screen.devicePixelRatio
                smooth: false

                Loader {
                    anchors.right: parent.right
                    anchors.rightMargin: 5
                    anchors.top: parent.top
                    anchors.topMargin: 5
                    active: postImages.length > 1

                    sourceComponent: SkySvg {
                        width: 20
                        height: 20
                        svg: SvgOutline.imageCollection
                        color: "white"
                    }
                }

                FilteredImageWarning {
                    id: filter
                    x: 10
                    width: parent.width - 20
                    anchors.verticalCenter: parent.verticalCenter
                    contentVisibility: postContentVisibility
                    contentWarning: postContentWarning
                    images: postImages
                }
            }
        }
    }

    Rectangle {
        x: mediaRect.x
        width: mediaRect.width
        anchors.top: postColumn.top
        anchors.bottom: postColumn.bottom
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#00000000" }
            GradientStop { position: 1.0; color: "#BF000000" }
        }
        visible: postColumn.visible
    }

    Column {
        id: postColumn
        x: 10
        anchors.bottom: mediaRect.bottom
        width: mediaRect.width - 20
        visible: mediaItem ? mediaItem.contentFilter.imageVisible() : true

        Loader {
            width: parent.width
            height: guiSettings.postStatsHeight(feedAcceptsInteractions, 10, true)
            active: true
            asynchronous: true

            sourceComponent: PostStats {
                id: postStats
                topPadding: 10
                skywalker: page.skywalker
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
                showViewThread: true
                record: postRecord
                recordWithMedia: postRecordWithMedia
                feedAcceptsInteractions: page.feedAcceptsInteractions
                limitedStats: true
                color: "white"

                onReply: {
                    const lang = postLanguages.length > 0 ? postLanguages[0].shortCode : ""
                    root.composeReply(postUri, postCid, postText, postIndexedDateTime,
                                      author, postReplyRootUri, postReplyRootCid, lang,
                                      postMentionDids, "", "", feedDid, postFeedContext, userDid)
                }

                onReplyLongPress: (mouseEvent) => {
                    if (!root.isActiveUser(userDid))
                        return

                    const lang = postLanguages.length > 0 ? postLanguages[0].shortCode : ""
                    root.replyByNonActiveUser(
                            mouseEvent, postStats, page.GridView.view,
                            postUri, postCid, postText, postIndexedDateTime,
                            author, postReplyRootUri, postReplyRootCid, lang,
                            postMentionDids)
                }

                onRepost: {
                    root.repost(postRepostUri, postUri, postCid,
                                postReasonRepostUri, postReasonRepostCid,
                                feedDid, postFeedContext, postText,
                                postIndexedDateTime, author, postEmbeddingDisabled,
                                postPlainText, userDid)
                }

                function quote(quoteByDid = "") {
                    root.quotePost(postUri, postCid,
                            postText, postIndexedDateTime, author, postEmbeddingDisabled,
                            feedDid, postFeedContext, quoteByDid)
                }

                onRepostLongPress: (mouseEvent) => {
                    if (!root.isActiveUser(userDid)) {
                        quote(userDid)
                        return
                    }

                    const actionDone = root.repostByNonActiveUser(
                            mouseEvent, postStats, page.GridView.view, postUri, postCid,
                            postText, postIndexedDateTime, author, postEmbeddingDisabled,
                            postReasonRepostUri, postReasonRepostCid)

                    if (!actionDone)
                        quote()
                }

                onLike: root.like(postLikeUri, postUri, postCid,
                                  postReasonRepostUri, postReasonRepostCid,
                                  feedDid, postFeedContext, userDid)

                onLikeLongPress: (mouseEvent) => {
                    if (root.isActiveUser(userDid))
                        root.likeByNonActiveUser(mouseEvent, postStats, page.GridView.view, postUri, postReasonRepostUri, postReasonRepostCid)
                }

                onBookmark: {
                    if (isBookmarked)
                        skywalker.getBookmarks().removeBookmark(postUri, postCid)
                    else
                        skywalker.getBookmarks().addBookmark(postUri, postCid)
                }

                onBookmarkLongPress: (mouseEvent) => {
                    if (root.isActiveUser(userDid))
                        root.bookmarkByNonActiveUser(mouseEvent, postStats, page.GridView.view, postUri)
                }

                onShare: skywalker.sharePost(postUri)

                onViewThread: {
                    if (!postIsPlaceHolder && postUri)
                        skywalker.getPostThread(postUri)
                }

                onUnrollThread: {
                    if (!postIsPlaceHolder && postUri)
                        skywalker.getPostThread(postUri, QEnums.POST_THREAD_UNROLLED)
                }

                onMuteThread: root.muteThread(postIsReply ? postReplyRootUri : postUri, postThreadMuted, userDid)
                onThreadgate: root.gateRestrictions(postThreadgateUri, postIsReply ? postReplyRootUri : postUri, postIsReply ? postReplyRootCid : postCid, postUri, postReplyRestriction, postReplyRestrictionLists, postHiddenReplies, userDid)
                onHideReply: root.hidePostReply(postThreadgateUri, postReplyRootUri, postReplyRootCid, postUri, postReplyRestriction, postReplyRestrictionLists, postHiddenReplies, userDid)
                onEditPost: root.composePostEdit(page.GridView.view.model, page.index)
                onDeletePost: confirmDelete()
                onCopyPostText: skywalker.copyPostTextToClipboard(postPlainText)
                onReportPost: root.reportPost(postUri, postCid, postText, postIndexedDateTime, author, userDid)
                onTranslatePost: root.translateText(postPlainText)
                onDetachQuote: (uri, detach) => root.detachQuote(uri, postUri, postCid, detach, userDid)
                onPin: root.pinPost(postUri, postCid, userDid)
                onUnpin: root.unpinPost(postCid, userDid)
                onBlockAuthor: root.blockAuthor(author, userDid)
                onShowMoreLikeThis: root.showMoreLikeThis(feedDid, postUri, postCid, postFeedContext, userDid)
                onShowLessLikeThis: root.showLessLikeThis(feedDid, postUri, postCid, postFeedContext, userDid)
            }
        }

        Rectangle {
            width: parent.width
            height: 5
            color: "transparent"
        }
    }

    MouseArea {
        z: -2
        anchors.fill: parent
        enabled: !(postThreadType & QEnums.THREAD_ENTRY)
        onClicked: activateSwipe()
    }

    function checkOnScreen() {
        const headerHeight = GridView.view.headerItem ? GridView.view.headerItem.height : 0
        const topY = GridView.view.contentY + headerHeight
        onScreen = (y + height > topY) && (y < GridView.view.contentY + GridView.view.height)
    }

    function cover() {
        if (feedAcceptsInteractions && onScreen)
            GridView.view.model.reportOffScreen(postUri, postFeedContext)
    }

    function confirmDelete() {
        guiSettings.askYesNoQuestion(
                    page,
                    qsTr("Do you really want to delete your post?"),
                    () => root.deletePost(postUri, postCid, userDid))
    }

    Component.onDestruction: {
        if (feedAcceptsInteractions && onScreen)
            GridView.view.model.reportOffScreen(postUri, postFeedContext)

        GridView.view.onContentMoved.disconnect(checkOnScreen)
    }

    Component.onCompleted: {
        checkOnScreen()
        GridView.view.onContentMoved.connect(checkOnScreen)
    }
}
