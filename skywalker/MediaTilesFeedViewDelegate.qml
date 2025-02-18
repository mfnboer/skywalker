import QtQuick
import QtQuick.Controls.Material
import skywalker

Rectangle {
    required property int index
    property var skywalker: root.getSkywalker()
    required property basicprofile author
    required property string postUri
    required property string postCid
    required property string postText
    required property string postPlainText
    required property list<language> postLanguages
    required property date postIndexedDateTime
    required property double postIndexedSecondsAgo
    required property basicprofile postRepostedByAuthor
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
    required property bool postBookmarkNotFound
    required property list<contentlabel> postLabels
    required property int postContentVisibility // QEnums::PostContentVisibility
    required property string postContentWarning
    required property int postMutedReason // QEnums::MutedPostReason
    required property string postHighlightColor
    required property bool postIsPinned
    required property bool postLocallyDeleted
    required property bool endOfFeed

    readonly property bool isLeftCell: index % GridView.view.columns == 0
    readonly property bool isRightCell: index % GridView.view.columns == GridView.view.columns - 1
    readonly property int leftMargin: isLeftCell ? 0 : (isRightCell ? GridView.view.spacing : GridView.view.spacing / GridView.view.columns)
    readonly property int rightMargin: isRightCell ? 0 : (isLeftCell ? GridView.view.spacing : GridView.view.spacing / GridView.view.columns)
    readonly property int bottomMargin: GridView.view.spacing

    property var videoItem: postVideo ? videoLoader.item : null
    property var imageItem: postImages.length > 0 ? imageLoader.item : null
    property var mediaItem: videoItem ? videoItem : imageItem

    signal activateSwipe

    id: page
    color: guiSettings.backgroundColor

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
            active: true
            width: parent.width
            height: guiSettings.statsHeight + 10
            asynchronous: true

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
                embeddingDisabled: postEmbeddingDisabled
                viewerStatePinned: postViewerStatePinned
                replyRestriction: postReplyRestriction
                isHiddenReply: postIsHiddenReply
                isReply: postIsReply
                replyRootAuthorDid: postReplyRootAuthorDid
                replyRootUri: postReplyRootUri
                authorIsUser: guiSettings.isUser(author)
                isBookmarked: postBookmarked
                bookmarkNotFound: postBookmarkNotFound
                showViewThread: true
                record: postRecord
                recordWithMedia: postRecordWithMedia
                limitedStats: true
                color: "white"

                onReply: {
                    const lang = postLanguages.length > 0 ? postLanguages[0].shortCode : ""
                    root.composeReply(postUri, postCid, postText, postIndexedDateTime,
                                      author, postReplyRootUri, postReplyRootCid, lang)
                }

                onRepost: {
                    root.repost(postRepostUri, postUri, postCid, postText,
                                postIndexedDateTime, author, postEmbeddingDisabled, postPlainText)
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

                onViewThread: {
                    if (postUri)
                        skywalker.getPostThread(postUri)
                }

                onMuteThread: root.muteThread(postIsReply ? postReplyRootUri : postUri, postThreadMuted)
                onThreadgate: root.gateRestrictions(postThreadgateUri, postIsReply ? postReplyRootUri : postUri, postIsReply ? postReplyRootCid : postCid, postUri, postReplyRestriction, postReplyRestrictionLists, postHiddenReplies)
                onHideReply: root.hidePostReply(postThreadgateUri, postReplyRootUri, postReplyRootCid, postUri, postReplyRestriction, postReplyRestrictionLists, postHiddenReplies)
                onDeletePost: confirmDelete()
                onCopyPostText: skywalker.copyPostTextToClipboard(postPlainText)
                onReportPost: root.reportPost(postUri, postCid, postText, postIndexedDateTime, author)
                onTranslatePost: root.translateText(postPlainText)
                onDetachQuote: (uri, detach) => root.detachQuote(uri, postUri, postCid, detach)
                onPin: root.pinPost(postUri, postCid)
                onUnpin: root.unpinPost(postCid)
                onBlockAuthor: root.blockAuthor(author.did)
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
        enabled: !(postThreadType & QEnums.THREAD_ENTRY) && !postBookmarkNotFound
        onClicked: activateSwipe()
    }
}
