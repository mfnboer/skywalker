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
    property int contentVisibility: QEnums.CONTENT_VISIBILITY_HIDE_POST // QEnums::ContentVisibility
    property bool showWarnedMedia: false
    property bool isVideoFeed: feed.contentMode === QEnums.CONTENT_MODE_VIDEO
    readonly property string sideBarTitle: isVideoFeed ? qsTr("Video feed") : qsTr("Feed")
    readonly property string sideBarFeedAvatarUrl: !contentVisible() ? "" : feed.avatar

    signal closed

    id: page

    Accessible.role: Accessible.Pane

    onIsPinnedFeedChanged: {
        if (!isPinnedFeed) {
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

    GridLayout {
        id: grid
        rowSpacing: 0
        columns: 3
        x: 10
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

                CloseMenuItem {
                    text: qsTr("<b>Feed</b>")
                    Accessible.name: qsTr("close more options menu")
                }
                AccessibleMenuItem {
                    text: isSavedFeed ? qsTr("Unsave feed") : qsTr("Save feed")
                    svg: isSavedFeed ? SvgOutline.remove : SvgOutline.add
                    onTriggered: {
                        if (isSavedFeed)
                            skywalker.favoriteFeeds.removeFeed(feed)
                        else
                            skywalker.favoriteFeeds.addFeed(feed)

                        isSavedFeed = !isSavedFeed
                        isPinnedFeed = skywalker.favoriteFeeds.isPinnedFeed(feed.uri)
                        skywalker.saveFavoriteFeeds()
                    }
                }
                AccessibleMenuItem {
                    text: isPinnedFeed ? qsTr("Remove favorite") : qsTr("Add favorite")
                    svg: isPinnedFeed ? SvgFilled.star : SvgOutline.star
                    svgColor: isPinnedFeed ? guiSettings.favoriteColor : guiSettings.textColor
                    onTriggered: {
                        skywalker.favoriteFeeds.pinFeed(feed, !isPinnedFeed)
                        isPinnedFeed = !isPinnedFeed
                        isSavedFeed = skywalker.favoriteFeeds.isSavedFeed(feed.uri)
                        skywalker.saveFavoriteFeeds()
                    }
                }
                AccessibleMenuItem {
                    text: qsTr("Translate")
                    svg: SvgOutline.googleTranslate
                    enabled: feed.description
                    onTriggered: root.translateText(feed.description)
                }
                AccessibleMenuItem {
                    text: qsTr("Share")
                    svg: SvgOutline.share
                    onTriggered: skywalker.shareFeed(feed)
                }
                AccessibleMenuItem {
                    text: qsTr("Report feed")
                    svg: SvgOutline.report
                    onTriggered: root.reportFeed(feed, userDid)
                }
                AccessibleMenuItem {
                    text: qsTr("Emoji names")
                    svg: SvgOutline.emojiLanguage
                    visible: UnicodeFonts.hasEmoji(feed.description)
                    onTriggered: root.showEmojiNamesList(feed.description)
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
            }
        }

        FeedViewerState {
            Layout.columnSpan: 3
            Layout.fillWidth: true
            topPadding: 5
            hideFollowing: feedHideFollowing
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

    function hideFollowing(hide) {
        feedUtils.hideFollowing(feed.uri, hide)
        feedHideFollowing = hide
    }

    function contentVisible() {
        if (feed.viewer.blockedBy)
            return false

        return contentVisibility === QEnums.CONTENT_VISIBILITY_SHOW || showWarnedMedia
    }

    Component.onCompleted: {
        contentVisibility = skywalker.getContentVisibility(feed.labels)
    }
}
