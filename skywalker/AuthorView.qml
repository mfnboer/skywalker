import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Page {
    required property var skywalker
    required property detailedprofile author
    required property int modelId

    // Initialize properties in onCompleted.
    // For some weird reason description and banner are not available here.
    // Must have something to do with those properties being in subclasses of BasicProfile.
    property string authorName
    property string authorDescription
    property string authorAvatar
    property string authorBanner
    property string following: author.viewer.following
    property string blocking: author.viewer.blocking
    property bool authorMuted: author.viewer.muted
    property bool authorMutedReposts: false
    property int contentVisibility: QEnums.CONTENT_VISIBILITY_HIDE_POST // QEnums::ContentVisibility
    property string contentWarning: ""
    property bool showWarnedMedia: false
    readonly property int feedListModelId: skywalker.createFeedListModel()
    property bool hasFeeds: false
    readonly property int listListModelId: skywalker.createListListModel(QEnums.LIST_TYPE_ALL, QEnums.LIST_PURPOSE_UNKNOWN, author.did)
    property bool hasLists: false

    signal closed

    id: page

    Accessible.role: Accessible.Pane
    Accessible.name: qsTr(`${author.name}\n\n@${author.handle}`)

    header: Rectangle {
        width: parent.width
        height: bannerImg.visible ? bannerImg.height : noBanner.height

        ImageAutoRetry {
            id: bannerImg
            anchors.top: parent.top
            width: parent.width
            source: authorBanner
            fillMode: Image.PreserveAspectFit
            visible: authorBanner && contentVisible() && status === Image.Ready
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

            Accessible.role: Accessible.Button
            Accessible.name: qsTr("go back")
            Accessible.onPressAction: clicked()
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
                avatarUrl: !contentVisible() ? "" : authorAvatar
                onClicked:  {
                    if (authorAvatar)
                        root.viewFullImage([author.imageView], 0)
                }
            }
        }
    }

    function getAvatarY() {
        const contentShift = Math.max(authorFeedView.contentY + header.height, 0)
        const shift = Math.min(contentShift, avatar.height / 2 + 10)
        return header.height - avatar.height / 2 - shift
    }

    footer: Rectangle {
        width: parent.width

        PostButton {
            y: -height - 10
            svg: (isUser(author) || author.hasInvalidHandle()) ? svgOutline.chat : svgOutline.atSign
            overrideOnClicked: () => mentionPost()

            Accessible.role: Accessible.Button
            Accessible.name: qsTr(`post and mention ${author.name}`)
            Accessible.onPressAction: clicked()
        }
    }

    ListView {
        id: authorFeedView
        y: 10
        width: parent.width
        height: parent.height - y
        contentHeight: parent.height * 2
        spacing: 0
        model: 1
        boundsBehavior: Flickable.StopAtBounds
        flickDeceleration: guiSettings.flickDeceleration

        Accessible.role: Accessible.List

        onContentYChanged: {
            if (contentY > -headerItem.getFeedMenuBarHeight() + 10) {
                contentY = -headerItem.getFeedMenuBarHeight() + 10
                interactive = false
            }
            else if (!interactive) {
                // When a post thread is opened, Qt changes contentY??
                contentY = -headerItem.getFeedMenuBarHeight() + 10
            }
        }

        header: Column {
            width: parent.width
            leftPadding: 10
            rightPadding: 10

            Accessible.role: Accessible.StaticText
            Accessible.name: qsTr(`${author.name}\n\n@${author.handle}\n\n${author.description}`)

            RowLayout {
                SvgButton {
                    svg: svgOutline.edit
                    onClicked: editAuthor(author)
                    visible: isUser(author)

                    Accessible.role: Accessible.Button
                    Accessible.name: qsTr("edit your profile")
                    Accessible.onPressAction: clicked()
                }

                SvgButton {
                    id: moreButton
                    svg: svgOutline.moreVert
                    onClicked: moreMenu.open()

                    Accessible.role: Accessible.Button
                    Accessible.name: qsTr("more options")
                    Accessible.onPressAction: clicked()

                    Menu {
                        id: moreMenu
                        modal: true

                        onAboutToShow: root.enablePopupShield(true)
                        onAboutToHide: root.enablePopupShield(false)

                        CloseMenuItem {
                            Accessible.name: qsTr("close more options menu")
                        }
                        AccessibleMenuItem {
                            text: qsTr("Translate")
                            enabled: authorDescription
                            onTriggered: root.translateText(authorDescription)

                            MenuItemSvg { svg: svgOutline.googleTranslate }
                        }
                        AccessibleMenuItem {
                            text: qsTr("Share")
                            onTriggered: skywalker.shareAuthor(author)

                            MenuItemSvg { svg: svgOutline.share }
                        }
                        AccessibleMenuItem {
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
                        AccessibleMenuItem {
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
                        AccessibleMenuItem {
                           text: authorMutedReposts ? qsTr("Unmute reposts") : qsTr("Mute reposts")
                           enabled: !isUser(author)
                           onTriggered: {
                               if (authorMutedReposts)
                                   graphUtils.unmuteReposts(author.did)
                               else
                                   graphUtils.muteReposts(author)
                           }

                           MenuItemSvg { svg: svgOutline.repost }
                        }

                        AccessibleMenuItem {
                            text: qsTr("Update lists")
                            onTriggered: updateLists()

                            MenuItemSvg { svg: svgOutline.list }
                        }
                        AccessibleMenuItem {
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

                    Accessible.role: Accessible.Button
                    Accessible.name: qsTr(`press to follow ${author.name}`)
                    Accessible.onPressAction: clicked()
                }
                SkyButton {
                    flat: true
                    text: qsTr("Following")
                    visible: following && !isUser(author) && contentVisible()
                    onClicked: graphUtils.unfollow(author.did, following)

                    Accessible.role: Accessible.Button
                    Accessible.name: qsTr(`press to unfollow ${author.name}`)
                    Accessible.onPressAction: clicked()
                }
            }

            SkyCleanedText {
                id: nameText
                width: parent.width - (parent.leftPadding + parent.rightPadding)
                elide: Text.ElideRight
                wrapMode: Text.Wrap
                maximumLineCount: 3
                font.bold: true
                font.pointSize: guiSettings.scaledFont(12/8)
                color: guiSettings.textColor
                plainText: authorName

                Accessible.ignored: true
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
                    text: qsTr("muted reposts")
                    visible: authorMutedReposts
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
                    contentAuthorDid: author.did
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

                    Accessible.role: Accessible.Link
                    Accessible.name: unicodeFonts.toPlainText(text)
                    Accessible.onPressAction: showFollowers()

                    MouseArea {
                        anchors.fill: parent
                        onClicked: parent.showFollowers()
                    }

                    function showFollowers() {
                        let modelId = skywalker.createAuthorListModel(
                                QEnums.AUTHOR_LIST_FOLLOWERS, author.did)
                        root.viewAuthorList(modelId, qsTr("Followers"))
                    }
                }
                Text {
                    color: guiSettings.linkColor
                    text: qsTr(`<b>${author.followsCount}</b> following`)

                    Accessible.role: Accessible.Link
                    Accessible.name: unicodeFonts.toPlainText(text)
                    Accessible.onPressAction: showFollowing()

                    MouseArea {
                        anchors.fill: parent
                        onClicked: parent.showFollowing()   
                    }

                    function showFollowing() {
                        let modelId = skywalker.createAuthorListModel(
                                QEnums.AUTHOR_LIST_FOLLOWS, author.did)
                        root.viewAuthorList(modelId, qsTr("Following"))
                    }
                }
                Text {
                    color: guiSettings.textColor
                    text: qsTr(`<b>${author.postsCount}</b> posts`)

                    Accessible.role: Accessible.StaticText
                    Accessible.name: unicodeFonts.toPlainText(text)
                }
            }

            Text {
                id: descriptionText
                width: parent.width - (parent.leftPadding + parent.rightPadding)
                topPadding: 10
                wrapMode: Text.Wrap
                textFormat: Text.RichText
                color: guiSettings.textColor
                text: postUtils.linkiFy(authorDescription, guiSettings.linkColor)
                visible: contentVisible()

                onLinkActivated: (link) => root.openLink(link)
            }

            TabBar {
                id: feedMenuBar
                width: parent.width - (parent.leftPadding + parent.rightPadding)

                AccessibleTabButton {
                    text: qsTr("Posts")
                    width: implicitWidth
                }
                AccessibleTabButton {
                    text: qsTr("Replies")
                    width: implicitWidth
                }
                AccessibleTabButton {
                    text: qsTr("Media")
                    width: implicitWidth
                }
                AccessibleTabButton {
                    text: qsTr("Likes")
                    visible: isUser(author)
                    width: isUser(author) ? implicitWidth : 0
                }
                AccessibleTabButton {
                    text: qsTr("Feeds")
                    visible: hasFeeds
                    width: hasFeeds ? implicitWidth : 0
                }
                AccessibleTabButton {
                    text: qsTr("Lists")
                    visible: hasLists
                    width: hasLists ? implicitWidth : 0
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
            id: feedStack
            width: parent.width

            // -1 to make the interactive enable/disable work
            height: page.height - (authorFeedView.headerItem ? authorFeedView.headerItem.getFeedMenuBarHeight() + page.header.height - 1 : 0)

            currentIndex: authorFeedView.headerItem ? authorFeedView.headerItem.getFeedMenuBar().currentIndex : 0

            // Posts
            AuthorPostsList {
                id: authorPostsList
                author: page.author
                enclosingView: authorFeedView
                getFeed: (id) => page.getFeed(id)
                getFeedNextPage: (id) => page.getFeedNextPage(id)
                getEmptyListIndicationSvg: () => page.getEmptyListIndicationSvg()
                getEmptyListIndicationText: () => page.getEmptyListIndicationText()
                visibilityShowProfileLink: (list) => page.visibilityShowProfileLink(list)
                disableWarning: () => page.disableWarning()
                modelId: page.modelId
            }

            // Replies
            AuthorPostsList {
                id: authorRepliesList
                author: page.author
                enclosingView: authorFeedView
                getFeed: (id) => page.getFeed(id)
                getFeedNextPage: (id) => page.getFeedNextPage(id)
                getEmptyListIndicationSvg: () => page.getEmptyListIndicationSvg()
                getEmptyListIndicationText: () => page.getEmptyListIndicationText()
                visibilityShowProfileLink: (list) => page.visibilityShowProfileLink(list)
                disableWarning: () => page.disableWarning()
                feedFilter: QEnums.AUTHOR_FEED_FILTER_REPLIES
            }

            // Media
            AuthorPostsList {
                id: authorMediaList
                author: page.author
                enclosingView: authorFeedView
                getFeed: (id) => page.getFeed(id)
                getFeedNextPage: (id) => page.getFeedNextPage(id)
                getEmptyListIndicationSvg: () => page.getEmptyListIndicationSvg()
                getEmptyListIndicationText: () => page.getEmptyListIndicationText()
                visibilityShowProfileLink: (list) => page.visibilityShowProfileLink(list)
                disableWarning: () => page.disableWarning()
                feedFilter: QEnums.AUTHOR_FEED_FILTER_MEDIA
            }

            // Likes
            AuthorPostsList {
                id: authorLikesList
                author: page.author
                enclosingView: authorFeedView
                getFeed: (id) => skywalker.getAuthorLikes(id)
                getFeedNextPage: (id) => skywalker.getAuthorLikesNextPage(id)
                getEmptyListIndicationSvg: () => page.getEmptyListIndicationSvg()
                getEmptyListIndicationText: () => page.getEmptyListIndicationText()
                visibilityShowProfileLink: (list) => page.visibilityShowProfileLink(list)
                disableWarning: () => page.disableWarning()
                feedFilter: QEnums.AUTHOR_FEED_FILTER_NONE
            }

            // Feeds
            ListView {
                id: authorFeedList
                width: parent.width
                height: parent.height
                clip: true
                spacing: 0
                model: skywalker.getFeedListModel(feedListModelId)
                flickDeceleration: guiSettings.flickDeceleration
                ScrollIndicator.vertical: ScrollIndicator {}
                interactive: !authorFeedView.interactive

                onVerticalOvershootChanged: {
                    if (verticalOvershoot < 0)
                        authorFeedView.interactive = true
                }

                onCountChanged: hasFeeds = count

                delegate: GeneratorViewDelegate {
                    width: authorFeedView.width
                }

                FlickableRefresher {
                    inProgress: skywalker.getFeedInProgress
                    verticalOvershoot: authorFeedList.verticalOvershoot
                    bottomOvershootFun: () => getFeedListNextPage(feedListModelId)
                    topText: ""
                }

                BusyIndicator {
                    anchors.centerIn: parent
                    running: skywalker.getFeedInProgress
                }

                EmptyListIndication {
                    svg: svgOutline.noPosts
                    text: qsTr("No feeds")
                    list: authorFeedList
                }

                function refresh() {
                    getFeedList(feedListModelId)
                }

                function clear() {
                    model.clear()
                }
            }

            // Lists
            ListView {
                id: authorListList
                width: parent.width
                height: parent.height
                clip: true
                spacing: 0
                model: skywalker.getListListModel(listListModelId)
                flickDeceleration: guiSettings.flickDeceleration
                ScrollIndicator.vertical: ScrollIndicator {}
                interactive: !authorFeedView.interactive

                onVerticalOvershootChanged: {
                    if (verticalOvershoot < 0)
                        authorFeedView.interactive = true
                }

                onCountChanged: hasLists = count

                delegate: ListViewDelegate {
                    width: authorFeedView.width
                    ownLists: false
                    allowEdit: false

                    onBlockList: (list) => graphUtils.blockList(list.uri)
                    onUnblockList: (list, blockedUri) => graphUtils.unblockList(list.uri, blockedUri)
                    onMuteList: (list) => graphUtils.muteList(list.uri)
                    onUnmuteList: (list) => graphUtils.unmuteList(list.uri)
                }

                FlickableRefresher {
                    inProgress: skywalker.getListListInProgress
                    verticalOvershoot: authorListList.verticalOvershoot
                    bottomOvershootFun: () => getListListNextPage(listListModelId)
                    topText: ""
                }

                BusyIndicator {
                    anchors.centerIn: parent
                    running: skywalker.getListListInProgress
                }

                EmptyListIndication {
                    svg: svgOutline.noLists
                    text: qsTr("No lists")
                    list: authorListList
                }

                function refresh() {
                    getListList(listListModelId)
                }

                function clear() {
                    model.clear()
                }
            }

            function feedOk(modelId) {
                for (let i = 0; i < children.length; ++i) {
                    let c = children[i]

                    if (c instanceof AuthorPostsList && c.modelId === modelId) {
                        c.feedOk()
                    }
                }
            }

            function retryGetFeed(modelId) {
                for (let i = 0; i < children.length; ++i) {
                    let c = children[i]

                    if (c instanceof AuthorPostsList && c.modelId === modelId) {
                        return c.retryGetFeed()
                    }
                }

                return false
            }

            function refresh() {
                authorFeedView.headerItem.getFeedMenuBar().setCurrentIndex(0)

                for (let i = 0; i < children.length; ++i) {
                    let c = children[i]

                    if (c instanceof AuthorPostsList && c.modelId !== page.modelId)
                        c.removeModel()
                    else
                        children[i].refresh()
                }
            }

            function clear() {
                for (let i = 0; i < children.length; ++i) {
                    children[i].clear()
                }
            }
        }

        function feedOk(modelId) {
            itemAtIndex(0).feedOk(modelId)
        }

        function retryGetFeed(modelId, error, msg) {
            // TODO: define error strings in a central place
            if (error === "BlockedActor" && itemAtIndex(0).retryGetFeed(modelId))
                return

            statusPopup.show(msg, QEnums.STATUS_LEVEL_ERROR)
        }

        function refresh() {
            itemAtIndex(0).refresh()
        }

        function clear() {
            itemAtIndex(0).clear()
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
                       authorFeedView.clear()
                   }

        onBlockFailed: (error) => { statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR) }

        onUnblockOk: {
            blocking = ""
            authorFeedView.refresh()
        }

        onUnblockFailed: (error) => { statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR) }

        onMuteOk: {
            authorMuted = true
            authorFeedView.clear()
        }

        onMuteFailed: (error) => { statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR) }

        onUnmuteOk: {
            authorMuted = false
            authorFeedView.refresh()
        }

        onUnmuteFailed: (error) => { statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR) }

        onBlockListFailed: (error) => statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
        onUnblockListFailed: (error) => statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
        onMuteListFailed: (error) => statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
        onUnmuteListFailed: (error) => statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)

        onMuteRepostsOk: {
            authorMutedReposts = graphUtils.areRepostsMuted(author.did)
            authorFeedView.refresh()
        }
        onMuteRepostsFailed: (error) => statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)

        onUnmuteRepostsOk: {
            authorMutedReposts = graphUtils.areRepostsMuted(author.did)
            authorFeedView.refresh()
        }
        onUnmuteRepostsFailed: (error) => statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
    }

    UnicodeFonts {
        id: unicodeFonts
    }

    GuiSettings {
        id: guiSettings
    }

    function editAuthor(author) {
        let component = Qt.createComponent("EditProfile.qml")
        let editPage = component.createObject(page, {
                skywalker: skywalker,
                authorDid: author.did,
                authorName: authorName,
                authorDescription: authorDescription,
                authorAvatar: authorAvatar,
                authorBanner: authorBanner
            })
        editPage.profileUpdated.connect((name, description, avatar, banner) => {
            statusPopup.show(qsTr("Profile updated."), QEnums.STATUS_LEVEL_INFO, 2)
            authorName = name
            authorDescription = description
            authorAvatar = avatar
            setAuthorBanner(banner)

            // NOTE: if avatar is an "image://" source, then the profile takes ownership
            skywalker.updateUserProfile(name, description, avatar)

            root.popStack()
        })
        editPage.onClosed.connect(() => { root.popStack() })
        root.pushStack(editPage)
    }

    function setAuthorBanner(source) {
        if (source === authorBanner)
            return

        if (authorBanner.startsWith("image://"))
            postUtils.dropPhoto(authorBanner)

        authorBanner = source
    }

    function mentionPost() {
        const mentionText = (isUser(author) || author.hasInvalidHandle()) ? "" : `@${author.handle} `
        root.composePost(mentionText)
    }

    function feedOkHandler(modelId) {
        authorFeedView.feedOk(modelId)
    }

    function feedFailedHandler(modelId, error, msg) {
        authorFeedView.retryGetFeed(modelId, error, msg)
    }

    function getFeed(modelId) {
        if (mustGetFeed())
            skywalker.getAuthorFeed(modelId, 100)
    }

    function getFeedNextPage(modelId) {
        if (mustGetFeed())
            skywalker.getAuthorFeedNextPage(modelId)
    }

    function getFeedList(modelId) {
        if (mustGetFeed())
            skywalker.getAuthorFeedList(author.did, modelId)
    }

    function getFeedListNextPage(modelId) {
        if (mustGetFeed())
            skywalker.getAuthorFeedListNextPage(author.did, modelId)
    }

    function getListList(modelId) {
        if (mustGetFeed())
            skywalker.getListList(modelId)
    }

    function getListListNextPage(modelId) {
        if (mustGetFeed())
            skywalker.getListListNextPage(modelId)
    }

    function updateLists() {
        let listModelId = skywalker.createListListModel(QEnums.LIST_TYPE_ALL, QEnums.LIST_PURPOSE_UNKNOWN, skywalker.getUserDid())
        let listModel = skywalker.getListListModel(listModelId)
        listModel.setMemberCheckDid(author.did)
        listModel.setExcludeInternalLists(true)
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

    function visibilityShowProfileLink(list) {
        return list.count === 0 && !blocking && !author.viewer.blockedBy &&
                !authorMuted && contentVisibilityIsWarning()
    }

    function disableWarning() {
        showWarnedMedia = true
        authorFeedView.refresh()
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
            const listName = unicodeFonts.toCleanedHtml(author.viewer.blockingByList.name)
            return qsTr(`Blocked by list: <a href="${author.viewer.blockingByList.uri}" style="color: ${guiSettings.linkColor}">${listName}</a>`)
        } else if (blocking) {
            return qsTr("You blocked this account")
        } else if (author.viewer.blockedBy) {
            return qsTr("You are blocked")
        } else if (!author.viewer.mutedByList.isNull()) {
            const listName = unicodeFonts.toCleanedHtml(author.viewer.mutedByList.name)
            return qsTr(`Muted by list: <a href="${author.viewer.mutedByList.uri}" style="color: ${guiSettings.linkColor}">${listName}</a>`)
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

    Component.onDestruction: {
        skywalker.onAuthorFeedError.disconnect(feedFailedHandler)
        skywalker.onAuthorFeedOk.disconnect(feedOkHandler)

        setAuthorBanner("")
        skywalker.removeFeedListModel(feedListModelId)
        skywalker.removeListListModel(listListModelId)
    }

    Component.onCompleted: {
        skywalker.onAuthorFeedError.connect(feedFailedHandler)
        skywalker.onAuthorFeedOk.connect(feedOkHandler)

        authorName = author.displayName
        authorDescription = author.description
        authorAvatar = author.avatarUrl
        authorBanner = author.banner
        authorMutedReposts = graphUtils.areRepostsMuted(author.did)
        contentVisibility = skywalker.getContentVisibility(author.labels)
        contentWarning = skywalker.getContentWarning(author.labels)
        getFeed(modelId)
        getFeedList(feedListModelId)
        getListList(listListModelId)
    }
}
