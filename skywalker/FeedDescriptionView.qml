import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyPage {
    required property var skywalker
    required property generatorview feed
    property int feedLikeCount: feed.likeCount
    property string feedLikeUri: feed.viewer.like
    property bool feedLikeTransient: false
    property bool isSavedFeed: skywalker.favoriteFeeds.isSavedFeed(feed.uri)
    property bool isPinnedFeed: skywalker.favoriteFeeds.isPinnedFeed(feed.uri)
    property int contentVisibility: QEnums.CONTENT_VISIBILITY_HIDE_POST // QEnums::ContentVisibility
    property string contentWarning: ""
    property bool showWarnedMedia: false

    signal closed

    id: page

    Accessible.role: Accessible.Pane

    header: SimpleHeader {
        text: qsTr("Feed")
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

        FeedAvatar {
            x: 8
            y: 5
            width: 100
            avatarUrl: !contentVisible() ? "" : feed.avatar
            onClicked: {
                if (feed.avatar)
                    root.viewFullImage([feed.imageView], 0)
            }

            Accessible.role: Accessible.StaticText
            Accessible.name: qsTr("feed avatar") + (isPinnedFeed ? qsTr(", one of your favorites") : "")

            FavoriteStar {
                width: 30
                visible: isPinnedFeed
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
                elide: Text.ElideRight
                wrapMode: Text.Wrap
                maximumLineCount: 2
                font.bold: true
                font.pointSize: guiSettings.scaledFont(12/8)
                color: guiSettings.textColor
                plainText: feed.displayName
            }

            SkyCleanedTextLine {
                topPadding: 5
                width: parent.width
                elide: Text.ElideRight
                color: guiSettings.textColor
                plainText: feed.creator.name

                Accessible.role: Accessible.Link
                Accessible.name: feed.creator.name
                Accessible.onPressAction: skywalker.getDetailedProfile(feed.creator.did)

                MouseArea {
                    anchors.fill:  parent
                    onClicked: skywalker.getDetailedProfile(feed.creator.did)
                }
            }

            Text {
                topPadding: 2
                width: parent.width
                elide: Text.ElideRight
                font.pointSize: guiSettings.scaledFont(7/8)
                color: guiSettings.handleColor
                text: "@" + feed.creator.handle

                Accessible.role: Accessible.Link
                Accessible.name: text
                Accessible.onPressAction: skywalker.getDetailedProfile(feed.creator.did)

                MouseArea {
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

            Menu {
                id: moreMenu
                modal: true

                CloseMenuItem {
                    text: qsTr("<b>Feed</b>")
                    Accessible.name: qsTr("close more options menu")
                }
                AccessibleMenuItem {
                    text: isSavedFeed ? qsTr("Unsave feed") : qsTr("Save feed")
                    onTriggered: {
                        if (isSavedFeed)
                            skywalker.favoriteFeeds.removeFeed(feed)
                        else
                            skywalker.favoriteFeeds.addFeed(feed)

                        isSavedFeed = !isSavedFeed
                        isPinnedFeed = skywalker.favoriteFeeds.isPinnedFeed(feed.uri)
                        skywalker.saveFavoriteFeeds()
                    }

                    MenuItemSvg {
                        svg: isSavedFeed ? SvgOutline.remove : SvgOutline.add
                    }
                }
                AccessibleMenuItem {
                    text: isPinnedFeed ? qsTr("Remove favorite") : qsTr("Add favorite")
                    onTriggered: {
                        skywalker.favoriteFeeds.pinFeed(feed, !isPinnedFeed)
                        isPinnedFeed = !isPinnedFeed
                        isSavedFeed = skywalker.favoriteFeeds.isSavedFeed(feed.uri)
                        skywalker.saveFavoriteFeeds()
                    }

                    MenuItemSvg {
                        svg: isPinnedFeed ? SvgFilled.star : SvgOutline.star
                        color: isPinnedFeed ? guiSettings.favoriteColor : guiSettings.textColor
                    }
                }
                AccessibleMenuItem {
                    text: qsTr("Translate")
                    enabled: feed.description
                    onTriggered: root.translateText(feed.description)

                    MenuItemSvg {
                        svg: SvgOutline.googleTranslate
                    }
                }
                AccessibleMenuItem {
                    text: qsTr("Share")
                    onTriggered: skywalker.shareFeed(feed)

                    MenuItemSvg {
                        svg: SvgOutline.share
                    }
                }
                AccessibleMenuItem {
                    text: qsTr("Report feed")
                    onTriggered: root.reportFeed(feed)

                    MenuItemSvg {
                        svg: SvgOutline.report
                    }
                }
            }
        }

        Text {
            topPadding: 5
            bottomPadding: 10
            Layout.columnSpan: 3
            Layout.fillWidth: true
            wrapMode: Text.Wrap
            maximumLineCount: 1000
            elide: Text.ElideRight
            textFormat: Text.RichText
            color: guiSettings.textColor
            text: feed.formattedDescription

            onLinkActivated: (link) => root.openLink(link)

            Accessible.role: Accessible.StaticText
            Accessible.name: feed.description
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
                contentAuthorDid: feed.creator.did
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
                atUri: feed.uri
                count: feedLikeCount
                nameSingular: qsTr("like")
                namePlural: qsTr("likes")
                authorListType: QEnums.AUTHOR_LIST_LIKES
                authorListHeader: qsTr("Liked by")
            }
        }
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
            statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
        }

        onUndoLikeOk: {
            feedLikeCount--
            feedLikeUri = ""
            feedLikeTransient = false
        }
        onUndoLikeFailed: (error) => {
            feedLikeTransient = false
            statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
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

    function contentVisible() {
        if (feed.viewer.blockedBy)
            return false

        return contentVisibility === QEnums.CONTENT_VISIBILITY_SHOW || showWarnedMedia
    }

    Component.onCompleted: {
        contentVisibility = skywalker.getContentVisibility(feed.labels)
        contentWarning = skywalker.getContentWarning(feed.labels)
    }
}
