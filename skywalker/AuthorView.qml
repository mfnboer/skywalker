import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Page {
    required property var skywalker
    required property detailedprofile author
    required property int modelId

    property string following: author.viewer.following
    property string blocking: author.viewer.blocking
    property bool authorMuted: author.viewer.muted
    property int contentVisibility: QEnums.CONTENT_VISIBILITY_HIDE_POST // QEnums::ContentVisibility
    property string contentWarning: ""
    property bool showWarnedMedia: false
    property int replyModelId: -1
    property int mediaModelId: -1

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
            color: guiSettings.bannerDefaultColor
            visible: !bannerImg.visible
        }

        SvgButton {
            anchors.top: parent.top
            anchors.left: parent.left
            iconColor: guiSettings.headerTextColor
            Material.background: guiSettings.headerColor
            opacity: 0.5
            svg: svgOutline.arrowBack
            onClicked: page.closed()
        }

        Rectangle {
            id: avatar
            x: parent.width - width - 10
            y: getAvatarY()
            width: 104
            height: width
            radius: width / 2
            color: guiSettings.backgroundColor

            Avatar {
                anchors.centerIn: parent
                width: parent.width - 4
                height: parent.height - 4
                avatarUrl: !contentVisible() ? "" : author.avatarUrl
                onClicked:  {
                    if (author.avatarUrl)
                        root.viewFullImage([author.imageView], 0)
                }
            }
        }
    }

    function getBannerBottomY() {
        return bannerImg.visible ? bannerImg.y + bannerImg.height : noBanner.y + noBanner.height;
    }

    function getAvatarY() {
        const contentShift = Math.max(authorFeedView.contentY + getBannerBottomY(), 0)
        const shift = Math.min(contentShift, avatar.height / 2 + 10)
        return getBannerBottomY() - avatar.height / 2 - shift
    }

    footer: Rectangle {
        width: parent.width

        PostButton {
            x: parent.width - width - 10
            y: -height - 10
            svg: (isUser(author) || author.hasInvalidHandle()) ? svgOutline.chat : svgOutline.atSign
            overrideOnClicked: () => mentionPost()
        }
    }

    ListView {
        id: authorFeedView
        y: getBannerBottomY() + 10
        width: parent.width
        height: parent.height - y
        contentHeight: parent.height * 2
        spacing: 0
        model: 1
        boundsBehavior: Flickable.StopAtBounds
        flickDeceleration: guiSettings.flickDeceleration

        onContentYChanged: {
            if (contentY > -headerItem.getFeedMenuBarHeight() + 10) {
                contentY = -headerItem.getFeedMenuBarHeight() + 10
                interactive = false
            }
        }

        header: Column {
            width: parent.width
            leftPadding: 10
            rightPadding: 10

            RowLayout {
                SvgButton {
                    id: moreButton
                    svg: svgOutline.moreVert
                    onClicked: moreMenu.open()

                    Menu {
                        id: moreMenu
                        MenuItem {
                            text: qsTr("Translate")
                            onTriggered: root.translateText(author.description)

                            MenuItemSvg { svg: svgOutline.googleTranslate }
                        }
                        MenuItem {
                            text: qsTr("Share")
                            onTriggered: skywalker.shareAuthor(author)

                            MenuItemSvg { svg: svgOutline.share }
                        }
                        MenuItem {
                            text: authorMuted ? qsTr("Unmute account") : qsTr("Mute account")
                            enabled: !isUser(author) && author.viewer.mutedByList.isNull()
                            onTriggered: {
                                if (authorMuted)
                                    graphUtils.unmute(author.did)
                                else
                                    graphUtils.mute(author.did)
                            }

                            MenuItemSvg { svg: authorMuted ? svgOutline.unmute : svgOutline.mute }
                        }
                        MenuItem {
                            text: blocking ? qsTr("Unblock account") : qsTr("Block account")
                            enabled: !isUser(author) && author.viewer.blockingByList.isNull()
                            onTriggered: {
                                if (blocking)
                                    graphUtils.unblock(author.did, blocking)
                                else
                                    graphUtils.block(author.did)
                            }

                            MenuItemSvg { svg: blocking ? svgOutline.unblock : svgOutline.block }
                        }
                        MenuItem {
                            text: qsTr("Update lists")
                            onTriggered: updateLists()

                            MenuItemSvg { svg: svgOutline.list }
                        }
                        MenuItem {
                            text: qsTr("Report account")
                            onTriggered: root.reportAuthor(author)

                            MenuItemSvg { svg: svgOutline.report }
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
                font.bold: true
                font.pointSize: guiSettings.scaledFont(12/8)
                color: guiSettings.textColor
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
                color: "transparent"

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
                    color: guiSettings.textColor
                    text: qsTr(`<b>${author.postsCount}</b> posts`)
                }
            }

            Text {
                id: descriptionText
                width: parent.width - (parent.leftPadding + parent.rightPadding)
                topPadding: 10
                wrapMode: Text.Wrap
                textFormat: Text.RichText
                color: guiSettings.textColor
                text: postUtils.linkiFy(author.description, guiSettings.linkColor)
                visible: contentVisible()

                onLinkActivated: (link) => root.openLink(link)
            }

            TabBar {
                id: feedMenuBar
                width: parent.width - (parent.leftPadding + parent.rightPadding)

                TabButton {
                    text: qsTr("Posts")
                }
                TabButton {
                    text: qsTr("Replies")
                }
                TabButton {
                    text: qsTr("Media")
                }
            }

            Rectangle {
                x: -parent.leftPadding
                width: parent.width
                height: 1
                color: guiSettings.separatorColor
            }

            function getFeedMenuBarHeight() {
                return feedMenuBar.height
            }

            function getFeedMenuBar() {
                return feedMenuBar
            }
        }

        delegate: StackLayout {
            width: parent.width
            height: page.height - page.header.height
            currentIndex: authorFeedView.headerItem ? authorFeedView.headerItem.getFeedMenuBar().currentIndex : 0

            // Posts
            ListView {
                id: authorPostsFeed
                width: parent.width
                height: parent.height
                clip: true
                spacing: 0
                model: skywalker.getAuthorFeedModel(page.modelId)
                flickDeceleration: guiSettings.flickDeceleration
                ScrollIndicator.vertical: ScrollIndicator {}
                interactive: !authorFeedView.interactive

                onVerticalOvershootChanged: {
                    if (verticalOvershoot < 0) {
                        contentY = 0
                        authorFeedView.interactive = true
                    }
                }

                delegate: PostFeedViewDelegate {
                    viewWidth: authorFeedView.width
                }

                FlickableRefresher {
                    inProgress: skywalker.getAuthorFeedInProgress
                    verticalOvershoot: authorFeedView.verticalOvershoot
                    topOvershootFun: () => getFeed(page.modelId)
                    bottomOvershootFun: () => getFeedNextPage(page.modelId)
                    topText: ""
                }

                BusyIndicator {
                    id: busyIndicator
                    anchors.centerIn: parent
                    running: skywalker.getAuthorFeedInProgress
                }

                EmptyListIndication {
                    id: noPostIndication
                    y: authorFeedView.headerItem ? authorFeedView.headerItem.height : 0
                    svg: getEmptyListIndicationSvg()
                    text: getEmptyListIndicationText()
                    list: authorPostsFeed
                    onLinkActivated: (link) => root.viewListByUri(link, false)
                }
                Text {
                    anchors.top: noPostIndication.bottom
                    anchors.horizontalCenter: parent.horizontalCenter
                    elide: Text.ElideRight
                    textFormat: Text.RichText
                    text: `<br><a href=\"show\" style=\"color: ${guiSettings.linkColor};\">` + qsTr("Show profile") + "</a>"
                    visible: visibilityShowProfileLink()
                    onLinkActivated: disableWarning(modelId)
                }
            }

            // Replies
            ListView {
                id: authorRepliesFeed
                width: parent.width
                height: parent.height
                clip: true
                spacing: 0
                model: null
                flickDeceleration: guiSettings.flickDeceleration
                ScrollIndicator.vertical: ScrollIndicator {}
                interactive: !authorFeedView.interactive

                StackLayout.onIsCurrentItemChanged: {
                    if (StackLayout.isCurrentItem && replyModelId < 0) {
                        replyModelId = skywalker.createAuthorFeedModel(author, QEnums.AUTHOR_FEED_FILTER_REPLIES)
                        model = skywalker.getAuthorFeedModel(replyModelId)
                        getFeed(replyModelId)
                    }
                }

                onVerticalOvershootChanged: {
                    if (verticalOvershoot < 0) {
                        contentY = 0
                        authorFeedView.interactive = true
                    }
                }

                delegate: PostFeedViewDelegate {
                    viewWidth: authorFeedView.width
                }

                FlickableRefresher {
                    inProgress: skywalker.getAuthorFeedInProgress
                    verticalOvershoot: authorFeedView.verticalOvershoot
                    topOvershootFun: () => getFeed(replyModelId)
                    bottomOvershootFun: () => getFeedNextPage(replyModelId)
                    topText: ""
                }

                BusyIndicator {
                    id: replyBusyIndicator
                    anchors.centerIn: parent
                    running: skywalker.getAuthorFeedInProgress
                }

                EmptyListIndication {
                    id: noReplyIndication
                    y: authorFeedView.headerItem ? authorFeedView.headerItem.height : 0
                    svg: getEmptyListIndicationSvg()
                    text: getEmptyListIndicationText()
                    list: authorRepliesFeed
                    onLinkActivated: (link) => root.viewListByUri(link, false)
                }
                Text {
                    anchors.top: noReplyIndication.bottom
                    anchors.horizontalCenter: parent.horizontalCenter
                    elide: Text.ElideRight
                    textFormat: Text.RichText
                    text: `<br><a href=\"show\" style=\"color: ${guiSettings.linkColor};\">` + qsTr("Show profile") + "</a>"
                    visible: visibilityShowProfileLink()
                    onLinkActivated: disableWarning(replyModelId)
                }
            }

            // Media
            ListView {
                id: authorMediaFeed
                width: parent.width
                height: parent.height
                clip: true
                spacing: 0
                model: null
                flickDeceleration: guiSettings.flickDeceleration
                ScrollIndicator.vertical: ScrollIndicator {}
                interactive: !authorFeedView.interactive

                StackLayout.onIsCurrentItemChanged: {
                    if (StackLayout.isCurrentItem && mediaModelId < 0) {
                        mediaModelId = skywalker.createAuthorFeedModel(author, QEnums.AUTHOR_FEED_FILTER_MEDIA)
                        model = skywalker.getAuthorFeedModel(mediaModelId)
                        getFeed(mediaModelId)
                    }
                }

                onVerticalOvershootChanged: {
                    if (verticalOvershoot < 0) {
                        contentY = 0
                        authorFeedView.interactive = true
                    }
                }

                delegate: PostFeedViewDelegate {
                    viewWidth: authorFeedView.width
                }

                FlickableRefresher {
                    inProgress: skywalker.getAuthorFeedInProgress
                    verticalOvershoot: authorFeedView.verticalOvershoot
                    topOvershootFun: () => getFeed(mediaModelId)
                    bottomOvershootFun: () => getFeedNextPage(mediaModelId)
                    topText: ""
                }

                BusyIndicator {
                    id: mediaBusyIndicator
                    anchors.centerIn: parent
                    running: skywalker.getAuthorFeedInProgress
                }

                EmptyListIndication {
                    id: noMediaIndication
                    y: authorFeedView.headerItem ? authorFeedView.headerItem.height : 0
                    svg: getEmptyListIndicationSvg()
                    text: getEmptyListIndicationText()
                    list: authorMediaFeed
                    onLinkActivated: (link) => root.viewListByUri(link, false)
                }
                Text {
                    anchors.top: noMediaIndication.bottom
                    anchors.horizontalCenter: parent.horizontalCenter
                    elide: Text.ElideRight
                    textFormat: Text.RichText
                    text: `<br><a href=\"show\" style=\"color: ${guiSettings.linkColor};\">` + qsTr("Show profile") + "</a>"
                    visible: visibilityShowProfileLink()
                    onLinkActivated: disableWarning(mediaModelId)
                }
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
        }

        onUnblockFailed: (error) => { statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR) }

        onMuteOk: {
            authorMuted = true
            skywalker.clearAuthorFeed(modelId)
        }

        onMuteFailed: (error) => { statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR) }

        onUnmuteOk: {
            authorMuted = false
        }

        onUnmuteFailed: (error) => { statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR) }
    }

    GuiSettings {
        id: guiSettings
    }

    function mentionPost(text = "", imgSource = "") {
        const mentionText = (isUser(author) || author.hasInvalidHandle()) ? "" : `@${author.handle} `
        root.composePost(mentionText + text, imgSource)
    }

    function getFeed(modelId) {
        if (mustGetFeed())
            skywalker.getAuthorFeed(modelId, 100)
    }

    function getFeedNextPage(modelId) {
        if (mustGetFeed())
            skywalker.getAuthorFeedNextPage(modelId)
    }

    function updateLists() {
        let listModelId = skywalker.createListListModel(QEnums.LIST_TYPE_ALL, QEnums.LIST_PURPOSE_UNKNOWN, skywalker.getUserDid())
        skywalker.getListListModel(listModelId).setMemberCheckDid(author.did)
        let component = Qt.createComponent("AddUserListListView.qml")
        let view = component.createObject(page, { author: author, modelId: listModelId, skywalker: skywalker })
        view.onClosed.connect(() => { popStack() })
        pushStack(view)
        skywalker.getListList(listModelId)
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

    function visibilityShowProfileLink() {
        return authorFeedView.count === 0 && !blocking && !author.viewer.blockedBy &&
                !authorMuted && contentVisibilityIsWarning()
    }

    function disableWarning(modelId) {
        showWarnedMedia = true
        getFeed(modelId)
    }

    function getEmptyListIndicationSvg() {
        if (author.viewer.blockedBy || blocking) {
            return svgOutline.block
        } else if (authorMuted) {
            return svgOutline.mute
        } else if (!contentVisible()) {
            return svgOutline.hideVisibility
        }

        return svgOutline.noPosts
    }

    function getEmptyListIndicationText() {
        if (!author.viewer.blockingByList.isNull()) {
            return qsTr(`Blocked by list: <a href="${author.viewer.blockingByList.uri}" style="color: ${guiSettings.linkColor}">${author.viewer.blockingByList.name}</a>`)
        } else if (blocking) {
            return qsTr("You blocked this account")
        } else if (author.viewer.blockedBy) {
            return qsTr("You are blocked")
        } else if (!author.viewer.mutedByList.isNull()) {
            return qsTr(`Muted by list: <a href="${author.viewer.mutedByList.uri}" style="color: ${guiSettings.linkColor}">${author.viewer.mutedByList.name}</a>`)
        } else if (authorMuted) {
            return qsTr("You muted this account")
        } else if (!contentVisible()) {
            return contentWarning
        }

        return qsTr("No posts")
    }

    function isUser(author) {
        return skywalker.getUserDid() === author.did
    }

    Component.onCompleted: {
        contentVisibility = skywalker.getContentVisibility(author.labels)
        contentWarning = skywalker.getContentWarning(author.labels)
        getFeed(modelId)
    }

    Component.onDestruction: {
        skywalker.removeAuthorFeedModel(modelId)

        if (replyModelId >= 0)
            skywalker.removeAuthorFeedModel(replyModelId)

        if (mediaModelId >= 0)
            skywalker.removeAuthorFeedModel(mediaModelId)
    }
}
