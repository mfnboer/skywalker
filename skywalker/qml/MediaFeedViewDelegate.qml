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
    required property basicprofile postContentLabeler
    required property int postMutedReason // QEnums::MutedPostReason
    required property string postHighlightColor
    required property bool postIsThread
    required property bool postIsThreadReply
    required property bool postIsPinned
    required property bool postLocallyDeleted
    required property bool endOfFeed
    property int startImageIndex: -1
    property int startImageWidth: -1
    property var postOrRecordVideo: postVideo ? postVideo : postRecordWithMedia?.video
    property list<imageview> postOrRecordImages: postImages.length > 0 ? postImages : (postRecordWithMedia ? postRecordWithMedia.images : [])
    property bool feedAcceptsInteractions: false
    property string feedDid: ""
    property int headerHeight: 0
    property int footerHeight: 0
    property int leftMarginWidth: 0
    property int rightMarginWidth: 0
    property int extraFooterHeight: 0

    property bool onScreen: ListView.isCurrentItem
    property bool showFullPostText: false
    property var videoItem: postOrRecordVideo ? videoLoader.item : null
    property var imageItem: postOrRecordImages.length > 0 ? imageLoader.item : null
    property bool zooming: imageItem ? imageItem.zooming : false

    signal closed
    signal imageLoaded

    id: videoPage
    width: root.width
    height: root.height + (endOfFeed ? root.height : 0) + extraFooterHeight
    color: guiSettings.fullScreenColor

    onOnScreenChanged: {
        if (onScreen) {
            if (feedAcceptsInteractions)
                videoPage.ListView.view.model.reportOnScreen(postUri)

            if (postOrRecordVideo)
                videoItem.play()
        } else {
            cover()
        }
    }

    function cover() {
        if (feedAcceptsInteractions)
            videoPage.ListView.view.model.reportOffScreen(postUri, postFeedContext)

        if (postOrRecordVideo)
            videoItem.pause()
    }

    Rectangle {
        property int bottomMargin: videoItem ? videoItem.playControlsHeight : 0
        property int mediaWidth: videoItem ? videoItem.playControlsWidth : (imageItem ? imageItem.imageWidth : width)
        property bool showDetails: videoItem ? videoItem.showPlayControls : imageLoader.showDetails

        id: mediaRect
        width: root.width
        height: root.height
        color: "transparent"

        Loader {
            id: videoLoader
            active: Boolean(postOrRecordVideo)

            sourceComponent: VideoView {
                id: video
                width: mediaRect.width
                height: root.height - videoPage.footerHeight
                maxHeight: root.height
                videoView: postOrRecordVideo
                contentVisibility: postContentVisibility
                contentWarning: postContentWarning
                contentLabeler: postContentLabeler
                controlColor: "white"
                disabledColor: "darkslategrey"
                backgroundColor: videoPage.color
                isFullViewMode: true
                swipeMode: true
                autoPlay: false
                useIfNeededHeight: postColumn.height

                onThumbImageLoaded: imageLoaded()

                onVideoLoaded: {
                    if (onScreen)
                        videoItem.play()
                }
            }
        }

        Loader {
            property bool showDetails: true

            id: imageLoader
            active: postOrRecordImages.length > 0

            sourceComponent: SwipeView {
                property int imageWidth: currentItem ? currentItem.imageWidth : 0
                property bool zooming: currentItem ? currentItem.zooming : false

                id: imgSwipeView
                width: mediaRect.width
                height: root.height
                interactive: !zooming

                Repeater {
                    model: postOrRecordImages.length

                    Rectangle {
                        required property int index
                        property int imageWidth: filter.imageVisible() ? img.paintedWidth : filter.width
                        property bool zooming: img.zooming

                        width: mediaRect.width
                        height: root.height
                        color: "transparent"

                        ImageWithZoom {
                            id: img
                            width: parent.width
                            height: parent.height
                            fillMode: Image.PreserveAspectFit
                            source: filter.getImage(index).fullSizeUrl
                            reloadIconColor: "white"

                            onHeightChanged: alignImage()
                            onPaintedHeightChanged: alignImage()

                            function alignImage() {
                                if (status === Image.Ready && height - paintedHeight < guiSettings.topAlignmentThreshold)
                                    verticalAlignment = Image.AlignTop;
                                else
                                    verticalAlignment = Image.AlignVCenter
                            }

                            SkyLabel {
                                x: (parent.width - parent.paintedWidth) / 2 + parent.paintedWidth - width - 10
                                y: parent.verticalAlignment == Image.AlignVCenter ? (parent.height - parent.paintedHeight) / 2 + 5 : optionsButton.y + optionsButton.height + 5
                                backgroundColor: "black"
                                backgroundOpacity: 0.6
                                color: "white"
                                text: `${index + 1}/${postOrRecordImages.length}`
                                visible: postOrRecordImages.length > 1 && filter.imageVisible() && showDetails
                            }

                            onStatusChanged: {
                                if (status == Image.Ready && index == videoPage.startImageIndex) {
                                    imgSwipeView.currentIndex = videoPage.startImageIndex
                                    imageLoaded()
                                }
                            }
                        }

                        MouseArea {
                            width: parent.width
                            height: parent.height

                            onClicked: showDetails = !showDetails
                            onDoubleClicked: (mouse) => {
                                showDetails = !showDetails
                                img.toggleFullScale(mouse.x, mouse.y)
                            }
                        }

                        FilteredImageWarning {
                            id: filter
                            x: 10
                            width: parent.width - 20
                            anchors.verticalCenter: parent.verticalCenter
                            contentVisibility: postContentVisibility
                            contentWarning: postContentWarning
                            contentLabeler: postContentLabeler
                            images: postOrRecordImages
                        }
                    }
                }
            }
        }
    }

    SvgButton {
        x: leftMarginWidth + 10
        y: headerHeight + 10
        iconColor: "white"
        Material.background: "transparent"
        svg: SvgOutline.arrowBack
        accessibleName: qsTr("go back")
        visible: mediaRect.showDetails
        onClicked: videoPage.closed()
    }

    SvgButton {
        id: optionsButton
        anchors.right: currentUserAvatar.active ? currentUserAvatar.left : parent.right
        anchors.rightMargin: currentUserAvatar.active ? 0 : rightMarginWidth + 10
        y: headerHeight + 10
        iconColor: "white"
        Material.background: "transparent"
        svg: SvgOutline.moreVert
        accessibleName: qsTr("more options")
        visible: mediaRect.showDetails
        onClicked: moreMenu.open()

        SkyMenu {
            id: moreMenu

            AccessibleMenuItem {
                text: qsTr("Save picture")
                textColor: "black"
                onTriggered: root.savePhoto(postOrRecordImages[imageItem.currentIndex].fullSizeUrl)
                visible: Boolean(imageItem)

                MenuItemSvg { svg: SvgOutline.save; color: "black" }
            }

            AccessibleMenuItem {
                text: qsTr("Share picture")
                textColor: "black"
                onTriggered: root.sharePhotoToApp(postOrRecordImages[imageItem.currentIndex].fullSizeUrl)
                visible: Boolean(imageItem)

                MenuItemSvg { svg: SvgOutline.share; color: "black" }
            }

            AccessibleMenuItem {
                text: qsTr("Save video")
                textColor: "black"
                onTriggered: root.saveVideo(videoItem.videoSource, postOrRecordVideo.playlistUrl)
                visible: Boolean(videoItem)

                MenuItemSvg { svg: SvgOutline.save; color: "black" }
            }
        }
    }

    Loader {
        id: currentUserAvatar
        anchors.right: parent.right
        anchors.rightMargin: rightMarginWidth + 10
        y: headerHeight + 15
        height: 40
        width: height
        active: !root.isActiveUser(userDid)
        visible: mediaRect.showDetails

        sourceComponent: CurrentUserAvatar {
            userDid: videoPage.userDid
        }
    }

    Rectangle {
        width: parent.width
        anchors.top: postColumn.top
        height: postColumn.height + (Boolean(postOrRecordVideo) ? 0 : (root.height - postColumn.y - postColumn.height))
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#00000000" }
            GradientStop { position: 1.0; color: "#EF000000" }
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
        x: Math.max(leftMarginWidth + 10, (mediaRect.width - width) / 2)
        anchors.bottom: mediaRect.bottom
        anchors.bottomMargin: mediaRect.bottomMargin + videoPage.footerHeight
        width: calcColumnWidth()
        visible: mediaRect.showDetails

        function calcColumnWidth() {
            let w = Math.max(mediaRect.mediaWidth, startImageWidth, 220)
            w = Math.min(w, mediaRect.width - leftMarginWidth - rightMarginWidth)
            w -= 20
            return w
        }

        Rectangle {
            width: parent.width
            height: 5
            color: "transparent"
        }

        PostHeaderWithAvatar {
            width: parent.width
            userDid: videoPage.userDid
            author: videoPage.author
            postIndexedSecondsAgo: videoPage.postIndexedSecondsAgo
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
            userDid: videoPage.userDid
            postAuthor: videoPage.author
            postText: videoPage.postText
            postPlainText: videoPage.postPlainText
            postHasUnknownEmbed: false
            postUnknownEmbedType: ""
            postImages: []
            postLanguageLabels: videoPage.postLanguages
            postContentLabels: videoPage.postLabels
            postContentVisibility: videoPage.postContentVisibility
            postContentWarning: videoPage.postContentWarning
            postContentLabeler: videoPage.postContentLabeler
            postMuted: videoPage.postMutedReason
            postIsThread: videoPage.postIsThread
            postIsThreadReply: videoPage.postIsThreadReply
            postDateTime: videoPage.postIndexedDateTime
            maxTextLines: videoPage.showFullPostText ? 1000 : 2
            bodyBackgroundColor: "transparent"

            onUnrollThread: {
                if (!postIsPlaceHolder && postUri)
                    skywalker.getPostThread(postUri, QEnums.POST_THREAD_UNROLLED)
            }
        }

        Loader {
            width: parent.width
            height: guiSettings.postStatsHeight(feedAcceptsInteractions, 10)
            active: true
            asynchronous: true

            sourceComponent: PostStats {
                id: postStats
                topPadding: 10
                skywalker: videoPage.skywalker
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
                showViewThread: true
                record: postRecord
                recordWithMedia: postRecordWithMedia
                feedAcceptsInteractions: videoPage.feedAcceptsInteractions

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
                            mouseEvent, postStats, videoPage.ListView.view,
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
                            mouseEvent, postStats, videoPage.ListView.view, postUri, postCid,
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
                        root.likeByNonActiveUser(mouseEvent, postStats, videoPage.ListView.view, postUri, postReasonRepostUri, postReasonRepostCid)
                }

                onBookmark: {
                    if (isBookmarked)
                        skywalker.getBookmarks().removeBookmark(postUri, postCid)
                    else
                        skywalker.getBookmarks().addBookmark(postUri, postCid)
                }

                onBookmarkLongPress: (mouseEvent) => {
                    if (root.isActiveUser(userDid))
                        root.bookmarkByNonActiveUser(mouseEvent, postStats, videoPage.ListView.view, postUri)
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

                onQuoteChain: {
                    if (!postIsPlaceHolder && postUri)
                        root.viewQuoteChain(postUri, userDid)
                }

                onMuteThread: root.muteThread(postIsReply ? postReplyRootUri : postUri, postThreadMuted, userDid)
                onThreadgate: root.gateRestrictions(postThreadgateUri, postIsReply ? postReplyRootUri : postUri, postIsReply ? postReplyRootCid : postCid, postUri, postReplyRestriction, postReplyRestrictionLists, postHiddenReplies, userDid)
                onHideReply: root.hidePostReply(postThreadgateUri, postReplyRootUri, postReplyRootCid, postUri, postReplyRestriction, postReplyRestrictionLists, postHiddenReplies, userDid)
                onEditPost: root.composePostEdit(videoPage.ListView.view.model, videoPage.index)
                onDeletePost: confirmDelete()
                onCopyPostText: skywalker.copyPostTextToClipboard(postPlainText)
                onReportPost: root.reportPost(postUri, postCid, postText, postIndexedDateTime, author, userDid)
                onTranslatePost: root.translateText(postPlainText)
                onDetachQuote: (uri, detach) => root.detachQuote(uri, postUri, postCid, detach, userDid)
                onPin: root.pinPost(postUri, postCid, userDid)
                onUnpin: root.unpinPost(postCid, userDid)
                onBlockAuthor: root.blockAuthor(author, userDid)
                onShowEmojiNames: root.showEmojiNamesList(postPlainText)
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

    GuiSettings {
        id: guiSettings
        isLightMode: false
        backgroundColor: videoPage.color
        linkColor: "#58a6ff"
        textColor: "white"
    }

    Loader {
        x: leftMarginWidth
        anchors.top: mediaRect.bottom
        height: root.height
        active: endOfFeed

        sourceComponent: Rectangle {
            width: mediaRect.width
            height: root.height
            color: "transparent"

            SvgButton {
                x: 10
                y: headerHeight + 10
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
                asynchronous: true
            }
        }
    }

    function confirmDelete() {
        guiSettings.askYesNoQuestion(
                    videoPage,
                    qsTr("Do you really want to delete your post?"),
                    () => root.deletePost(postUri, postCid, userDid))
    }

    function checkOnScreen() {}

    Component.onDestruction: {
        if (feedAcceptsInteractions && onScreen)
            videoPage.ListView.view.model.reportOffScreen(postUri, postFeedContext)
    }

    Component.onCompleted: {
        ListView.view.enableOnScreenCheck = true
    }
}
