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
    property int headerHeight: 0
    property int footerHeight: 0
    property int leftMarginWidth: 0
    property int rightMarginWidth: 0
    property int extraFooterHeight: 0

    property bool onScreen: ListView.isCurrentItem
    property bool showFullPostText: false
    property var videoItem: postVideo ? videoLoader.item : null
    property var imageItem: postImages.length > 0 ? imageLoader.item : null
    property bool zooming: imageItem ? imageItem.zooming : false

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
        property int bottomMargin: videoItem ? videoItem.playControlsHeight : 0
        property int mediaWidth: videoItem ? videoItem.playControlsWidth : (imageItem ? imageItem.imageWidth : width)
        property bool showDetails: videoItem ? videoItem.showPlayControls : imageLoader.showDetails

        id: mediaRect
        x: leftMarginWidth
        width: root.width - leftMarginWidth - rightMarginWidth
        height: root.height
        color: "transparent"

        Loader {
            id: videoLoader
            active: Boolean(postVideo)

            sourceComponent: VideoView {
                id: video
                width: mediaRect.width
                height: root.height - videoPage.footerHeight
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

            sourceComponent: SwipeView {
                property int imageWidth: currentItem ? currentItem.imageWidth : 0
                property bool zooming: currentItem ? currentItem.zooming : false

                width: mediaRect.width
                height: root.height
                interactive: !zooming

                Repeater {
                    model: postImages.length

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

                            onHeightChanged: alighImage()
                            onPaintedHeightChanged: alighImage()

                            function alighImage() {
                                if (status === Image.Ready && height - paintedHeight < 300)
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
                                text: `${index + 1}/${postImages.length}`
                                visible: postImages.length > 1 && filter.imageVisible() && showDetails
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
                            images: postImages
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
                onTriggered: root.savePhoto(postImages[imageItem.currentIndex].fullSizeUrl)
                visible: Boolean(imageItem)

                MenuItemSvg { svg: SvgOutline.save; color: "black" }
            }

            AccessibleMenuItem {
                text: qsTr("Share picture")
                textColor: "black"
                onTriggered: root.sharePhotoToApp(postImages[imageItem.currentIndex].fullSizeUrl)
                visible: Boolean(imageItem)

                MenuItemSvg { svg: SvgOutline.share; color: "black" }
            }

            AccessibleMenuItem {
                text: qsTr("Save video")
                textColor: "black"
                onTriggered: root.saveVideo(videoItem.videoSource, postVideo.playlistUrl)
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
        //anchors.bottom: postColumn.bottom
        height: postColumn.height + (Boolean(postVideo) ? 0 : (root.height - postColumn.y - postColumn.height))
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
        x: leftMarginWidth + (mediaRect.width - width) / 2
        anchors.bottom: mediaRect.bottom
        anchors.bottomMargin: mediaRect.bottomMargin + videoPage.footerHeight
        width: Math.max(mediaRect.mediaWidth - 20, 200)
        visible: mediaRect.showDetails

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
            postMuted: videoPage.postMutedReason
            postIsThread: videoPage.postIsThread
            postIsThreadReply: videoPage.postIsThreadReply
            postDateTime: videoPage.postIndexedDateTime
            maxTextLines: videoPage.showFullPostText ? 1000 : 2
            bodyBackgroundColor: "transparent"

            onUnrollThread: {
                if (!postIsPlaceHolder && postUri)
                    skywalker.getPostThread(postUri, true)
            }
        }

        Loader {
            active: true
            width: parent.width
            height: guiSettings.statsHeight + 10
            asynchronous: true

            sourceComponent: PostStats {
                id: postStats
                width: parent.width
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
                isThread: postIsThread || postIsThreadReply
                showViewThread: true
                record: postRecord
                recordWithMedia: postRecordWithMedia
                feedAcceptsInteractions: videoPage.feedAcceptsInteractions

                onReply: {
                    const lang = postLanguages.length > 0 ? postLanguages[0].shortCode : ""
                    root.composeReply(postUri, postCid, postText, postIndexedDateTime,
                                      author, postReplyRootUri, postReplyRootCid, lang,
                                      postMentionDids, "", "", userDid)
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
                    root.repost(postRepostUri, postUri, postCid, postReasonRepostUri, postReasonRepostCid, postText,
                                postIndexedDateTime, author, postEmbeddingDisabled, postPlainText,
                                userDid)
                }

                function quote(quoteByDid = "") {
                    root.quotePost(postUri, postCid,
                            postText, postIndexedDateTime, author, postEmbeddingDisabled,
                            quoteByDid)
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

                onLike: root.like(postLikeUri, postUri, postCid, postReasonRepostUri, postReasonRepostCid, userDid)

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
                        skywalker.getPostThread(postUri, true)
                }

                onMuteThread: root.muteThread(postIsReply ? postReplyRootUri : postUri, postThreadMuted, userDid)
                onThreadgate: root.gateRestrictions(postThreadgateUri, postIsReply ? postReplyRootUri : postUri, postIsReply ? postReplyRootCid : postCid, postUri, postReplyRestriction, postReplyRestrictionLists, postHiddenReplies, userDid)
                onHideReply: root.hidePostReply(postThreadgateUri, postReplyRootUri, postReplyRootCid, postUri, postReplyRestriction, postReplyRestrictionLists, postHiddenReplies, userDid)
                onDeletePost: confirmDelete()
                onCopyPostText: skywalker.copyPostTextToClipboard(postPlainText)
                onReportPost: root.reportPost(postUri, postCid, postText, postIndexedDateTime, author, userDid)
                onTranslatePost: root.translateText(postPlainText)
                onDetachQuote: (uri, detach) => root.detachQuote(uri, postUri, postCid, detach, userDid)
                onPin: root.pinPost(postUri, postCid, userDid)
                onUnpin: root.unpinPost(postCid, userDid)
                onBlockAuthor: root.blockAuthor(author, userDid)
                onShowEmojiNames: root.showEmojiNamesList(postPlainText)
                onShowMoreLikeThis: root.showMoreLikeThis(feedDid, postUri, postFeedContext, userDid)
                onShowLessLikeThis: root.showLessLikeThis(feedDid, postUri, postFeedContext, userDid)
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

    function cover() {
        if (postVideo)
            videoItem.pause()
    }

    function checkOnScreen() {}

    Component.onCompleted: {
        ListView.view.enableOnScreenCheck = true
    }
}
