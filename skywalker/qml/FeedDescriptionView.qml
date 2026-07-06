import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyPage {
    property string userDid
    property Skywalker skywalker: root.getSkywalker(userDid)
    required property generatorview feed
    property int feedLikeCount: feed.likeCount
    property string feedLikeUri: feed.viewer.like
    property bool feedLikeTransient: false
    property bool isSavedFeed: skywalker.favoriteFeeds.isSavedFeed(feed.uri)
    property bool isPinnedFeed: skywalker.favoriteFeeds.isPinnedFeed(feed.uri)
    property bool feedHideFollowing: skywalker.getUserSettings().getFeedHideFollowing(skywalker.getUserDid(), feed.uri)
    property bool feedSync: skywalker.getUserSettings().mustSyncFeed(skywalker.getUserDid(), feed.uri)
    property int contentVisibility: QEnums.CONTENT_VISIBILITY_HIDE_POST // QEnums::ContentVisibility
    property bool showWarnedMedia: false
    property bool isVideoFeed: feed.contentMode === QEnums.CONTENT_MODE_VIDEO
    readonly property int postFeedModelId: skywalker.createPostFeedModel(feed)
    readonly property string sideBarTitle: isVideoFeed ? qsTr("Video feed") : qsTr("Feed")
    readonly property string sideBarFeedAvatarUrl: !contentVisible() ? "" : feed.avatar

    signal closed

    id: page

    Accessible.role: Accessible.Pane

    onIsPinnedFeedChanged: {
        if (!isPinnedFeed) {
            if (feedSync)
                syncFeed(false)

            if (feedHideFollowing)
                hideFollowing(false)
        }
    }

    header: SimpleHeader {
        userDid: page.userDid
        text: sideBarTitle
        visible: !root.showSideBar
        onBack: closed()
    }

    SwipeListView {
        id: feedStack
        width: parent.width
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        headerHeight: grid.height + feedsSeparator.height
        headerScrollHeight: grid.height

        SkyListView {
            id: postListView
            model: skywalker.getPostFeedModel(postFeedModelId)

            header: PlaceholderHeader { height: feedStack.headerHeight }
            headerPositioning: ListView.InlineHeader

            delegate: PostFeedViewDelegate {
                width: postListView.width
            }

            SwipeView.onIsCurrentItemChanged: {
                if (!SwipeView.isCurrentItem)
                    cover()
            }

            FlickableRefresher {
                inProgress: postListView.model && postListView.model.getFeedInProgress
                topOvershootFun: () => skywalker.getFeed(postFeedModelId)
                bottomOvershootFun: () => skywalker.getFeedNextPage(postFeedModelId)
                topText: qsTr("Pull down to refresh feed")
            }

            EmptyListIndication {
                y: feedStack.headerHeight
                svg: SvgOutline.noPosts
                text: qsTr("Feed is empty")
                list: postListView
            }

            BusyIndicator {
                id: busyIndicator
                anchors.centerIn: parent
                running: postListView.model && postListView.model.getFeedInProgress
            }
        }
    }

    GridLayout {
        id: grid
        rowSpacing: 0
        columns: 3
        x: 10
        y: Math.max(feedStack.headerTopMinY, feedStack.headerTopMaxY, -height)
        width: parent.width - 20

        Rectangle {
            Layout.columnSpan: 3
            Layout.fillWidth: true
            height: 10
            color: "transparent"
        }

        Rectangle {
            x: 8
            y: 5
            Layout.preferredWidth: 100
            Layout.preferredHeight: 100
            color: "transparent"

            FeedAvatar {
                id: feedAvatar
                width: parent.width
                height: parent.height
                userDid: page.userDid
                avatarUrl: !contentVisible() ? "" : feed.avatar
                unknownSvg: guiSettings.feedDefaultAvatar(feed)
                contentMode: feed.contentMode
                onClicked: {
                    if (feed.avatar) {
                        fullImageLoader.show(0)
                        feedAvatar.visible = false
                    }
                }

                Accessible.role: Accessible.StaticText
                Accessible.name: qsTr("feed avatar") + (isPinnedFeed ? qsTr(", one of your favorites") : "")

                FavoriteStar {
                    width: 30
                    visible: isPinnedFeed
                }
            }
        }

        Column {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0
            leftPadding: 10
            rightPadding: 10

            SkyCleanedText {
                width: parent.width
                bottomPadding: 5
                elide: Text.ElideRight
                wrapMode: Text.Wrap
                maximumLineCount: 2
                font.bold: true
                font.pointSize: guiSettings.scaledFont(12/8)
                color: guiSettings.textColor
                plainText: feed.displayName
            }

            AuthorNameAndStatus {
                width: parent.width
                userDid: page.userDid
                author: feed.creator

                Accessible.role: Accessible.Link
                Accessible.name: feed.creator.name
                Accessible.onPressAction: skywalker.getDetailedProfile(feed.creator.did)

                SkyMouseArea {
                    anchors.fill:  parent
                    onClicked: skywalker.getDetailedProfile(feed.creator.did)
                }
            }

            AccessibleText {
                topPadding: 2
                width: parent.width
                elide: Text.ElideRight
                font.pointSize: guiSettings.scaledFont(7/8)
                color: guiSettings.handleColor
                text: "@" + feed.creator.handle

                Accessible.role: Accessible.Link
                Accessible.name: text
                Accessible.onPressAction: skywalker.getDetailedProfile(feed.creator.did)

                SkyMouseArea {
                    anchors.fill:  parent
                    onClicked: skywalker.getDetailedProfile(feed.creator.did)
                }
            }
        }

        SvgButton {
            id: moreButton
            svg: SvgOutline.moreVert
            accessibleName: qsTr("more options")
            onClicked: moreMenu.open()

            SkyMenu {
                id: moreMenu

                SkyMenuButton {
                    text: isSavedFeed ? qsTr("Unsave feed") : qsTr("Save feed")
                    svg: isSavedFeed ? SvgOutline.remove : SvgOutline.add
                    popup: moreMenu
                    onClicked: {
                        if (isSavedFeed)
                            skywalker.favoriteFeeds.removeFeed(feed)
                        else
                            skywalker.favoriteFeeds.addFeed(feed)

                        isSavedFeed = !isSavedFeed
                        isPinnedFeed = skywalker.favoriteFeeds.isPinnedFeed(feed.uri)
                        skywalker.saveFavoriteFeeds()
                    }
                }
                SkyMenuButton {
                    text: isPinnedFeed ? qsTr("Remove favorite") : qsTr("Add favorite")
                    svg: isPinnedFeed ? SvgFilled.star : SvgOutline.star
                    svgColor: isPinnedFeed ? guiSettings.favoriteColor : guiSettings.textColor
                    popup: moreMenu
                    onClicked: {
                        skywalker.favoriteFeeds.pinFeed(feed, !isPinnedFeed)
                        isPinnedFeed = !isPinnedFeed
                        isSavedFeed = skywalker.favoriteFeeds.isSavedFeed(feed.uri)
                        skywalker.saveFavoriteFeeds()
                    }
                }
                TranslateMenuButton {
                    popup: moreMenu
                    enabled: feed.description
                    onClicked: root.translateText(feed.description)
                }
                SkyMenuButton {
                    text: qsTr("Share")
                    svg: SvgOutline.share
                    popup: moreMenu
                    onClicked: skywalker.getShareUtils().shareFeed(feed)
                }
                SkyMenuButton {
                    text: qsTr("Copy feed link")
                    svg: SvgOutline.link
                    popup: moreMenu
                    onClicked: skywalker.getShareUtils().copyUriToClipboard(feed.uri)
                }
                SkyMenuButton {
                    text: qsTr("Report feed")
                    svg: SvgOutline.report
                    popup: moreMenu
                    onClicked: root.reportFeed(feed, userDid)
                }
                SkyMenuButton {
                    text: qsTr("Emoji names")
                    svg: SvgOutline.emojiLanguage
                    popup: moreMenu
                    visible: UnicodeFonts.hasEmoji(feed.description)
                    onClicked: root.showEmojiNamesList(feed.description)
                }
                AccessibleMenuItem {
                    text: qsTr("Show following")
                    checkable: true
                    checked: !feedHideFollowing
                    onToggled: {
                        feedUtils.hideFollowing(feed.uri, !checked)
                        feedHideFollowing = !checked
                    }

                    SkyMouseArea {
                        anchors.fill: parent
                        enabled: !isPinnedFeed
                        onClicked: skywalker.showStatusMessage(qsTr("Show following can only be disabled for favorite feeds."), QEnums.STATUS_LEVEL_INFO, 10)
                    }
                }
                AccessibleMenuItem {
                    text: qsTr("Rewind on startup")
                    checkable: true
                    checked: feedSync
                    onToggled: {
                        feedUtils.syncFeed(feed.uri, checked)
                        feedSync = checked
                    }

                    SkyMouseArea {
                        anchors.fill: parent
                        enabled: !isPinnedFeed
                        onClicked: skywalker.showStatusMessage(qsTr("Rewinding can only be enabled for favorite lists."), QEnums.STATUS_LEVEL_INFO, 10)
                    }
                }
            }
        }

        FeedViewerState {
            Layout.columnSpan: 3
            Layout.fillWidth: true
            topPadding: 5
            hideFollowing: feedHideFollowing
            sync: feedSync
        }

        Rectangle {
            Layout.columnSpan: 3
            Layout.fillWidth: true
            height: contentLabels.height
            color: "transparent"

            ContentLabels {
                id: contentLabels
                anchors.left: parent.left
                anchors.right: undefined
                contentLabels: feed.labels
                contentAuthor: feed.creator
            }
        }

        SkyCleanedText {
            topPadding: 5
            bottomPadding: 10
            Layout.columnSpan: 3
            Layout.fillWidth: true
            wrapMode: Text.Wrap
            maximumLineCount: 1000
            elide: Text.ElideRight
            textFormat: Text.RichText
            color: guiSettings.textColor
            plainText: feed.formattedDescription

            LinkCatcher {
                containingText: feed.description
                author: feed.creator
            }
        }

        Rectangle {
            height: likeIcon.height
            Layout.columnSpan: 3
            Layout.fillWidth: true
            color: "transparent"

            StatIcon {
                id: likeIcon
                iconColor: feedLikeUri ? guiSettings.likeColor : guiSettings.statsColor
                svg: feedLikeUri ? SvgFilled.like : SvgOutline.like
                onClicked: likeFeed(feedLikeUri, feed.uri, feed.cid)
                Accessible.name: qsTr("like") + accessibilityUtils.statSpeech(feedLikeCount, qsTr("like"), qsTr("likes"))

                BlinkingOpacity {
                    target: likeIcon
                    running: feedLikeTransient
                }
            }

            StatAuthors {
                anchors.left: likeIcon.right
                anchors.top: parent.top
                anchors.leftMargin: 10
                userDid: page.userDid
                atUri: feed.uri
                count: feedLikeCount
                nameSingular: qsTr("like")
                namePlural: qsTr("likes")
                authorListType: QEnums.AUTHOR_LIST_LIKES
                authorListHeader: qsTr("Liked by")
            }
        }
    }

    Rectangle {
        id: feedsSeparator
        anchors.top: grid.bottom
        width: parent.width
        height: 1
        color: guiSettings.separatorColor
    }

    FullImageViewLoader {
        id: fullImageLoader
        thumbImageViewList: [feedAvatar.getImage()]
        images: [feed.imageView]
        onFinished: feedAvatar.visible = true
    }

    FeedUtils {
        id: feedUtils
        skywalker: page.skywalker

        onLikeOk: (likeUri) => {
            feedLikeCount++
            feedLikeUri = likeUri
            feedLikeTransient = false
        }
        onLikeFailed: (error) => {
            feedLikeTransient = false
            skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
        }

        onUndoLikeOk: {
            feedLikeCount--
            feedLikeUri = ""
            feedLikeTransient = false
        }
        onUndoLikeFailed: (error) => {
            feedLikeTransient = false
            skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
        }
    }

    AccessibilityUtils {
        id: accessibilityUtils
    }


    function likeFeed(likeUri, uri, cid) {
        feedLikeTransient = true

        if (likeUri)
            feedUtils.undoLike(likeUri, cid)
        else
            feedUtils.like(uri, cid)
    }

    function syncFeed(sync) {
        feedUtils.syncFeed(feed.uri, sync)
        feedSync = sync
    }

    function hideFollowing(hide) {
        feedUtils.hideFollowing(feed.uri, hide)
        feedHideFollowing = hide
    }

    function contentVisible() {
        if (feed.viewer.blockedBy)
            return false

        return contentVisibility === QEnums.CONTENT_VISIBILITY_SHOW || showWarnedMedia
    }

    Component.onDestruction: {
        skywalker.removePostFeedModel(postFeedModelId)
    }

    Component.onCompleted: {
        contentVisibility = skywalker.getContentVisibility(feed.labels)
        skywalker.getFeed(postFeedModelId)
    }
}
