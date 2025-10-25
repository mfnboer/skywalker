import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker
import atproto.lib

SkyPage {
    required property string userDid
    property Skywalker skywalker: root.getSkywalker(userDid)
    required property detailedprofile author
    required property int modelId
    readonly property string sideBarTitle: author.name
    readonly property alias sideBarAuthor: page.author

    // Initialize properties in onCompleted.
    // For some weird reason description and banner are not available here.
    // Must have something to do with those properties being in subclasses of BasicProfile.
    property string authorName
    property string authorPronouns
    property string authorDescription
    property string authorWebsite
    property string authorAvatar
    property string authorBanner
    property bool authorVerified: author.verificationState.verifiedStatus === QEnums.VERIFIED_STATUS_VALID
    property bool isTrustedVerifier: author.verificationState.trustedVerifierStatus === QEnums.VERIFIED_STATUS_VALID
    property string following: author.viewer.following
    property string blocking: author.viewer.blocking
    property activitysubscription activitySubscription: author.viewer.activitySubscription
    property bool authorMuted: author.viewer.muted
    property bool authorMutedReposts: false
    property bool authorHideFromTimeline: false
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
    readonly property bool isSubscribed: contentGroupListModel ? contentGroupListModel.subscribed : false
    readonly property bool isFixedLabelerEnabled: contentGroupListModel ? contentGroupListModel.fixedLabelerEnabled : false
    property labelerviewdetailed labeler
    property string labelerLikeUri: ""
    property int labelerLikeCount: 0
    property bool labelerLikeTransient: false
    property string firstAppearanceDate: "unknown"
    property bool feedInitialized: false

    signal closed

    id: page

    Accessible.role: Accessible.Pane
    Accessible.name: qsTr(`${author.name}\n\n@${author.handle}`)

    header: DeadHeaderMargin {}

    footer: DeadFooterMargin {
        PostButton {
            y: -height - 10
            svg: authorCanBeMentioned() ? SvgOutline.atSign : SvgOutline.chat
            overrideOnClicked: () => mentionPost()
            visible: !root.showSideBar

            Accessible.role: Accessible.Button
            Accessible.name: qsTr(`post and mention ${author.name}`)
            Accessible.onPressAction: clicked()
        }
    }

    onCover: {
        let feeds = authorFeedView.itemAtIndex(0)

        if (!(feeds instanceof SwipeView)) {
            qWarning() << "Wrong type:" << feeds
            return
        }

        let item = feeds.itemAt(feeds.currentIndex) // qmllint disable missing-property

        if (item instanceof AuthorPostsList) {
            item.cover()
        }
        else if (item instanceof Loader) {
            let loaderItem = item.item

            if (loaderItem && loaderItem instanceof AuthorPostsList)
                loaderItem.cover()
        }
    }

    ListView {
        id: authorFeedView
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
            if (contentY > -headerItem.getFeedMenuBarHeight()) { // qmllint disable missing-property
                contentY = -headerItem.getFeedMenuBarHeight()
                interactive = false
            }
            else if (!interactive) {
                // When a post thread is opened, Qt changes contentY??
                contentY = -headerItem.getFeedMenuBarHeight()
            }
        }

        header: Column {
            id: viewHeader
            width: parent.width
            leftPadding: 10
            rightPadding: 10

            Accessible.role: Accessible.StaticText
            Accessible.name: qsTr(`${author.name}\n\n@${author.handle}\n\n${author.description}`)

            Rectangle {
                x: -viewHeader.leftPadding
                width: page.width
                height: bannerImg.visible ? bannerImg.height : noBanner.height

                ImageAutoRetry {
                    id: bannerImg
                    anchors.top: parent.top
                    width: parent.width
                    source: authorBanner
                    fillMode: Image.PreserveAspectFit
                    indicateLoading: false
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
                    iconColor: "white"
                    Material.background: "black"
                    opacity: 0.5
                    svg: SvgOutline.arrowBack
                    accessibleName: qsTr("go back")
                    visible: !root.showSideBar
                    onClicked: page.closed()
                }

                Loader {
                    anchors.top: parent.top
                    anchors.topMargin: 10
                    anchors.right: parent.right
                    anchors.rightMargin: 10
                    height: 40
                    width: height
                    active: !root.isActiveUser(userDid) && !root.showSideBar

                    sourceComponent: CurrentUserAvatar {
                        userDid: page.userDid
                    }
                }

                Rectangle {
                    id: avatar
                    x: parent.width - width - 10
                    y: parent.height - height / 2
                    width: 104
                    height: width
                    radius: width / 2
                    color: guiSettings.backgroundColor

                    Avatar {
                        id: avatarImg
                        anchors.centerIn: parent
                        width: parent.width - 4
                        userDid: page.userDid
                        author: page.author
                        showWarnedMedia: page.showWarnedMedia
                        onClicked:  {
                            if (authorAvatar) {
                                fullImageLoader.show(0)
                                avatarImg.visible = false
                            }
                        }
                    }
                }

                FullImageViewLoader {
                    id: fullImageLoader
                    thumbImageViewList: [avatarImg.getImage()]
                    images: [author.imageView]
                    onFinished: avatarImg.visible = true
                }
            }

            RowLayout {
                height: 50
                spacing: 0

                SvgButton {
                    Layout.preferredWidth: 40
                    Layout.preferredHeight: width
                    svg: SvgOutline.edit
                    onClicked: editAuthor(author)
                    accessibleName: qsTr("edit your profile")
                    visible: page.isUser(author)
                }

                SvgButton {
                    id: moreButton
                    Layout.preferredWidth: 40
                    Layout.preferredHeight: width
                    svg: SvgOutline.moreVert
                    accessibleName: qsTr("more options")
                    onClicked: moreMenuLoader.open()

                    Loader {
                        id: moreMenuLoader
                        active: false

                        function open() {
                            active = true
                        }

                        onStatusChanged: {
                            if (status == Loader.Ready)
                                item.open() // qmllint disable missing-property
                        }

                        sourceComponent: SkyMenu {
                            id: moreMenu
                            onAboutToShow: root.enablePopupShield(true)
                            onAboutToHide: { root.enablePopupShield(false); parent.active = false }

                            CloseMenuItem {
                                text: qsTr("<b>Account</b>")
                                Accessible.name: qsTr("close more options menu")
                            }
                            AccessibleMenuItem {
                                text: qsTr("Translate")
                                visible: authorDescription
                                onTriggered: root.translateText(authorDescription)

                                MenuItemSvg { svg: SvgOutline.googleTranslate }
                            }
                            AccessibleMenuItem {
                                text: qsTr("Share")
                                onTriggered: skywalker.shareAuthor(author)

                                MenuItemSvg { svg: SvgOutline.share }
                            }
                            AccessibleMenuItem {
                                text: following ? qsTr("Unfollow") : qsTr("Follow")
                                visible: isLabeler && !page.isUser(author) && contentVisible()
                                onClicked: {
                                    if (following)
                                        graphUtils.unfollow(author.did, following)
                                    else
                                        graphUtils.follow(author)
                                }

                                MenuItemSvg { svg: following ? SvgOutline.noUsers : SvgOutline.addUser }
                            }
                            AccessibleMenuItem {
                                text: qsTr("Search")
                                onTriggered: root.viewSearchView("", author.handle)

                                MenuItemSvg { svg: SvgOutline.search }
                            }
                            AccessibleMenuItem {
                                text: authorMuted ? qsTr("Unmute account") : qsTr("Mute account")
                                visible: !page.isUser(author) && author.viewer.mutedByList.isNull()
                                onTriggered: {
                                    if (authorMuted) {
                                        graphUtils.unmute(author.did)
                                    }
                                    else {
                                        let gu = graphUtils
                                        let did = author.did
                                        root.showBlockMuteDialog(false, author, (expiresAt) => gu.mute(did, expiresAt), page.userDid)
                                    }
                                }

                                MenuItemSvg { svg: authorMuted ? SvgOutline.unmute : SvgOutline.mute }
                            }
                            AccessibleMenuItem {
                                text: blocking ? qsTr("Unblock account") : qsTr("Block account")
                                visible: !page.isUser(author) && author.viewer.blockingByList.isNull()
                                onTriggered: {
                                    if (blocking) {
                                        graphUtils.unblock(author.did, blocking)
                                    }
                                    else {
                                        let gu = graphUtils
                                        let did = author.did
                                        root.showBlockMuteDialog(true, author, (expiresAt) => gu.block(did, expiresAt), page.userDid)
                                    }
                                }

                                MenuItemSvg { svg: blocking ? SvgOutline.unblock : SvgOutline.block }
                            }
                            AccessibleMenuItem {
                               text: authorMutedReposts ? qsTr("Unmute reposts") : qsTr("Mute reposts")
                               visible: !page.isUser(author)
                               onTriggered: {
                                   if (authorMutedReposts)
                                       graphUtils.unmuteReposts(author.did)
                                   else
                                       graphUtils.muteReposts(author)
                               }

                               MenuItemSvg { svg: SvgOutline.repost }
                            }

                            AccessibleMenuItem {
                                text: qsTr("Update lists")
                                onTriggered: updateLists()

                                MenuItemSvg { svg: SvgOutline.list }
                            }
                            AccessibleMenuItem {
                                text: qsTr("Report account")
                                visible: !page.isUser(author)
                                onTriggered: root.reportAuthor(author, userDid)

                                MenuItemSvg { svg: SvgOutline.report }
                            }
                            AccessibleMenuItem {
                                text: qsTr("Emoji names")
                                visible: UnicodeFonts.hasEmoji(authorDescription)
                                onTriggered: root.showEmojiNamesList(authorDescription)

                                MenuItemSvg { svg: SvgOutline.emojiLanguage }
                            }
                        }
                    }
                }

                SvgButton {
                    Layout.preferredWidth: 40
                    Layout.preferredHeight: width
                    svg: SvgOutline.notificationsAdd
                    visible: !page.isUser(author) && !activitySubscription.isSubscribed && author.allowsActivitySubscriptions()
                    onClicked: subscribeActivity()
                    Accessible.name: qsTr(`press to subscribe to posts from ${author.name}`)
                }
                SvgButton {
                    Layout.preferredWidth: 40
                    Layout.preferredHeight: width
                    svg: SvgFilled.notificationsActive
                    visible: !page.isUser(author) && activitySubscription.isSubscribed
                    onClicked: subscribeActivity()
                    Accessible.name: qsTr(`press to modify post subscription from ${author.name}`)
                }

                SvgButton {
                    Layout.preferredWidth: 40
                    Layout.preferredHeight: width
                    svg: SvgOutline.directMessage
                    accessibleName: qsTr(`direct message ${author.name}`)
                    onClicked: skywalker.chat.startConvoForMember(author.did)
                    visible: author.canSendDirectMessage() && !page.isUser(author) && skywalker.chat && !skywalker.chat.messageConvoOpen()
                }

                SkyButton {
                    Layout.preferredHeight: 40
                    text: qsTr("Follow")
                    visible: !following && !page.isUser(author) && contentVisible() && !isLabeler
                    onClicked: graphUtils.follow(author)
                    Accessible.name: qsTr(`press to follow ${author.name}`)
                }
                SkyButton {
                    Layout.preferredHeight: 40
                    flat: true
                    text: qsTr("Following")
                    visible: following && !page.isUser(author) && contentVisible() && !isLabeler
                    onClicked: graphUtils.unfollow(author.did, following)
                    Accessible.name: qsTr(`press to unfollow ${author.name}`)
                }

                SkyButton {
                    Layout.preferredHeight: 40
                    text: qsTr("Subscribe")
                    visible: !isSubscribed && isLabeler && !author.isFixedLabeler()
                    onClicked: contentGroupListModel.subscribed = true
                    Accessible.name: qsTr(`press to subscribe to labeler ${author.name}`)
                }
                SkyButton {
                    Layout.preferredHeight: 40
                    flat: true
                    text: qsTr("Unsubscribe")
                    visible: isSubscribed && isLabeler && !author.isFixedLabeler()
                    onClicked: contentGroupListModel.subscribed = false
                    Accessible.name: qsTr(`press to unsubscribe from labeler ${author.name}`)
                }

                SkyButton {
                    Layout.preferredHeight: 40
                    text: qsTr("Subscribe")
                    visible: !isFixedLabelerEnabled && isLabeler && author.isFixedLabeler()
                    onClicked: contentGroupListModel.fixedLabelerEnabled = true
                    Accessible.name: qsTr(`press to subscribe to labeler ${author.name}`)
                }
                SkyButton {
                    Layout.preferredHeight: 40
                    flat: true
                    text: qsTr("Unsubscribe")
                    visible: isFixedLabelerEnabled && isLabeler && author.isFixedLabeler()
                    onClicked: contentGroupListModel.fixedLabelerEnabled = false
                    Accessible.name: qsTr(`press to unsubscribe from labeler ${author.name}`)
                }
            }

            AuthorNameAndStatusMultiLine {
                topPadding: author.actorStatus.isActive ? 10 : 0
                width: parent.width - (parent.leftPadding + parent.rightPadding)
                userDid: page.userDid
                author: page.author
                pointSize: guiSettings.scaledFont(12/8)
                maximumLineCount: 3
                wrapMode: Text.Wrap
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
                SkyLabel {
                    text: qsTr("hide from timeline")
                    visible: authorHideFromTimeline
                }
            }

            SkyCleanedText {
                width: parent.width - (parent.leftPadding + parent.rightPadding)
                topPadding: 5
                bottomPadding: 5
                plainText: `${authorPronouns}`
                visible: Boolean(authorPronouns)
            }

            Rectangle {
                width: parent.width - (parent.leftPadding + parent.rightPadding)
                height: contentLabels.height
                color: "transparent"

                ContentLabels {
                    id: contentLabels
                    anchors.left: parent.left
                    anchors.right: undefined
                    userDid: page.userDid
                    contentLabels: author.labels
                    contentAuthorDid: author.did
                }
            }

            // Live status
            Rectangle {
                width: parent.width - (parent.leftPadding + parent.rightPadding)
                height: visible ? liveView.y + liveView.height : 0
                color: "transparent"
                visible: author.actorStatus.isActive && !author.actorStatus.externalView.isNull() && contentVisible()

                LinkCardView {
                    id: liveView
                    y: 10
                    width: parent.width
                    uri: author.actorStatus.externalView.uri
                    title: author.actorStatus.externalView.title
                    description: author.actorStatus.externalView.description
                    thumbUrl: author.actorStatus.externalView.thumbUrl
                    contentVisibility: QEnums.CONTENT_VISIBILITY_SHOW
                    contentWarning: ""
                    isLiveExternal: true

                    LiveLabel {
                        x: 10
                        y: 10
                    }
                }
            }

            Row {
                id: statsRow
                width: parent.width - (parent.leftPadding + parent.rightPadding)
                spacing: 15
                topPadding: 10
                visible: contentVisible()

                StatAuthors {
                    userDid: page.userDid
                    atUri: author.did
                    count: author.followersCount
                    nameSingular: qsTr("follower")
                    namePlural: qsTr("followers")
                    authorListType: QEnums.AUTHOR_LIST_FOLLOWERS
                    authorListHeader: qsTr("Followers")
                }
                StatAuthors {
                    userDid: page.userDid
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
                    Accessible.name: UnicodeFonts.toPlainText(text)
                }
            }

            SkyCleanedText {
                id: descriptionText
                width: parent.width - (parent.leftPadding + parent.rightPadding)
                topPadding: 10
                wrapMode: Text.Wrap
                textFormat: Text.RichText
                color: guiSettings.textColor
                plainText: postUtils.linkiFy(authorDescription, guiSettings.linkColor)
                visible: contentVisible()

                LinkCatcher {
                    userDid: page.userDid
                }
            }

            AccessibleText {
                id: websiteText
                width: parent.width - (parent.leftPadding + parent.rightPadding)
                topPadding: 10
                textFormat: Text.RichText
                text: qsTr(`🌐 ${guiSettings.toHtmlLink(authorWebsite)}`)
                visible: contentVisible() && Boolean(authorWebsite)

                onLinkActivated: (link) => root.openLink(link, "", page.userDid)
            }

            AccessibleText {
                readonly property date lastActive: avatarImg.lastActive
                readonly property string statusIcon: avatarImg.isActive ? "🟢" : "🟡"

                width: parent.width - (parent.leftPadding + parent.rightPadding)
                topPadding: 10
                text: qsTr(`${statusIcon} Last activity: ${guiSettings.dateTimeIndication(lastActive)}`)
                visible: !isNaN(lastActive.getTime())
            }

            AccessibleText {
                id: firstAppearanceText
                width: parent.width - (parent.leftPadding + parent.rightPadding)
                topPadding: 10
                text: qsTr(`🗓 First appearance: ${firstAppearanceDate}`)
                visible: contentVisible()
            }

            Loader {
                active: author.viewer.knownFollowers.count > 0
                sourceComponent: KnownFollowers {
                    userDid: page.userDid
                    author: page.author
                    width: viewHeader.width - (viewHeader.leftPadding + viewHeader.rightPadding)
                }
            }

            Loader {
                active: isLabeler

                sourceComponent: Rectangle {
                    width: viewHeader.width - (viewHeader.leftPadding + viewHeader.rightPadding)
                    height: labelerContentLabels.height
                    color: "transparent"

                    ContentLabels {
                        id: labelerContentLabels
                        anchors.left: parent.left
                        anchors.right: undefined
                        userDid: page.userDid
                        contentLabels: labeler.contentLabels
                        contentAuthorDid: author.did
                    }
                }
            }

            Loader {
                active: isLabeler

                sourceComponent: Row {
                    width: viewHeader.width - (viewHeader.leftPadding + viewHeader.rightPadding)
                    topPadding: 10
                    spacing: 10

                    StatIcon {
                        id: likeIcon
                        iconColor: labelerLikeUri ? guiSettings.likeColor : guiSettings.statsColor
                        svg: labelerLikeUri ? SvgFilled.like : SvgOutline.like
                        onClicked: likeLabeler(labelerLikeUri, labeler.uri, labeler.cid)
                        Accessible.name: qsTr("like") + accessibilityUtils.statSpeech(labelerLikeCount, qsTr("like"), qsTr("likes"))

                        BlinkingOpacity {
                            target: likeIcon
                            running: labelerLikeTransient
                        }
                    }

                    StatAuthors {
                        userDid: page.userDid
                        atUri: labeler.uri
                        count: labelerLikeCount
                        nameSingular: qsTr("like")
                        namePlural: qsTr("likes")
                        authorListType: QEnums.AUTHOR_LIST_LIKES
                        authorListHeader: qsTr("Liked by")
                    }
                }
            }

            SkyTabBar {
                id: feedMenuBar
                width: parent.width - (parent.leftPadding + parent.rightPadding)
                currentIndex: 0

                AccessibleTabButton {
                    id: labelsTab
                    text: qsTr("Labels")
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
                    text: qsTr("Media gallery")
                    width: implicitWidth
                }
                AccessibleTabButton {
                    text: qsTr("Video")
                    width: implicitWidth
                }
                AccessibleTabButton {
                    text: qsTr("Video gallery")
                    width: implicitWidth
                }
                AccessibleTabButton {
                    id: likesTab
                    text: qsTr("Likes")
                }
                AccessibleTabButton {
                    id: feedsTab
                    text: qsTr("Feeds")
                }
                AccessibleTabButton {
                    id: starterPacksTab
                    text: qsTr("Starter packs")
                }
                AccessibleTabButton {
                    id: listsTab
                    text: qsTr("Lists")
                }

                Component.onCompleted: {
                    if (!isLabeler)
                        removeItem(labelsTab)

                    if (!page.isUser(author))
                        removeItem(likesTab)

                    if (!hasFeeds)
                        removeItem(feedsTab)

                    if (!hasStarterPacks)
                        removeItem(starterPacksTab)

                    if (!hasLists)
                        removeItem(listsTab)
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

        delegate: SwipeView {
            id: feedStack
            width: parent.width

            // -1 to make the interactive enable/disable work
            height: page.height - (authorFeedView.headerItem ? authorFeedView.headerItem.getFeedMenuBarHeight() - 1 : 0) - guiSettings.headerMargin

            currentIndex: authorFeedView.headerItem ? authorFeedView.headerItem.getFeedMenuBar().currentIndex : 0

            onCurrentIndexChanged: {
                if (authorFeedView.headerItem) {
                    console.debug("CURRENT:", currentIndex)
                    authorFeedView.headerItem.getFeedMenuBar().setCurrentIndex(currentIndex)
                }
            }

            // Labels
            Loader {
                id: labelsView
                active: isLabeler
                asynchronous: true

                sourceComponent: ListView {
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
                            Layout.preferredHeight: page.isSubscribed ? 0 : implicitHeight
                            wrapMode: Text.Wrap
                            color: guiSettings.textColor
                            text: qsTr(`Subscribe to ${author.handle} to use these labels`)
                            visible: !page.isSubscribed
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.topMargin: 10
                            Layout.preferredHeight: 1
                            color: guiSettings.separatorColor
                        }

                        EmptyListIndication {
                            svg: SvgOutline.noLabels
                            text: qsTr("No labels")
                            list: labelList
                        }
                    }

                    footer: DeadFooterMargin {}
                    footerPositioning: ListView.OverlayFooter

                    delegate: ContentGroupDelegate {
                        width: authorFeedView.width
                        userDid: page.userDid
                        isSubscribed: page.isSubscribed
                        adultContent: labelList.model.adultContent
                    }

                    FlickableRefresher {}

                    function refresh() {}
                    function clear() {}
                }
            }

            // Posts
            AuthorPostsList {
                id: authorPostsList
                userDid: page.userDid
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
            Loader {
                active: true
                asynchronous: true

                SwipeView.onIsCurrentItemChanged: {
                    if (item)
                        item.changeCurrentItem(SwipeView.isCurrentItem) // qmllint disable missing-property
                }

                sourceComponent: AuthorPostsList {
                    id: authorRepliesList
                    width: parent.width
                    height: parent.height
                    userDid: page.userDid
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
            }

            // Media
            Loader {
                active: true
                asynchronous: true

                SwipeView.onIsCurrentItemChanged: {
                    if (item)
                        item.changeCurrentItem(SwipeView.isCurrentItem) // qmllint disable missing-property
                }

                sourceComponent: AuthorPostsList {
                    id: authorMediaList
                    width: parent.width
                    height: parent.height
                    userDid: page.userDid
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
            }

            // Media gallery
            Loader {
                active: true
                asynchronous: true

                SwipeView.onIsCurrentItemChanged: {
                    if (item)
                        item.changeCurrentItem(SwipeView.isCurrentItem) // qmllint disable missing-property
                }

                sourceComponent: AuthorPostsList {
                    id: authorMediaGallery
                    width: parent.width
                    height: parent.height
                    userDid: page.userDid
                    author: page.author
                    enclosingView: authorFeedView
                    getFeed: (id) => page.getFeed(id)
                    getFeedNextPage: (id) => page.getFeedNextPage(id)
                    getEmptyListIndicationSvg: () => page.getEmptyListIndicationSvg()
                    getEmptyListIndicationText: () => page.getEmptyListIndicationText()
                    visibilityShowProfileLink: (list) => page.visibilityShowProfileLink(list)
                    disableWarning: () => page.disableWarning()
                    feedFilter: QEnums.AUTHOR_FEED_FILTER_MEDIA
                    galleryMode: true
                }
            }

            // Video
            Loader {
                active: true
                asynchronous: true

                SwipeView.onIsCurrentItemChanged: {
                    if (item)
                        item.changeCurrentItem(SwipeView.isCurrentItem) // qmllint disable missing-property
                }

                sourceComponent: AuthorPostsList {
                    id: authorVideoList
                    width: parent.width
                    height: parent.height
                    userDid: page.userDid
                    author: page.author
                    enclosingView: authorFeedView
                    getFeed: (id) => page.getFeed(id)
                    getFeedNextPage: (id) => page.getFeedNextPage(id)
                    getEmptyListIndicationSvg: () => page.getEmptyListIndicationSvg()
                    getEmptyListIndicationText: () => page.getEmptyListIndicationText()
                    visibilityShowProfileLink: (list) => page.visibilityShowProfileLink(list)
                    disableWarning: () => page.disableWarning()
                    feedFilter: QEnums.AUTHOR_FEED_FILTER_VIDEO
                }
            }

            // Video gallery
            Loader {
                active: true
                asynchronous: true

                SwipeView.onIsCurrentItemChanged: {
                    if (item)
                        item.changeCurrentItem(SwipeView.isCurrentItem) // qmllint disable missing-property
                }

                sourceComponent: AuthorPostsList {
                    id: authorVideoGallery
                    width: parent.width
                    height: parent.height
                    userDid: page.userDid
                    author: page.author
                    enclosingView: authorFeedView
                    getFeed: (id) => page.getFeed(id)
                    getFeedNextPage: (id) => page.getFeedNextPage(id)
                    getEmptyListIndicationSvg: () => page.getEmptyListIndicationSvg()
                    getEmptyListIndicationText: () => page.getEmptyListIndicationText()
                    visibilityShowProfileLink: (list) => page.visibilityShowProfileLink(list)
                    disableWarning: () => page.disableWarning()
                    feedFilter: QEnums.AUTHOR_FEED_FILTER_VIDEO
                    galleryMode: true
                }
            }

            // Likes
            Loader {
                id: likesView
                active: page.isUser(author)
                asynchronous: true

                SwipeView.onIsCurrentItemChanged: {
                    if (item)
                        item.changeCurrentItem(SwipeView.isCurrentItem) // qmllint disable missing-property
                }

                sourceComponent: AuthorPostsList {
                    id: authorLikesList
                    width: parent.width
                    height: parent.height
                    userDid: page.userDid
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
            }

            // Feeds
            Loader {
                id: feedsView
                active: hasFeeds
                asynchronous: true

                sourceComponent: ListView {
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

                    footer: DeadFooterMargin {}
                    footerPositioning: ListView.OverlayFooter

                    delegate: GeneratorViewDelegate {
                        width: authorFeedView.width
                        userDid: page.userDid

                        onHideFollowing: (feed, hide) => feedUtils.hideFollowing(feed.uri, hide)
                    }

                    FlickableRefresher {
                        inProgress: model.getFeedInProgress
                        bottomOvershootFun: () => getFeedListNextPage(feedListModelId)
                    }

                    BusyIndicator {
                        anchors.centerIn: parent
                        running: model.getFeedInProgress
                    }

                    EmptyListIndication {
                        svg: SvgOutline.noPosts
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
            }

            // Starter packs
            Loader {
                id: starterPacksView
                active: hasStarterPacks
                asynchronous: true

                sourceComponent: ListView {
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

                    footer: DeadFooterMargin {}
                    footerPositioning: ListView.OverlayFooter

                    delegate: StarterPackViewDelegate {
                        width: authorFeedView.width
                        userDid: page.userDid
                    }

                    FlickableRefresher {
                        inProgress: authorStarterPackList.model?.getFeedInProgress
                        bottomOvershootFun: () => getStarterPackListNextPage(starterPackListModelId)
                    }

                    BusyIndicator {
                        anchors.centerIn: parent
                        running: authorStarterPackList.model?.getFeedInProgress
                    }

                    EmptyListIndication {
                        svg: SvgOutline.noLists
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
            }

            // Lists
            Loader {
                id: listsView
                active: hasLists
                asynchronous: true

                sourceComponent: ListView {
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

                    footer: DeadFooterMargin {}
                    footerPositioning: ListView.OverlayFooter

                    delegate: ListViewDelegate {
                        width: authorFeedView.width
                        userDid: page.userDid
                        allowEdit: false

                        onBlockList: (list) => graphUtils.blockList(list.uri)
                        onUnblockList: (list, blockedUri) => graphUtils.unblockList(list.uri, blockedUri)
                        onMuteList: (list) => graphUtils.muteList(list.uri)
                        onUnmuteList: (list) => graphUtils.unmuteList(list.uri)
                        onHideList: (list) => graphUtils.hideList(list.uri)
                        onUnhideList: (list) => graphUtils.unhideList(list.uri)
                        onHideReplies: (list, hide) => graphUtils.hideReplies(list.uri, hide)
                        onHideFollowing: (list, hide) => graphUtils.hideFollowing(list.uri, hide)
                        onSyncList: (list, sync) => graphUtils.syncList(list.uri, sync)
                    }

                    FlickableRefresher {
                        inProgress: authorListList.model?.getFeedInProgress
                        bottomOvershootFun: () => getListListNextPage(listListModelId)
                    }

                    BusyIndicator {
                        anchors.centerIn: parent
                        running: authorListList.model?.getFeedInProgress
                    }

                    EmptyListIndication {
                        svg: SvgOutline.noLists
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
            }

            Component.onCompleted: {
                if (!isLabeler)
                    removeItem(labelsView)

                if (!page.isUser(author))
                    removeItem(likesView)

                if (!hasFeeds)
                    removeItem(feedsView)

                if (!hasStarterPacks)
                    removeItem(starterPacksView)

                if (!hasLists)
                    removeItem(listsView)
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

                for (let i = 0; i < contentChildren.length; ++i) {
                    let c = contentChildren[i]

                    if (c instanceof AuthorPostsList) {
                        if (c.modelId !== page.modelId)
                            c.removeModel()
                        else
                            c.refresh()
                    }
                    else if (c.item) {
                        c.item.refresh()
                    }
                }
            }

            function clear() {
                for (let i = 0; i < contentChildren.length; ++i) {
                    let c = contentChildren[i]

                    if (c instanceof AuthorPostsList)
                        c.clear()
                    else if (c.item)
                        c.item.clear()
                }
            }
        }

        function feedOk(modelId) {
            itemAtIndex(0).feedOk(modelId) // qmllint disable missing-property
        }

        function retryGetFeed(modelId, error, msg) {
            if (error === ATProtoErrorMsg.BLOCKED_ACTOR && itemAtIndex(0).retryGetFeed(modelId)) // qmllint disable missing-property
                return

            skywalker.showStatusMessage(msg, QEnums.STATUS_LEVEL_ERROR)
        }

        function refresh() {
            itemAtIndex(0).refresh() // qmllint disable missing-property
        }

        function clear() {
            itemAtIndex(0).clear() // qmllint disable missing-property
        }
    }

    PostUtils {
        id: postUtils
        skywalker: page.skywalker
    }

    FeedUtils {
        id: feedUtils
        skywalker: page.skywalker
    }

    GraphUtils {
        id: graphUtils
        skywalker: page.skywalker

        onFollowOk: (uri) => { following = uri }
        onFollowFailed: (error) => { skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR) }
        onUnfollowOk: following = ""
        onUnfollowFailed: (error) => { skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR) }

        onBlockOk: (uri, expiresAt) => {
                       blocking = uri
                       authorFeedView.clear()

                        if (isNaN(expiresAt.getTime()))
                            skywalker.showStatusMessage(qsTr("Blocked"), QEnums.STATUS_LEVEL_INFO, 2)
                        else
                            skywalker.showStatusMessage(qsTr(`Blocked till ${guiSettings.expiresIndication(expiresAt)}`), QEnums.STATUS_LEVEL_INFO, 2)
                   }

        onBlockFailed: (error) => { skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR) }

        onUnblockOk: {
            blocking = ""
            authorFeedView.refresh()
            skywalker.showStatusMessage(qsTr("Unblocked"), QEnums.STATUS_LEVEL_INFO, 2)
        }

        onUnblockFailed: (error) => { skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR) }

        onMuteOk: (expiresAt) => {
            authorMuted = true
            authorFeedView.clear()

            if (isNaN(expiresAt.getTime()))
                skywalker.showStatusMessage(qsTr("Muted"), QEnums.STATUS_LEVEL_INFO, 2)
            else
                skywalker.showStatusMessage(qsTr(`Muted till ${guiSettings.expiresIndication(expiresAt)}`), QEnums.STATUS_LEVEL_INFO, 2)
        }

        onMuteFailed: (error) => { skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR) }

        onUnmuteOk: {
            authorMuted = false
            authorFeedView.refresh()
            skywalker.showStatusMessage(qsTr("Unmuted"), QEnums.STATUS_LEVEL_INFO, 2)
        }

        onUnmuteFailed: (error) => { skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR) }

        onBlockListFailed: (error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
        onUnblockListFailed: (error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
        onMuteListFailed: (error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
        onUnmuteListFailed: (error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)

        onMuteRepostsOk: {
            authorMutedReposts = graphUtils.areRepostsMuted(author.did)
            authorFeedView.refresh()
            skywalker.showStatusMessage(qsTr("Muted reposts"), QEnums.STATUS_LEVEL_INFO, 2)
        }
        onMuteRepostsFailed: (error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)

        onUnmuteRepostsOk: {
            authorMutedReposts = graphUtils.areRepostsMuted(author.did)
            authorFeedView.refresh()
            skywalker.showStatusMessage(qsTr("Unmuted reposts"), QEnums.STATUS_LEVEL_INFO, 2)
        }
        onUnmuteRepostsFailed: (error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
    }

    ProfileUtils {
        id: profileUtils
        skywalker: page.skywalker // qmllint disable missing-type

        onGetLabelerViewDetailedOk: (view) => setLabeler(view)
        onGetLabelerViewDetailedFailed: (error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)

        onLikeLabelerOk: (likeUri) => {
            labelerLikeUri = likeUri
            ++labelerLikeCount
            labelerLikeTransient = false
        }

        onLikeLabelerFailed: (error) => {
            labelerLikeTransient = false
            skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
        }

        onUndoLikeLabelerOk: {
            labelerLikeUri = ""
            --labelerLikeCount
            labelerLikeTransient = false
        }

        onUndoLikeLabelerFailed: (error) => {
            labelerLikeTransient = false
            skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
        }

        onFirstAppearanceOk: (did, appearance) => setFirstAppearance(appearance)
    }

    NotificationUtils {
        id: notificationUtils
        skywalker: page.skywalker

        onSubscribeActivityOk: (did, subscription) => {
            page.activitySubscription = subscription

            if (!subscription.post && !subscription.reply) {
                skywalker.showStatusMessage(qsTr(`Unsubscribed from activities from ${author.name}`), QEnums.STATUS_LEVEL_INFO, 4)
                return
            }

            let activityList = []

            if (subscription.post)
                activityList.push(qsTr("posts"))

            if (subscription.reply)
                activityList.push(qsTr("replies"))

            const activityText = guiSettings.toWordSequence(activityList)
            skywalker.showStatusMessage(qsTr(`Subscribed to ${activityText} from ${author.name}`), QEnums.STATUS_LEVEL_INFO, 4)
        }

        onSubscribeActivityFailed: (error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
    }

    AccessibilityUtils {
        id: accessibilityUtils
    }

    function authorCanBeMentioned() {
        return !page.isUser(author) && !author.hasInvalidHandle()
    }

    function subscribeActivity() {
        let component = guiSettings.createComponent("SubscribeActivity.qml")
        let dialog = component.createObject(page, { subscription: page.activitySubscription })

        dialog.onSubscribeTo.connect((post, reply) => {
            notificationUtils.subscribeActivity(author.did, post, reply)
        })

        dialog.onRejected.connect(() => dialog.destroy())
        dialog.open()
    }

    function setFirstAppearance(appearanceDate) {
        firstAppearanceDate = appearanceDate.toLocaleDateString(Qt.locale(), Locale.ShortFormat)
    }

    function editAuthor(author) {
        let component = guiSettings.createComponent("EditProfile.qml")
        let editPage = component.createObject(page, {
                authorDid: author.did,
                authorName: authorName,
                authorPronouns: authorPronouns,
                authorDescription: authorDescription,
                authorWebsite: authorWebsite,
                authorAvatar: authorAvatar,
                authorBanner: authorBanner,
                authorVerified: authorVerified
            })
        editPage.profileUpdated.connect((name, description, avatar, banner, pronouns, website) => {
            skywalker.showStatusMessage(qsTr("Profile updated."), QEnums.STATUS_LEVEL_INFO, 2)
            authorName = name
            authorPronouns = pronouns
            authorDescription = description
            authorWebsite = website
            authorAvatar = avatar
            setAuthorBanner(banner)

            // NOTE: if avatar is an "image://" source, then the profile takes ownership
            skywalker.updateUserProfile(name, description, avatar, pronouns)

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
        const mentionText = (page.isUser(author) || author.hasInvalidHandle()) ? text : `@${author.handle} ${text}`
        root.composePost(mentionText, imgSource, page.userDid)
    }

    function mentionVideoPost(text = "", videoSource = "") {
        const mentionText = (page.isUser(author) || author.hasInvalidHandle()) ? text : `@${author.handle} ${text}`
        root.composeVideoPost(mentionText, videoSource, page.userDid)
    }

    function feedOkHandler(modelId) {
        authorFeedView.feedOk(modelId)

        // HACK: When a live view is present the page is sometimes not positioned at
        // the beginning. Somehow positioning the page only works after the initial feed
        // has been loaded??
        if (!feedInitialized) {
            console.debug("INIT FEED")
            authorFeedView.positionViewAtBeginning()
            feedInitialized = true
        }
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
        let component = guiSettings.createComponent("AddUserListListView.qml")
        let view = component.createObject(page, {
                userDid: page.userDid,
                author: author,
                modelId: listModelId
        })
        view.onClosed.connect(() => { popStack() }) // qmllint disable missing-property
        pushStack(view)
        skywalker.getListList(listModelId)
    }

    function mustGetFeed() {
        return !authorMuted && !blocking && contentVisible()
    }

    function contentVisible() {
        return showWarnedMedia || guiSettings.contentVisible(author, page.userDid)
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
            return SvgOutline.block
        } else if (authorMuted) {
            return SvgOutline.mute
        } else if (!contentVisible()) {
            return SvgOutline.hideVisibility
        }

        return SvgOutline.noPosts
    }

    function getEmptyListIndicationText() {
        if (!author.viewer.blockingByList.isNull()) {
            const listName = UnicodeFonts.toCleanedHtml(author.viewer.blockingByList.name)
            return qsTr(`Blocked by list: <a href="${author.viewer.blockingByList.uri}" style="color: ${guiSettings.linkColor}">${listName}</a>`)
        } else if (blocking) {
            return getBlockingText()
        } else if (author.viewer.blockedBy) {
            return qsTr("You are blocked")
        } else if (!author.viewer.mutedByList.isNull()) {
            const listName = UnicodeFonts.toCleanedHtml(author.viewer.mutedByList.name)
            return qsTr(`Muted by list: <a href="${author.viewer.mutedByList.uri}" style="color: ${guiSettings.linkColor}">${listName}</a>`)
        } else if (authorMuted) {
            return getMutingText()
        } else if (!contentVisible()) {
            return contentWarning
        }

        return qsTr("No posts")
    }

    function getBlockingText() {
        const blocksWithExpiry = skywalker.getUserSettings().blocksWithExpiry
        const expiresAt = blocksWithExpiry.getExpiry(blocking)

        if (!isNaN(expiresAt.getTime()))
            return qsTr(`You blocked this account till ${guiSettings.expiresIndication(expiresAt)}`)

        return qsTr("You blocked this account")
    }

    function getMutingText() {
        const mutesWithExpiry = skywalker.getUserSettings().mutesWithExpiry
        const expiresAt = mutesWithExpiry.getExpiry(author.did)

        if (!isNaN(expiresAt.getTime()))
            return qsTr(`You muted this account till ${guiSettings.expiresIndication(expiresAt)}`)

        return qsTr("You mutes this account")
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

    function isUserDid(did) {
        return did === userDid
    }

    function isUser(author) {
        return isUserDid(author.did)
    }

    Component.onDestruction: {
        skywalker.onGetAuthorFeedFailed.disconnect(feedFailedHandler)
        skywalker.onGetAuthorFeedOk.disconnect(feedOkHandler)

        if (isUser(author)) {
            const pu = root.getProfileUtils()
            pu.onSetPinnedPostOk.disconnect(pinnedPostHandler)
            pu.onClearPinnedPostOk.disconnect(unpinnedPostHandler)
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
        skywalker.onGetAuthorFeedFailed.connect(feedFailedHandler)
        skywalker.onGetAuthorFeedOk.connect(feedOkHandler)

        if (isUser(author)) {
            const pu = root.getProfileUtils()
            pu.onSetPinnedPostOk.connect(pinnedPostHandler)
            pu.onClearPinnedPostOk.connect(unpinnedPostHandler)
        }

        authorName = author.name
        authorPronouns = author.pronouns
        authorDescription = author.description
        authorWebsite = author.website
        authorAvatar = author.avatarUrl
        authorBanner = author.banner
        authorMutedReposts = graphUtils.areRepostsMuted(author.did)
        authorHideFromTimeline = skywalker.getTimelineHide().contains(author.did)
        contentVisibility = skywalker.getContentVisibility(author.labels)
        contentWarning = skywalker.getContentWarning(author.labels)

        getFeed(modelId)

        if (author.hasCreatedAt)
            setFirstAppearance(author.createdAt)
        else
            profileUtils.getFirstAppearance(author.did)

        if (hasFeeds)
            getFeedList(feedListModelId)

        if (hasLists)
            getListList(listListModelId)

        if (hasStarterPacks)
            getStarterPackList(starterPackListModelId)

        if (isLabeler)
            profileUtils.getLabelerViewDetailed(author.did)

        // HACK: directly setting the current index does not work...
        Qt.callLater(() => {
            authorFeedView.headerItem.getFeedMenuBar().setCurrentIndex(0)
        })
    }
}
