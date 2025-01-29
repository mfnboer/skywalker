import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
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
    property int headerHeight: 0
    property int footerHeight: 0
    property int leftMarginWidth: 0
    property int extraFooterHeight

    property bool onScreen: ListView.isCurrentItem
    property bool showFullPostText: false
    property var videoItem: postVideo ? videoLoader.item : null
    property var imageItem: postImages.length > 0 ? imageLoader.item : null

    signal closed

    id: videoPage
    width: root.width
    height: root.height + (endOfFeed ? root.height : 0) + extraFooterHeight
    color: guiSettings.fullScreenColor

    onOnScreenChanged: {
        if (!onScreen) {
            cover()
        }
        else {
            if (postVideo)
                videoItem.play()
        }
    }

    Rectangle {
        property int bottomMargin: postVideo ? videoItem.playControlsHeight : 0
        property int mediaWidth: postVideo ? videoItem.playControlsWidth : root.width
        property bool showDetails: postVideo ? videoItem.showPlayControls : imageLoader.showDetails

        id: mediaRect
        width: root.width
        height: root.height
        color: "transparent"

        Loader {
            id: videoLoader
            active: Boolean(postVideo)

            sourceComponent: VideoView {
                id: video
                width: root.width
                height: root.height
                maxHeight: root.height
                videoView: postVideo
                contentVisibility: postContentVisibility
                contentWarning: postContentWarning
                controlColor: "white"
                disabledColor: "darkslategrey"
                backgroundColor: videoPage.color
                isFullViewMode: true
                swipeMode: true
                autoPlay: false
                footerHeight: videoPage.footerHeight
                useIfNeededHeight: postColumn.height

                onVideoLoaded: {
                    if (onScreen)
                        videoItem.play()
                }
            }
        }

        Loader {
            property bool showDetails: true

            id: imageLoader
            active: postImages.length > 0

            sourceComponent: Rectangle {
                width: root.width
                height: root.height
                color: "transparent"

                ImageAutoRetry {
                    width: parent.width
                    height: parent.height
                    fillMode: Image.PreserveAspectFit
                    source: filter.getImage(0).fullSizeUrl
                    reloadIconColor: "white"
                }

                MouseArea {
                    width: parent.width
                    height: parent.height

                    onClicked: showDetails = !showDetails
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

    SvgButton {
        x: leftMarginWidth + 10
        y: headerHeight + 20
        iconColor: "white"
        Material.background: "transparent"
        svg: SvgOutline.arrowBack
        accessibleName: qsTr("go back")
        onClicked: videoPage.closed()
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
        anchors.bottom: mediaRect.bottom
        anchors.bottomMargin: mediaRect.bottomMargin + videoPage.footerHeight
        width: mediaRect.mediaWidth - 20
        visible: mediaRect.showDetails

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

        // Reply to
        Loader {
            width: parent.width
            active: postIsReply
            visible: status == Loader.Ready
            sourceComponent: ReplyToRow {
                width: parent.width
                text: qsTr(`Reply to ${postReplyToAuthor.name}`)
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

    GuiSettings {
        id: guiSettings
        isLightMode: false
        backgroundColor: videoPage.color
        linkColor: "#58a6ff"
        textColor: "white"
    }

    Loader {
        anchors.top: mediaRect.bottom
        active: endOfFeed

        sourceComponent: Rectangle {
            width: root.width
            height: root.height
            color: "transparent"

            SvgButton {
                x: leftMarginWidth + 10
                y: headerHeight + 20
                iconColor: "white"
                Material.background: "transparent"
                svg: SvgOutline.arrowBack
                accessibleName: qsTr("go back")
                onClicked: videoPage.closed()
            }

            Image {
                anchors.centerIn: parent
                width: parent.width
                fillMode: Image.PreserveAspectFit
                source: "/images/thats_all_folks.png"
            }
        }
    }

    function cover() {
        if (postVideo)
            videoItem.pause()
    }

    function checkOnScreen() {}

    Component.onCompleted: {
        ListView.view.enableOnScreenCheck = true
    }
}
