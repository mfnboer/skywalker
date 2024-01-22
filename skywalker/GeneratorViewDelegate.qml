import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Rectangle {
    property int margin: 10
    required property int viewWidth
    required property generatorview feed
    required property int feedLikeCount
    required property string feedLikeUri
    required property profile feedCreator
    required property bool feedSaved
    required property bool feedPinned
    required property bool endOfFeed
    property int maxTextLines: 1000

    id: generatorView
    width: grid.width
    height: grid.height
    color: "transparent"

    GridLayout {
        id: grid
        rowSpacing: 0
        columns: 3
        width: viewWidth

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
        }

        Column {
            spacing: 0
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.rightMargin: generatorView.margin

            Text {
                width: parent.width
                elide: Text.ElideRight
                font.bold: true
                color: guiSettings.textColor
                text: feed.displayName
            }

            Text {
                topPadding: 5
                width: parent.width
                elide: Text.ElideRight
                color: guiSettings.textColor
                text: feedCreator.name


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
            }

            StatIcon {
                width: parent.width / 2
                iconColor: guiSettings.statsColor
                svg: svgOutline.moreVert
                onClicked: moreMenu.open()

                Menu {
                    id: moreMenu

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
        Text {
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
