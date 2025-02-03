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
                        property int imageWidth: img.paintedWidth
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
                                y: (parent.height - parent.paintedHeight) / 2 + 5
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
        anchors.right: parent.right
        anchors.rightMargin: rightMarginWidth + 10
        y: headerHeight + 10
        iconColor: "white"
        Material.background: "transparent"
        svg: SvgOutline.moreVert
        accessibleName: qsTr("more options")
        visible: mediaRect.showDetails
        onClicked: moreMenu.open()

        Menu {
            id: moreMenu
            modal: true

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
        x: leftMarginWidth + (mediaRect.width - width) / 2
        anchors.bottom: mediaRect.bottom
        anchors.bottomMargin: mediaRect.bottomMargin + videoPage.footerHeight
        width: mediaRect.mediaWidth - 20
        visible: mediaRect.showDetails

        Rectangle {
            width: parent.width
            height: 5
            color: "transparent"
        }

        PostHeaderWithAvatar {
            width: parent.width
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
