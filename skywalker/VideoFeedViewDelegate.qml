import QtQuick
import QtQuick.Layouts
import skywalker

Rectangle {
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
    required property string postVideoTranscodedSource
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
    property int footerHeight: 0

    property bool onScreen: ListView.isCurrentItem
    property bool showFullPostText: false

    id: videoPage
    width: root.width
    height: root.height
    color: guiSettings.fullScreenColor

    onOnScreenChanged: {
        if (!onScreen)
            cover()
        else
            video.play()
    }

    VideoView {
        id: video
        width: parent.width
        height: parent.height
        maxHeight: parent.height
        videoView: postVideo
        transcodedSource: postVideoTranscodedSource
        contentVisibility: postContentVisibility
        contentWarning: postContentWarning
        controlColor: "white"
        disabledColor: "darkslategrey"
        backgroundColor: videoPage.color
        isFullViewMode: true
        isVideoFeed: true
        autoPlay: false
        footerHeight: videoPage.footerHeight
        useIfNeededHeight: postColumn.height

        onVideoLoaded: {
            if (onScreen)
                video.play()
        }
    }

    Rectangle {
        width: parent.width
        anchors.top: postColumn.top
        anchors.bottom: postColumn.bottom
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#00000000" }
            GradientStop { position: 1.0; color: "#CF000000" }
        }
        visible: postColumn.visible
    }

    MouseArea {
        width: parent.width
        anchors.top: postColumn.top
        anchors.bottom: postColumn.bottom
        enabled: postColumn.visible
        onClicked: showFullPostText = !showFullPostText
    }

    Column {
        id: postColumn
        x: (parent.width - width) / 2
        anchors.bottom: video.bottom
        anchors.bottomMargin: video.playControlsHeight + videoPage.footerHeight
        width: video.playControlsWidth - 20
        visible: video.showPlayControls

        Rectangle {
            width: parent.width
            height: 5
            color: "transparent"
        }

        RowLayout {
            width: parent.width
            spacing: 10

            Avatar {
                id: avatar
                Layout.alignment: Qt.AlignVCenter
                Layout.preferredWidth: 34
                Layout.preferredHeight: 34
                author: videoPage.author

                onClicked: skywalker.getDetailedProfile(author.did)
            }

            PostHeader {
                Layout.fillWidth: true
                author: videoPage.author
                postIndexedSecondsAgo: videoPage.postIndexedSecondsAgo
            }
        }

        PostBody {
            width: parent.width
            postAuthor: videoPage.author
            postText: videoPage.postText
            postImages: []
            postLanguageLabels: videoPage.postLanguages
            postContentLabels: videoPage.postLabels
            postContentVisibility: videoPage.postContentVisibility
            postContentWarning: videoPage.postContentWarning
            postMuted: videoPage.postMutedReason
            postDateTime: videoPage.postIndexedDateTime
            maxTextLines: videoPage.showFullPostText ? 1000 : 2
            bodyBackgroundColor: "transparent"
        }

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

        GuiSettings {
            id: guiSettings
            isLightMode: false
            backgroundColor: videoPage.color
            textColor: "white"
        }
    }

    function cover() {
        video.pause()
    }

    function checkOnScreen() {}

    Component.onCompleted: {
        ListView.view.enableOnScreenCheck = true
    }
}
