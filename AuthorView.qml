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
    property string blocking: author.viewer.blocking
    property bool authorMuted: author.viewer.muted
    property int contentVisibility: QEnums.CONTENT_VISIBILITY_HIDE_POST // QEnums::ContentVisibility
    property string contentWarning: ""
    property bool showWarnedMedia: false

    signal closed

    id: page

    header: Rectangle {
        ImageAutoRetry {
            id: bannerImg
            anchors.top: parent.top
            width: parent.width
            source: author.banner
            fillMode: Image.PreserveAspectFit
            visible: author.banner && contentVisible() && status === Image.Ready
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
                avatarUrl: !contentVisible() ? "" : author.avatarUrl
                onClicked: root.viewFullImage([author.imageView], 0)
            }
        }
    }

    footer: Rectangle {
        width: parent.width

        PostButton {
            x: parent.width - width - 10
            y: -height - 10
            initialText: (isUser(author) || author.hasInvalidHandle()) ? "" : `@${author.handle} `
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
                    id: moreButton
                    Material.background: guiSettings.buttonColor
                    iconColor: guiSettings.buttonTextColor
                    svg: svgOutline.moreVert
                    onClicked: moreMenu.open()

                    Menu {
                        id: moreMenu
                        MenuItem {
                            text: qsTr("Share")
                            onTriggered: skywalker.shareAuthor(author)

                            MenuItemSvg {
                                svg: svgOutline.share
                            }
                        }
                        MenuItem {
                            text: authorMuted ? qsTr("Unmute account") : qsTr("Mute account")
                            enabled: !isUser(author)
                            onTriggered: {
                                if (authorMuted)
                                    graphUtils.unmute(author.did)
                                else
                                    graphUtils.mute(author.did)
                            }

                            MenuItemSvg {
                                svg: authorMuted ? svgOutline.unmute : svgOutline.mute
                                visible: parent.enabled
                            }
                        }
                        MenuItem {
                            text: blocking ? qsTr("Unblock account") : qsTr("Block account")
                            enabled: !isUser(author)
                            onTriggered: {
                                if (blocking)
                                    graphUtils.unblock(author.did, blocking)
                                else
                                    graphUtils.block(author.did)
                            }

                            MenuItemSvg {
                                svg: blocking ? svgOutline.unblock : svgOutline.block
                                visible: parent.enabled
                            }
                        }
                    }
                }
                SkyButton {
                    text: qsTr("Follow")
                    visible: !following && !isUser(author) && contentVisible()
                    onClicked: graphUtils.follow(author)
                }
                SkyButton {
                    flat: true
                    text: qsTr("Following")
                    visible: following && !isUser(author) && contentVisible()
                    onClicked: graphUtils.unfollow(author.did, following)
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
                SkyLabel {
                    text: qsTr("follows you")
                    visible: author.viewer.followedBy
                }
            }

            Rectangle {
                width: parent.width - (parent.leftPadding + parent.rightPadding)
                height: contentLabels.height

                ContentLabels {
                    id: contentLabels
                    anchors.left: parent.left
                    anchors.right: undefined
                    contentLabels: author.labels
                }
            }

            Row {
                id: statsRow
                width: parent.width - (parent.leftPadding + parent.rightPadding)
                spacing: 15
                topPadding: 10
                visible: contentVisible()

                Text {
                    color: guiSettings.linkColor
                    text: qsTr(`<b>${author.followersCount}</b> followers`)

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            let modelId = skywalker.createAuthorListModel(
                                    QEnums.AUTHOR_LIST_FOLLOWERS, author.did)
                            root.viewAuthorList(modelId, qsTr("Followers"))
                        }
                    }
                }
                Text {
                    color: guiSettings.linkColor
                    text: qsTr(`<b>${author.followsCount}</b> following`)

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            let modelId = skywalker.createAuthorListModel(
                                    QEnums.AUTHOR_LIST_FOLLOWS, author.did)
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
                visible: contentVisible()

                onLinkActivated: (link) => {
                                     if (link.startsWith("@")) {
                                         console.debug("MENTION:", link)
                                         skywalker.getDetailedProfile(link.slice(1))
                                     } else {
                                         root.openLink(link)
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
                    getFeed()
                }

                inTopOvershoot = true
            } else {
                inTopOvershoot = false
            }

            if (verticalOvershoot > 0) {
                if (!inBottomOvershoot && !skywalker.getAuthorFeedInProgress) {
                    getFeedNextPage()
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

        SvgImage {
            id: noPostImage
            width: 150
            height: 150
            y: height + (parent.headerItem ? parent.headerItem.height : 0)
            anchors.horizontalCenter: parent.horizontalCenter
            color: "grey"
            svg: {
                if (author.viewer.blockedBy || blocking) {
                    return svgOutline.block
                } else if (authorMuted) {
                    return svgOutline.mute
                } else if (!contentVisible()) {
                    return svgOutline.hideVisibility
                }

                return svgOutline.noPosts
            }
            visible: authorFeedView.count === 0
        }
        Text {
            id: noPostText
            y: noPostImage.y
            anchors.horizontalCenter: parent.horizontalCenter
            font.pointSize: guiSettings.scaledFont(10/8)
            color: "grey"
            elide: Text.ElideRight
            text: {
                if (blocking) {
                    return qsTr("You blocked this account")
                } else if (author.viewer.blockedBy) {
                    return qsTr("You are blocked")
                } else if (authorMuted) {
                    return qsTr("You muted this account")
                } else if (!contentVisible()) {
                    return contentWarning
                }

                return qsTr("No posts")
            }
            visible: authorFeedView.count === 0
        }
        Text {
            anchors.top: noPostText.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            elide: Text.ElideRight
            textFormat: Text.RichText
            text: "<br><a href=\"show\">" + qsTr("Show profile") + "</a>"
            visible: authorFeedView.count === 0 && !blocking && !author.viewer.blockedBy &&
                     !authorMuted && contentVisibilityIsWarning()
            onLinkActivated: {
                showWarnedMedia = true
                getFeed()
            }
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

        onBlockOk: (uri) => {
                       blocking = uri
                       skywalker.clearAuthorFeed(modelId)
                   }

        onBlockFailed: (error) => { statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR) }

        onUnblockOk: {
            blocking = ""
            getFeed()
        }

        onUnblockFailed: (error) => { statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR) }

        onMuteOk: {
            authorMuted = true
            skywalker.clearAuthorFeed(modelId)
        }

        onMuteFailed: (error) => { statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR) }

        onUnmuteOk: {
            authorMuted = false
            getFeed()
        }

        onUnmuteFailed: (error) => { statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR) }
    }

    GuiSettings {
        id: guiSettings
    }

    function getFeed() {
        if (mustGetFeed())
            skywalker.getAuthorFeed(modelId, 100)
    }

    function getFeedNextPage() {
        if (mustGetFeed())
            skywalker.getAuthorFeedNextPage(modelId)
    }

    function mustGetFeed() {
        return !authorMuted && !blocking && contentVisible()
    }

    function contentVisible() {
        if (author.viewer.blockedBy)
            return false

        return contentVisibility === QEnums.CONTENT_VISIBILITY_SHOW || showWarnedMedia
    }

    function contentVisibilityIsWarning() {
        return [QEnums.CONTENT_VISIBILITY_WARN_MEDIA,
                QEnums.CONTENT_VISIBILITY_WARN_POST].includes(contentVisibility) && !showWarnedMedia
    }

    function isUser(author) {
        return skywalker.getUserDid() === author.did
    }

    Component.onCompleted: {
        contentVisibility = skywalker.getContentVisibility(author.labels)
        contentWarning = skywalker.getContentWarning(author.labels)
        getFeed()
    }

    Component.onDestruction: {
        skywalker.removeAuthorFeedModel(modelId)
    }
}
