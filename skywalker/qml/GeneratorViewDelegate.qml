import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker

Rectangle {
    property int margin: 10
    property string userDid
    property Skywalker skywalker: root.getSkywalker(userDid)
    required property generatorview feed
    required property int feedLikeCount
    required property string feedLikeUri
    required property bool feedLikeTransient
    required property profile feedCreator
    required property bool feedSaved
    required property bool feedPinned
    required property bool feedHideFollowing
    required property bool endOfFeed
    property bool showFeed: feedVisible()
    property int maxTextLines: 1000

    signal hideFollowing(generatorview feed, bool hide)

    id: generatorView
    height: grid.height
    color: "transparent"

    onFeedPinnedChanged: {
        if (!feedPinned) {
            if (feedHideFollowing)
                hideFollowing(feed, false)
        }
    }

    GridLayout {
        id: grid
        rowSpacing: 0
        columns: 3
        width: parent.width

        Rectangle {
            Layout.columnSpan: 3
            Layout.fillWidth: true
            Layout.preferredHeight: 10
            color: "transparent"
        }

        FeedAvatar {
            Layout.leftMargin: generatorView.margin
            Layout.rightMargin: generatorView.margin
            Layout.alignment: Qt.AlignTop
            Layout.preferredWidth: guiSettings.threadColumnWidth
            Layout.preferredHeight: guiSettings.threadColumnWidth
            userDid: generatorView.userDid
            avatarUrl: showFeed ? feed.avatarThumb : ""
            contentMode: feed.contentMode
            unknownSvg: guiSettings.feedDefaultAvatar(feed)

            onClicked: feedClicked(feed)

            Accessible.role: Accessible.Button
            Accessible.name: qsTr(`go to feed: ${feed.displayName}`)
            Accessible.onPressAction: clicked()
        }

        Column {
            spacing: 0
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.rightMargin: generatorView.margin

            SkyCleanedTextLine {
                width: parent.width
                bottomPadding: 5
                elide: Text.ElideRight
                font.bold: true
                color: guiSettings.textColor
                plainText: feed.displayName
            }

            AuthorNameAndStatus {
                width: parent.width
                userDid: generatorView.userDid
                author: feedCreator

                Accessible.role: Accessible.Link
                Accessible.name: feedCreator.name
                Accessible.onPressAction: skywalker.getDetailedProfile(feedCreator.did)

                MouseArea {
                    anchors.fill:  parent
                    onClicked: skywalker.getDetailedProfile(feedCreator.did)
                }
            }

            AccessibleText {
                width: parent.width
                elide: Text.ElideRight
                font.pointSize: guiSettings.scaledFont(7/8)
                color: guiSettings.handleColor
                text: "@" + feedCreator.handle

                Accessible.role: Accessible.Link
                Accessible.name: text
                Accessible.onPressAction: skywalker.getDetailedProfile(feedCreator.did)

                MouseArea {
                    anchors.fill:  parent
                    onClicked: skywalker.getDetailedProfile(feedCreator.did)
                }
            }

            FeedViewerState {
                topPadding: 5
                hideFollowing: feedHideFollowing
            }
        }

        Rectangle {
            Layout.preferredWidth: 80
            Layout.fillHeight: true
            Layout.rightMargin: generatorView.margin
            color: "transparent"

            Rectangle {
                anchors.right: addIcon.left
                width: 40
                height: width
                color: "transparent"

                Accessible.role: Accessible.Button
                Accessible.name: feedPinned ? qsTr("remove from favorites") : qsTr("add to favorites")
                Accessible.onPressAction: favoriteClicked(feed, !feedPinned)

                SkySvg {
                    id: favoIcon
                    width: 40
                    height: width
                    color: feedPinned ? guiSettings.favoriteColor : guiSettings.statsColor
                    svg: feedPinned ? SvgFilled.star : SvgOutline.star
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: favoriteClicked(feed, !feedPinned)
                }
            }

            SvgButton {
                id: addIcon
                anchors.right: parent.right
                width: 40
                height: width
                flat: feedSaved
                iconColor: guiSettings.buttonTextColor
                Material.background: flat ? guiSettings.buttonFlatColor : guiSettings.buttonColor
                svg: feedSaved ? SvgOutline.remove : SvgOutline.add
                accessibleName: feedSaved ? qsTr("remove from saved feeds") : qsTr("save feed")
                onClicked: addClicked(feed, !feedSaved)
            }
        }

        Rectangle {
            Layout.columnSpan: 3
            Layout.fillWidth: true
            Layout.preferredHeight: contentLabels.height + 3
            color: "transparent"

            ContentLabels {
                id: contentLabels
                anchors.left: parent.left
                anchors.leftMargin: generatorView.margin
                anchors.right: undefined
                contentLabels: feed.labels
                contentAuthor: feed.creator
            }
        }

        AccessibleText {
            topPadding: 5
            bottomPadding: 10
            Layout.columnSpan: 3
            Layout.fillWidth: true
            Layout.leftMargin: generatorView.margin
            Layout.rightMargin: generatorView.margin
            wrapMode: Text.Wrap
            maximumLineCount: maxTextLines
            elide: Text.ElideRight
            textFormat: Text.RichText
            color: guiSettings.textColor
            text: feed.formattedDescription
            visible: showFeed

            LinkCatcher {
                containingText: feed.description
            }
        }

        Row {
            Layout.columnSpan: 3
            Layout.fillWidth: true
            Layout.leftMargin: generatorView.margin
            Layout.rightMargin: generatorView.margin

            StatIcon {
                id: likeIcon
                width: parent.width / 2
                iconColor: feedLikeUri ? guiSettings.likeColor : guiSettings.statsColor
                svg: feedLikeUri ? SvgFilled.like : SvgOutline.like
                statistic: feedLikeCount
                onClicked: root.likeFeed(feedLikeUri, feed.uri, feed.cid, userDid)

                BlinkingOpacity {
                    target: likeIcon
                    running: feedLikeTransient
                }

                Accessible.name: qsTr("like") + accessibilityUtils.statSpeech(feedLikeCount, qsTr("like"), qsTr("likes"))
            }

            StatIcon {
                width: parent.width / 2
                iconColor: guiSettings.statsColor
                svg: SvgOutline.moreVert
                onClicked: moreMenu.open()

                Accessible.name: qsTr("more options")

                SkyMenu {
                    id: moreMenu

                    CloseMenuItem {
                        text: qsTr("<b>Feed</b>")
                        Accessible.name: qsTr("close more options menu")
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
                        onToggled: hideFollowing(feed, !checked)

                        MouseArea {
                            anchors.fill: parent
                            enabled: !feedPinned
                            onClicked: skywalker.showStatusMessage(qsTr("Show following can only be disabled for favorite feeds."), QEnums.STATUS_LEVEL_INFO, 10)
                        }
                    }
                }
            }
        }

        Rectangle {
            Layout.columnSpan: 3
            Layout.fillWidth: true
            Layout.preferredHeight: 5
            color: "transparent"
        }

        Rectangle {
            Layout.columnSpan: 3
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: guiSettings.separatorColor
        }

        // End of feed indication
        AccessibleText {
            Layout.columnSpan: 3
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            topPadding: generatorView.margin
            elide: Text.ElideRight
            color: guiSettings.textColor
            text: qsTr("End of feed")
            font.italic: true
            visible: endOfFeed
        }
    }

    MouseArea {
        z: -2 // Let other mouse areas on top
        anchors.fill: parent
        onClicked: {
            console.debug("FEED CLICKED:", feed.displayName)
            generatorView.feedClicked(feed)
        }
    }

    AccessibilityUtils {
        id: accessibilityUtils
    }

    function feedClicked(feed) {
        root.viewPostFeed(feed, userDid)
    }

    function addClicked(feed, add) {
        if (add)
            skywalker.favoriteFeeds.addFeed(feed)
        else
            skywalker.favoriteFeeds.removeFeed(feed)

        skywalker.saveFavoriteFeeds()
    }

    function favoriteClicked(feed, add) {
        skywalker.favoriteFeeds.pinFeed(feed, add)
        skywalker.saveFavoriteFeeds()
    }

    function feedVisible() {
        return guiSettings.feedContentVisible(feed, userDid)
    }
}
