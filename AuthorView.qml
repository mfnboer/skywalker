import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Page {
    required property var skywalker
    required property detailedprofile author
    required property int modelId
    property int margin: 8

    property bool inTopOvershoot: false
    property bool gettingNewPosts: false
    property bool inBottomOvershoot: false
    property bool gettingNextPage: false

    signal closed

    id: page

    header: Rectangle {
        Image {
            id: bannerImg
            anchors.top: parent.top
            width: parent.width
            source: author.banner
            fillMode: Image.PreserveAspectFit
            visible: author.banner && status === Image.Ready
        }

        Rectangle {
            id: noBanner
            anchors.top: parent.top
            width: parent.width
            height: width / 3
            color: "blue"
            visible: !bannerImg.visible
        }

        SvgButton {
            anchors.top: parent.top
            anchors.left: parent.left
            iconColor: "white"
            Material.background: "black"
            opacity: 0.5
            svg: svgOutline.arrowBack
            onClicked: page.closed()
        }

        Rectangle {
            id: avatar
            x: parent.width - width - 10
            y: (bannerImg.visible ? bannerImg.y + bannerImg.height : noBanner.y + noBanner.height) - height / 2
            width: 104
            height: width
            radius: width / 2
            color: "white"

            Avatar {
                anchors.centerIn: parent
                width: parent.width - 4
                height: parent.height - 4
                avatarUrl: author.avatarUrl
                onClicked: root.viewFullImage([author.imageView], 0)
            }
        }
    }

    ListView {
        id: authorFeedView
        y: avatar.y + avatar.height / 2 + 10
        width: parent.width
        height: parent.height - y
        spacing: 0
        model: skywalker.getAuthorFeedModel(page.modelId)
        ScrollIndicator.vertical: ScrollIndicator {}

        header: Column {
            width: parent.width
            leftPadding: 10
            rightPadding: 10

            RowLayout {
                Button {
                    Material.background: guiSettings.buttonColor
                    contentItem: Text {
                        color: guiSettings.buttonTextColor
                        text: qsTr("Follow")
                    }
                    visible: !author.viewer.following && skywalker.getUserDid() !== author.did
                }
                Button {
                    flat: true
                    Material.background: guiSettings.labelColor
                    contentItem: Text {
                        color: guiSettings.textColor
                        text: qsTr("following")
                    }
                    visible: author.viewer.following && skywalker.getUserDid() !== author.did
                }
                SvgButton {
                    Material.background: guiSettings.buttonColor
                    iconColor: guiSettings.buttonTextColor
                    svg: svgOutline.moreVert
                }
            }

            Text {
                id: nameText
                width: parent.width - (parent.leftPadding + parent.rightPadding)
                elide: Text.ElideRight
                wrapMode: Text.Wrap
                maximumLineCount: 2
                font.pointSize: guiSettings.scaledFont(16/8)
                text: author.name
            }

            RowLayout {
                width: parent.width - (parent.leftPadding + parent.rightPadding)

                Text {
                    id: handleText
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                    color: guiSettings.handleColor
                    text: `@${author.handle}`
                }
                Label {
                    padding: 3
                    background: Rectangle { color: guiSettings.labelColor }
                    font.pointSize: guiSettings.labelFontSize
                    text: qsTr("follows you")
                    visible: author.viewer.followedBy
                }
            }

            Row {
                id: statsRow
                width: parent.width - (parent.leftPadding + parent.rightPadding)
                spacing: 15
                topPadding: 10

                Text {
                    color: guiSettings.linkColor
                    text: qsTr(`<b>${author.followersCount}</b> followers`)
                }
                Text {
                    color: guiSettings.linkColor
                    text: qsTr(`<b>${author.followsCount}</b> following`)
                }
                Text {
                    text: qsTr(`<b>${author.postsCount}</b> posts`)
                }
            }

            Text {
                id: descriptionText
                width: parent.width - (parent.leftPadding + parent.rightPadding)
                topPadding: 10
                wrapMode: Text.Wrap
                textFormat: Text.RichText
                text: postUtils.linkiFy(author.description)

                onLinkActivated: (link) => {
                                     if (link.startsWith("@")) {
                                         console.debug("MENTION:", link)
                                         skywalker.getDetailedProfile(link.slice(1))
                                     } else {
                                         Qt.openUrlExternally(link)
                                     }
                                 }
            }

            Text {
                width: parent.width - (parent.leftPadding + parent.rightPadding)
                topPadding: 10
                bottomPadding: 10
                font.bold: true
                text: qsTr("Posts")
            }

            Rectangle {
                x: -parent.leftPadding
                width: parent.width
                height: 1
                color: "lightgrey"
            }
        }

        delegate: PostFeedViewDelegate {
            viewWidth: authorFeedView.width
        }

        onVerticalOvershootChanged: {
            if (verticalOvershoot < 0)  {
                if (!inTopOvershoot && !skywalker.getAuthorFeedInProgress) {
                    gettingNewPosts = true
                    skywalker.getAuthorFeed(modelId, 100)
                }

                inTopOvershoot = true
            } else {
                inTopOvershoot = false
            }

            if (verticalOvershoot > 0) {
                if (!inBottomOvershoot && !skywalker.getAuthorFeedInProgress) {
                    gettingNextPage = true
                    skywalker.getAuthorFeedNextPage(modelId)
                }

                inBottomOvershoot = true;
            } else {
                inBottomOvershoot = false;
            }
        }

        BusyIndicator {
            id: busyTopIndicator
            y: parent.y + guiSettings.headerHeight
            anchors.horizontalCenter: parent.horizontalCenter
            running: gettingNewPosts
        }

        BusyIndicator {
            id: busyBottomIndicator
            y: parent.y + parent.height - height - guiSettings.footerHeight
            anchors.horizontalCenter: parent.horizontalCenter
            running: gettingNextPage
        }
    }

    PostUtils {
        id: postUtils
        skywalker: page.skywalker
    }

    GuiSettings {
        id: guiSettings
    }

    function feedInProgressChanged() {
        if (!skywalker.getAuthorFeedInProgress) {
            gettingNewPosts = false
            gettingNextPage = false
        }
    }

    Component.onCompleted: {
        skywalker.onGetAuthorFeedInProgressChanged.connect(feedInProgressChanged)
    }
    Component.onDestruction: {
        skywalker.onGetAuthorFeedInProgressChanged.disconnect(feedInProgressChanged)
        skywalker.removeAuthorFeedModel(modelId)
    }
}
