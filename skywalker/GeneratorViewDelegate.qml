import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Rectangle {
    property int margin: 10
    required property generatorview feed
    required property int feedLikeCount
    required property string feedLikeUri
    required property profile feedCreator
    required property bool feedSaved
    required property bool feedPinned
    required property bool endOfFeed
    property int maxTextLines: 1000

    id: generatorView
    height: grid.height
    color: "transparent"

    GridLayout {
        id: grid
        rowSpacing: 0
        columns: 3
        width: parent.width

        Rectangle {
            Layout.columnSpan: 3
            Layout.fillWidth: true
            height: 10
            color: "transparent"
        }

        FeedAvatar {
            Layout.leftMargin: generatorView.margin
            Layout.rightMargin: generatorView.margin
            x: 8
            y: 5
            width: guiSettings.threadBarWidth * 5
            avatarUrl: feed.avatar

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

            SkyCleanedText {
                width: parent.width
                elide: Text.ElideRight
                font.bold: true
                color: guiSettings.textColor
                plainText: feed.displayName
            }

            SkyCleanedText {
                topPadding: 5
                width: parent.width
                elide: Text.ElideRight
                color: guiSettings.textColor
                plainText: feedCreator.name

                Accessible.role: Accessible.Link
                Accessible.name: feedCreator.name
                Accessible.onPressAction: skywalker.getDetailedProfile(feedCreator.did)

                MouseArea {
                    anchors.fill:  parent
                    onClicked: skywalker.getDetailedProfile(feedCreator.did)
                }
            }

            Text {
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
        }

        Rectangle {
            width: 80
            Layout.fillHeight: true
            color: "transparent"

            Rectangle {
                anchors.right: addIcon.left
                width: 40
                height: width
                color: "transparent"

                Accessible.role: Accessible.Button
                Accessible.name: feedPinned ? qsTr("remove from favorites") : qsTr("add to favorites")
                Accessible.onPressAction: favoriteClicked(feed, !feedPinned)

                SvgImage {
                    id: favoIcon
                    width: 40
                    height: width
                    color: feedPinned ? guiSettings.favoriteColor : guiSettings.statsColor
                    svg: feedPinned ? svgFilled.star : svgOutline.star
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
                iconColor: flat ? guiSettings.textColor : guiSettings.buttonTextColor
                Material.background: flat ? guiSettings.labelColor : guiSettings.buttonColor
                svg: feedSaved ? svgOutline.remove : svgOutline.add
                accessibleName: feedSaved ? qsTr("remove from saved feeds") : qsTr("save feed")
                onClicked: addClicked(feed, !feedSaved)
            }
        }

        Text {
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

            onLinkActivated: (link) => root.openLink(link)

            Accessible.role: Accessible.StaticText
            Accessible.name: feed.description
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
                svg: feedLikeUri ? svgFilled.like : svgOutline.like
                statistic: feedLikeCount

                onClicked: root.likeFeed(feedLikeUri, feed.uri, feed.cid)

                Accessible.role: Accessible.Button
                Accessible.name: qsTr("like") + accessibilityUtils.statSpeech(feedLikeCount, "like", "likes")
                Accessible.onPressAction: clicked()
            }

            StatIcon {
                width: parent.width / 2
                iconColor: guiSettings.statsColor
                svg: svgOutline.moreVert
                onClicked: moreMenu.open()

                Accessible.role: Accessible.Button
                Accessible.name: qsTr("more options")
                Accessible.onPressAction: clicked()

                Menu {
                    id: moreMenu
                    modal: true

                    CloseMenuItem {
                        text: qsTr("<b>Feed</b>")
                        Accessible.name: qsTr("close more options menu")
                    }
                    AccessibleMenuItem {
                        text: qsTr("Translate")
                        enabled: feed.description
                        onTriggered: root.translateText(feed.description)

                        MenuItemSvg { svg: svgOutline.googleTranslate }
                    }
                    AccessibleMenuItem {
                        text: qsTr("Share")
                        onTriggered: skywalker.shareFeed(feed)

                        MenuItemSvg { svg: svgOutline.share }
                    }
                    AccessibleMenuItem {
                        text: qsTr("Report feed")
                        onTriggered: root.reportFeed(feed)

                        MenuItemSvg { svg: svgOutline.report }
                    }
                }
            }
        }

        Rectangle {
            Layout.columnSpan: 3
            Layout.fillWidth: true
            height: 5
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

    GuiSettings {
        id: guiSettings
    }

    function feedClicked(feed) {
        root.viewPostFeed(feed)
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
}
