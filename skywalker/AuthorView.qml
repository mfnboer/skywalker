import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker
import atproto.lib

SkyPage {
    required property var skywalker
    required property var rootProfileUtils
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
    readonly property bool hasFeeds: author.associated.feeds > 0
    readonly property int feedListModelId: skywalker.createFeedListModel()
    readonly property bool hasLists: author.associated.lists > 0
    readonly property int listListModelId: skywalker.createListListModel(QEnums.LIST_TYPE_ALL, QEnums.LIST_PURPOSE_UNKNOWN, author.did)
    readonly property bool hasStarterPacks: author.associated.starterPacks > 0
    readonly property int starterPackListModelId: skywalker.createStarterPackListModel()
    readonly property bool isLabeler: author.associated.isLabeler
    property int contentGroupListModelId: -1
    property var contentGroupListModel: contentGroupListModelId > -1 ? skywalker.getContentGroupListModel(contentGroupListModelId) : null
    property bool isSubscribed: contentGroupListModel ? contentGroupListModel.subscribed : false
    property labelerviewdetailed labeler
    property string labelerLikeUri: ""
    property int labelerLikeCount: 0
    property bool labelerLikeTransient: false
    property string firstAppearanceDate: "unknown"

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
            accessibleName: qsTr("go back")
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
                author: page.author
                showWarnedMedia: page.showWarnedMedia
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
        maximumFlickVelocity: guiSettings.maxFlickVelocity
        pixelAligned: guiSettings.flickPixelAligned

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
                    accessibleName: qsTr("edit your profile")
                    visible: isUser(author)
                }

                SvgButton {
                    id: moreButton
                    svg: svgOutline.moreVert
                    accessibleName: qsTr("more options")
                    onClicked: moreMenu.open()

                    Menu {
                        id: moreMenu
                        modal: true

                        onAboutToShow: root.enablePopupShield(true)
                        onAboutToHide: root.enablePopupShield(false)

                        CloseMenuItem {
                            text: qsTr("<b>Account</b>")
                            Accessible.name: qsTr("close more options menu")
                        }
                        AccessibleMenuItem {
                            text: qsTr("Translate")
                            visible: authorDescription
                            onTriggered: root.translateText(authorDescription)

                            MenuItemSvg { svg: svgOutline.googleTranslate }
                        }
                        AccessibleMenuItem {
                            text: qsTr("Share")
                            onTriggered: skywalker.shareAuthor(author)

                            MenuItemSvg { svg: svgOutline.share }
                        }
                        AccessibleMenuItem {
                            text: following ? qsTr("Unfollow") : qsTr("Follow")
                            visible: isLabeler && !isUser(author) && contentVisible()
                            onClicked: {
                                if (following)
                                    graphUtils.unfollow(author.did, following)
                                else
                                    graphUtils.follow(author)
                            }

                            MenuItemSvg { svg: following ? svgOutline.noUsers : svgOutline.addUser }
                        }
                        AccessibleMenuItem {
                            text: qsTr("Search")
                            onTriggered: root.viewSearchView("", author.handle)

                            MenuItemSvg { svg: svgOutline.search }
                        }
                        AccessibleMenuItem {
                            text: authorMuted ? qsTr("Unmute account") : qsTr("Mute account")
                            visible: !isUser(author) && author.viewer.mutedByList.isNull()
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
                            visible: !isUser(author) && author.viewer.blockingByList.isNull()
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
                           visible: !isUser(author)
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
                            visible: !isUser(author)
                            onTriggered: root.reportAuthor(author)

                            MenuItemSvg { svg: svgOutline.report }
                        }
                    }
                }

                SvgButton {
                    svg: svgOutline.directMessage
                    accessibleName: qsTr(`direct message ${author.name}`)
                    onClicked: skywalker.chat.startConvoForMember(author.did)
                    visible: author.canSendDirectMessage() && !isUser(author) && !skywalker.chat.messageConvoOpen()
                }

                SkyButton {
                    text: qsTr("Follow")
                    visible: !following && !isUser(author) && contentVisible() && !isLabeler
                    onClicked: graphUtils.follow(author)
                    Accessible.name: qsTr(`press to follow ${author.name}`)
                }
                SkyButton {
                    flat: true
                    text: qsTr("Following")
                    visible: following && !isUser(author) && contentVisible() && !isLabeler
                    onClicked: graphUtils.unfollow(author.did, following)
                    Accessible.name: qsTr(`press to unfollow ${author.name}`)
                }

                SkyButton {
                    text: qsTr("Subscribe")
                    visible: !isSubscribed && isLabeler && !author.isFixedLabeler()
                    onClicked: contentGroupListModel.subscribed = true
                    Accessible.name: qsTr(`press to subscribe to labeler ${author.name}`)
                }
                SkyButton {
                    flat: true
                    text: qsTr("Unsubscribe")
                    visible: isSubscribed && isLabeler && !author.isFixedLabeler()
                    onClicked: contentGroupListModel.subscribed = false
                    Accessible.name: qsTr(`press to unsubscribe from labeler ${author.name}`)
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

                StatAuthors {
                    atUri: author.did
                    count: author.followersCount
                    nameSingular: qsTr("follower")
                    namePlural: qsTr("followers")
                    authorListType: QEnums.AUTHOR_LIST_FOLLOWERS
                    authorListHeader: qsTr("Followers")
                }
                StatAuthors {
                    atUri: author.did
                    count: author.followsCount
                    nameSingular: qsTr("following")
                    namePlural: qsTr("following")
                    authorListType: QEnums.AUTHOR_LIST_FOLLOWS
                    authorListHeader: qsTr("Following")
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

            AccessibleText {
                id: firstAppearanceText
                width: parent.width - (parent.leftPadding + parent.rightPadding)
                topPadding: 10
                color: guiSettings.textColor
                text: qsTr(`🗓 First appearance: ${firstAppearanceDate}`)
                visible: contentVisible()
            }

            RowLayout {
                id: knownOthersRow
                width: parent.width - (parent.leftPadding + parent.rightPadding)
                spacing: 10

                Row {
                    id: avatarRow
                    topPadding: 10
                    spacing: -20

                    Repeater {
                        model: author.viewer.knownFollowers.followers

                        Avatar {
                            required property int index
                            required property basicprofile modelData

                            z: 5 - index
                            width: 34
                            height: width
                            author: modelData
                            onClicked: knownOthersRow.showKnownFollowers()
                        }
                    }
                }

                SkyCleanedText {
                    id: knownFollowersText
                    Layout.fillWidth: true
                    topPadding: 10
                    elide: Text.ElideRight
                    wrapMode: Text.Wrap
                    textFormat: Text.RichText
                    maximumLineCount: 3
                    color: guiSettings.linkColor
                    plainText: qsTr(`Followed by ${getKnownFollowersText()}`)

                    MouseArea {
                        anchors.fill: parent
                        onClicked: knownOthersRow.showKnownFollowers()
                    }
                }

                visible: author.viewer.knownFollowers.count > 0

                function showKnownFollowers() {
                    let modelId = root.getSkywalker().createAuthorListModel(QEnums.AUTHOR_LIST_KNOWN_FOLLOWERS, author.did)
                    root.viewAuthorList(modelId, qsTr(`Followers you follow`));
                }
            }

            Rectangle {
                width: parent.width - (parent.leftPadding + parent.rightPadding)
                height: labelerContentLabels.height
                color: "transparent"
                visible: isLabeler

                ContentLabels {
                    id: labelerContentLabels
                    anchors.left: parent.left
                    anchors.right: undefined
                    contentLabels: labeler.contentLabels
                    contentAuthorDid: author.did
                }
            }

            Row {
                // height: likeIcon.height
                width: parent.width - (parent.leftPadding + parent.rightPadding)
                topPadding: 10
                spacing: 10
                visible: isLabeler

                StatIcon {
                    id: likeIcon
                    iconColor: labelerLikeUri ? guiSettings.likeColor : guiSettings.statsColor
                    svg: labelerLikeUri ? svgFilled.like : svgOutline.like
                    onClicked: likeLabeler(labelerLikeUri, labeler.uri, labeler.cid)
                    Accessible.name: qsTr("like") + accessibilityUtils.statSpeech(labelerLikeCount, qsTr("like"), qsTr("likes"))

                    BlinkingOpacity {
                        target: likeIcon
                        running: labelerLikeTransient
                    }
                }

                StatAuthors {
                    atUri: labeler.uri
                    count: labelerLikeCount
                    nameSingular: qsTr("like")
                    namePlural: qsTr("likes")
                    authorListType: QEnums.AUTHOR_LIST_LIKES
                    authorListHeader: qsTr("Liked by")
                }
            }

            TabBar {
                id: feedMenuBar
                width: parent.width - (parent.leftPadding + parent.rightPadding)
                currentIndex: isLabeler ? 0 : 1

                AccessibleTabButton {
                    text: qsTr("Labels")
                    visible: isLabeler
                    width: isLabeler ? implicitWidth : 0
                }
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
                    text: qsTr("Starter packs")
                    visible: hasStarterPacks
                    width: hasStarterPacks ? implicitWidth : 0
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

            currentIndex: authorFeedView.headerItem ? authorFeedView.headerItem.getFeedMenuBar().currentIndex : 1

            // Labels
            ListView {
                id: labelList
                width: parent.width
                height: parent.height
                topMargin: 10
                clip: true
                spacing: 0
                model: contentGroupListModel
                flickDeceleration: guiSettings.flickDeceleration
                maximumFlickVelocity: guiSettings.maxFlickVelocity
                pixelAligned: guiSettings.flickPixelAligned
                ScrollIndicator.vertical: ScrollIndicator {}
                interactive: !authorFeedView.interactive

                onVerticalOvershootChanged: {
                    if (verticalOvershoot < 0)
                        authorFeedView.interactive = true
                }

                header: ColumnLayout {
                    width: authorFeedView.width

                    AccessibleText {
                        Layout.leftMargin: 10
                        Layout.rightMargin: 10
                        Layout.fillWidth: true
                        wrapMode: Text.Wrap
                        color: guiSettings.textColor
                        text: qsTr("Labels are annotations on users and content. They can be used to warn, hide and categorize content.")
                    }

                    AccessibleText {
                        Layout.leftMargin: 10
                        Layout.rightMargin: 10
                        Layout.fillWidth: true
                        height: page.isSubscribed ? implicitHeight : 0
                        wrapMode: Text.Wrap
                        color: guiSettings.textColor
                        text: qsTr(`Subscribe to ${author.handle} to use these labels`)
                        visible: !page.isSubscribed
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.topMargin: 10
                        height: 1
                        color: guiSettings.separatorColor
                    }
                }

                delegate: ContentGroupDelegate {
                    width: authorFeedView.width
                    isSubscribed: page.isSubscribed
                    adultContent: labelList.model.adultContent
                }

                FlickableRefresher {}

                EmptyListIndication {
                    svg: svgOutline.noLabels
                    text: qsTr("No labels")
                    list: labelList
                }

                function refresh() {}
                function clear() {}
            }

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
                maximumFlickVelocity: guiSettings.maxFlickVelocity
                pixelAligned: guiSettings.flickPixelAligned
                ScrollIndicator.vertical: ScrollIndicator {}
                interactive: !authorFeedView.interactive

                onVerticalOvershootChanged: {
                    if (verticalOvershoot < 0)
                        authorFeedView.interactive = true
                }

                delegate: GeneratorViewDelegate {
                    width: authorFeedView.width
                }

                FlickableRefresher {
                    inProgress: skywalker.getFeedInProgress
                    bottomOvershootFun: () => getFeedListNextPage(feedListModelId)
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

            // Starter packs
            ListView {
                id: authorStarterPackList
                width: parent.width
                height: parent.height
                clip: true
                spacing: 0
                model: skywalker.getStarterPackListModel(starterPackListModelId)
                flickDeceleration: guiSettings.flickDeceleration
                maximumFlickVelocity: guiSettings.maxFlickVelocity
                pixelAligned: guiSettings.flickPixelAligned
                ScrollIndicator.vertical: ScrollIndicator {}
                interactive: !authorFeedView.interactive

                onVerticalOvershootChanged: {
                    if (verticalOvershoot < 0)
                        authorFeedView.interactive = true
                }

                delegate: StarterPackViewDelegate {
                    width: authorFeedView.width
                }

                FlickableRefresher {
                    inProgress: skywalker.getStarterPackListInProgress
                    bottomOvershootFun: () => getStarterPackListNextPage(starterPackListModelId)
                }

                BusyIndicator {
                    anchors.centerIn: parent
                    running: skywalker.getStarterPackListInProgress
                }

                EmptyListIndication {
                    svg: svgOutline.noLists
                    text: qsTr("No starter packs")
                    list: authorStarterPackList
                }

                function refresh() {
                    getStarterPackList(starterPackListModelId)
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
                maximumFlickVelocity: guiSettings.maxFlickVelocity
                pixelAligned: guiSettings.flickPixelAligned
                ScrollIndicator.vertical: ScrollIndicator {}
                interactive: !authorFeedView.interactive

                onVerticalOvershootChanged: {
                    if (verticalOvershoot < 0)
                        authorFeedView.interactive = true
                }

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
                    bottomOvershootFun: () => getListListNextPage(listListModelId)
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
                authorFeedView.headerItem.getFeedMenuBar().setCurrentIndex(1)

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
            if (error === ATProtoErrorMsg.BLOCKED_ACTOR && itemAtIndex(0).retryGetFeed(modelId))
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

    ProfileUtils {
        id: profileUtils
        skywalker: page.skywalker

        onGetLabelerViewDetailedOk: (view) => setLabeler(view)
        onGetLabelerViewDetailedFailed: (error) => statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)

        onLikeLabelerOk: (likeUri) => {
            labelerLikeUri = likeUri
            ++labelerLikeCount
            labelerLikeTransient = false
        }

        onLikeLabelerFailed: (error) => {
            labelerLikeTransient = false
            statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
        }

        onUndoLikeLabelerOk: {
            labelerLikeUri = ""
            --labelerLikeCount
            labelerLikeTransient = false
        }

        onUndoLikeLabelerFailed: (error) => {
            labelerLikeTransient = false
            statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
        }

        onFirstAppearanceOk: (did, appearance) => {
            firstAppearanceDate = appearance.toLocaleDateString(Qt.locale(), Locale.ShortFormat)
        }
    }

    UnicodeFonts {
        id: unicodeFonts
    }

    AccessibilityUtils {
        id: accessibilityUtils
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

    function mentionPost(text = "", imgSource = "") {
        const mentionText = (isUser(author) || author.hasInvalidHandle()) ? text : `@${author.handle} ${text}`
        root.composePost(mentionText, imgSource)
    }

    function mentionVideoPost(text = "", videoSource = "") {
        const mentionText = (isUser(author) || author.hasInvalidHandle()) ? text : `@${author.handle} ${text}`
        root.composeVideoPost(mentionText, videoSource)
    }

    function feedOkHandler(modelId) {
        authorFeedView.feedOk(modelId)
    }

    function feedFailedHandler(modelId, error, msg) {
        authorFeedView.retryGetFeed(modelId, error, msg)
    }

    function pinnedPostHandler(postUri, postCid) {
        let model = skywalker.getAuthorFeedModel(modelId)
        model.setPinnedPost(postUri)
        getFeed(modelId)
    }

    function unpinnedPostHandler() {
        let model = skywalker.getAuthorFeedModel(modelId)
        model.removePinnedPost()
        getFeed(modelId)
    }

    function getFeed(modelId) {
        if (mustGetFeed())
            skywalker.getAuthorFeed(modelId, 50)
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

    function getStarterPackList(modelId) {
        if (mustGetFeed())
            skywalker.getAuthorStarterPackList(author.did, modelId)
    }

    function getStarterPackListNextPage(modelId) {
        if (mustGetFeed())
            skywalker.getAuthorStarterPackListNextPage(author.did, modelId)
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
        return showWarnedMedia || guiSettings.contentVisible(author)
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

    function setLabeler(view) {
        contentGroupListModelId = skywalker.createContentGroupListModel(author.did, view.policies)
        labeler = view
        labelerLikeUri = labeler.viewer.like
        labelerLikeCount = labeler.likeCount
    }

    function likeLabeler(likeUri, uri, cid) {
        labelerLikeTransient = true

        if (likeUri)
            profileUtils.undoLikeLabeler(likeUri)
        else
            profileUtils.likeLabeler(uri, cid)
    }

    function getKnownFollowersText() {
        const knownFollowers = author.viewer.knownFollowers

        if (knownFollowers.count <= 0)
            return ""

        if (knownFollowers.followers.length === 0)
            return knownFollowers.count > 1 ? qsTr(`${knownFollowers.count} others you follow`) : qsTr("1 other you follow")

        let followersText = unicodeFonts.toCleanedHtml(knownFollowers.followers[0].name)

        if (knownFollowers.followers.length > 1)
            followersText = `${followersText}, ${unicodeFonts.toCleanedHtml(knownFollowers.followers[1].name)}`

        if (knownFollowers.count > 3)
            followersText = qsTr(`${followersText} and ${(knownFollowers.count - 2)} others`)
        else if (knownFollowers.count > 2)
            followersText = qsTr(`${followersText} and 1 other`)

        return followersText
    }

    function isUser(author) {
        return skywalker.getUserDid() === author.did
    }

    Component.onDestruction: {
        skywalker.onAuthorFeedError.disconnect(feedFailedHandler)
        skywalker.onAuthorFeedOk.disconnect(feedOkHandler)

        if (isUser(author)) {
            rootProfileUtils.onSetPinnedPostOk.disconnect(pinnedPostHandler)
            rootProfileUtils.onClearPinnedPostOk.disconnect(unpinnedPostHandler)
        }

        setAuthorBanner("")
        skywalker.removeFeedListModel(feedListModelId)
        skywalker.removeListListModel(listListModelId)
        skywalker.removeStarterPackListModel(starterPackListModelId)

        if (contentGroupListModelId > -1) {
            skywalker.getContentFilter().saveLabelIdsToSettings(author.did)
            skywalker.saveContentFilterPreferences(contentGroupListModel)
            skywalker.removeContentGroupListModel(contentGroupListModelId)
        }
    }

    Component.onCompleted: {
        skywalker.onAuthorFeedError.connect(feedFailedHandler)
        skywalker.onAuthorFeedOk.connect(feedOkHandler)

        if (isUser(author)) {
            rootProfileUtils.onSetPinnedPostOk.connect(pinnedPostHandler)
            rootProfileUtils.onClearPinnedPostOk.connect(unpinnedPostHandler)
        }

        authorName = author.name
        authorDescription = author.description
        authorAvatar = author.avatarUrl
        authorBanner = author.banner
        authorMutedReposts = graphUtils.areRepostsMuted(author.did)
        contentVisibility = skywalker.getContentVisibility(author.labels)
        contentWarning = skywalker.getContentWarning(author.labels)
        getFeed(modelId)
        profileUtils.getFirstAppearance(author.did)

        if (hasFeeds)
            getFeedList(feedListModelId)

        if (hasLists)
            getListList(listListModelId)

        if (hasStarterPacks)
            getStarterPackList(starterPackListModelId)

        if (isLabeler)
            profileUtils.getLabelerViewDetailed(author.did)
    }
}
