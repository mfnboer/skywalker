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
    property bool inBottomOvershoot: false
    property string following: author.viewer.following

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
                SvgButton {
                    Material.background: guiSettings.buttonColor
                    iconColor: guiSettings.buttonTextColor
                    svg: svgOutline.moreVert
                }
                RoundButton {
                    id: followButton
                    Material.background: guiSettings.buttonColor
                    contentItem: Text {
                        leftPadding: 10
                        rightPadding: 10
                        color: guiSettings.buttonTextColor
                        text: qsTr("Follow")
                    }
                    visible: !following && !authorIsUser()

                    onClicked: graphUtils.follow(author.did)
                }
                RoundButton {
                    id: unfollowButton
                    flat: true
                    Material.background: guiSettings.labelColor
                    contentItem: Text {
                        leftPadding: 10
                        rightPadding: 10
                        color: guiSettings.textColor
                        text: qsTr("Following")
                    }
                    visible: following && !authorIsUser()

                    onClicked: graphUtils.unfollow(following)
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

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            // TODO: proper enum instead of 0
                            let modelId = skywalker.createAuthorListModel(0, author.did)
                            root.viewAuthorList(modelId, qsTr("Following"))
                        }
                    }
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
                    skywalker.getAuthorFeed(modelId, 100)
                }

                inTopOvershoot = true
            } else {
                inTopOvershoot = false
            }

            if (verticalOvershoot > 0) {
                if (!inBottomOvershoot && !skywalker.getAuthorFeedInProgress) {
                    skywalker.getAuthorFeedNextPage(modelId)
                }

                inBottomOvershoot = true;
            } else {
                inBottomOvershoot = false;
            }
        }

        BusyIndicator {
            id: busyIndicator
            anchors.centerIn: parent
            running: skywalker.getAuthorFeedInProgress
        }
    }

    StatusPopup {
        id: statusPopup
    }

    PostUtils {
        id: postUtils
        skywalker: page.skywalker
    }

    GraphUtils {
        id: graphUtils
        skywalker: page.skywalker

        onFollowOk: (uri) => { following = uri }
        onFollowFailed: (error) => { statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR) }
        onUnfollowOk: following = ""
        onUnfollowFailed: (error) => { statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR) }
    }

    GuiSettings {
        id: guiSettings
    }

    function authorIsUser() {
        return skywalker.getUserDid() === author.did
    }

    Component.onDestruction: {
        skywalker.removeAuthorFeedModel(modelId)
    }
}
