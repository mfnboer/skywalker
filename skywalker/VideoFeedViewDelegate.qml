import QtQuick
import skywalker

Rectangle {
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
    required property string postVideoSource
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

    property bool onScreen: ListView.isCurrentItem

    id: videoPage
    width: root.width
    height: root.height
    color: "black"

    onOnScreenChanged: {
        if (!onScreen)
            cover()
        else
            video.play()
    }

    VideoView {
        id: video
        y: (parent.height - height) / 2
        width: parent.width
        maxHeight: parent.height
        videoView: postVideo
        videoSource: postVideoSource
        transcodedSource: postVideoTranscodedSource
        contentVisibility: postContentVisibility
        contentWarning: postContentWarning
        controlColor: "white"
        disabledColor: "darkslategrey"
        backgroundColor: videoPage.color
        isFullViewMode: true
        isVideoFeed: true
        autoPlay: false

        onVideoLoaded: {
            if (onScreen)
                video.play()
        }
    }

    function cover() {
        video.pause()
    }
}
