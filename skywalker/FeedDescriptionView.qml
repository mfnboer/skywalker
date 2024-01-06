import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Page {
    required property var skywalker
    required property generatorview feed
    property int feedLikeCount: feed.likeCount
    property string feedLikeUri: feed.viewer.like
    property bool isSavedFeed: skywalker.favoriteFeeds.isSavedFeed(feed.uri)
    property bool isPinnedFeed: skywalker.favoriteFeeds.isPinnedFeed(feed.uri)

    signal closed

    id: page

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
            avatarUrl: feed.avatar
            onClicked: {
                if (feed.avatar)
                    root.viewFullImage([feed.imageView], 0)
            }
        }

        Column {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0
            leftPadding: 10
            rightPadding: 10

            Text {
                width: parent.width
                elide: Text.ElideRight
                wrapMode: Text.Wrap
                maximumLineCount: 2
                font.bold: true
                font.pointSize: guiSettings.scaledFont(16/8)
                color: guiSettings.textColor
                text: feed.displayName
            }

            Text {
                topPadding: 5
                width: parent.width
                elide: Text.ElideRight
                color: guiSettings.textColor
                text: feed.creator.name

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


                MouseArea {
                    anchors.fill:  parent
                    onClicked: skywalker.getDetailedProfile(feed.creator.did)
                }
            }
        }

        SvgButton {
            id: moreButton
            svg: svgOutline.moreVert

            onClicked: moreMenu.open()

            Menu {
                id: moreMenu
                MenuItem {
                    text: isSavedFeed ? qsTr("Remove feed") : qsTr("Save feed")
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
                        svg: isSavedFeed ? svgOutline.remove : svgOutline.add
                    }
                }
                MenuItem {
                    text: isPinnedFeed ? qsTr("Remove favorite") : qsTr("Add favorite")
                    onTriggered: {
                        skywalker.favoriteFeeds.pinFeed(feed, !isPinnedFeed)
                        isPinnedFeed = !isPinnedFeed
                        isSavedFeed = skywalker.favoriteFeeds.isSavedFeed(feed.uri)
                        skywalker.saveFavoriteFeeds()
                    }

                    MenuItemSvg {
                        svg: isPinnedFeed ? svgFilled.star : svgOutline.star
                    }
                }
                MenuItem {
                    text: qsTr("Translate")
                    onTriggered: root.translateText(feed.description)

                    MenuItemSvg {
                        svg: svgOutline.googleTranslate
                    }
                }
                MenuItem {
                    text: qsTr("Share")
                    onTriggered: skywalker.shareFeed(feed)

                    MenuItemSvg {
                        svg: svgOutline.share
                    }
                }
                MenuItem {
                    text: qsTr("Report feed")
                    onTriggered: root.reportFeed(feed)

                    MenuItemSvg {
                        svg: svgOutline.report
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

            onLinkActivated: (link) => {
                if (link.startsWith("@")) {
                    console.debug("MENTION:", link)
                    skywalker.getDetailedProfile(link.slice(1))
                } else {
                    root.openLink(link)
                }
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
                svg: feedLikeUri ? svgFilled.like : svgOutline.like
                statistic: feedLikeCount

                onClicked: likeFeed(feedLikeUri, feed.uri, feed.cid)
            }
        }
    }

    FeedUtils {
        id: feedUtils
        skywalker: page.skywalker

        onLikeOk: (likeUri) => {
            feedLikeCount++
            feedLikeUri = likeUri
        }
        onLikeFailed: (error) => statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
        onUndoLikeOk: {
            feedLikeCount--
            feedLikeUri = ""
        }
        onUndoLikeFailed: (error) => statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
    }

    GuiSettings {
        id: guiSettings
    }

    function likeFeed(likeUri, uri, cid) {
        if (likeUri)
            feedUtils.undoLike(likeUri, cid)
        else
            feedUtils.like(uri, cid)
    }
}
