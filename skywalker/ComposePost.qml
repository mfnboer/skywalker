import QtQuick
import QtQuick.Controls
import QtQuick.Window 2.2
import skywalker

SkyPage {
    required property var skywalker
    property string initialText
    property string initialImage
    property string initialVideo: ""
    property int margin: 15

    readonly property int maxLanguageIdentificationLength: 200
    readonly property int maxPostLength: 300
    readonly property int maxThreadPosts: 99
    readonly property int minPostSplitLineLength: 30
    readonly property int maxImages: 4 // per post
    property bool pickingImage: false

    // Reply restrictions (on post thread)
    property bool restrictReply: false
    property bool allowReplyMentioned: false
    property bool allowReplyFollower: false
    property bool allowReplyFollowing: false
    property list<int> allowListIndexes: [0, 1, 2]
    property list<bool> allowLists: [false, false, false]

    // Used for list restrictions from draft of default
    property list<string> allowListUrisFromDraft: []
    property list<string> allowListNamesFromDraft: []

    property int restrictionsListModelId: -1
    property bool allowQuoting: true

    // Reply-to (first post)
    property basicprofile replyToAuthor
    property string replyToPostUri: ""
    property string replyToPostCid: ""
    property string replyRootPostUri: ""
    property string replyRootPostCid: ""
    property string replyToPostText
    property date replyToPostDateTime
    property string replyToLanguage

    // Quote post (for first post only)
    property bool openedAsQuotePost: false
    property basicprofile quoteAuthor
    property string quoteUri: ""
    property string quoteCid: ""
    property string quoteText
    property date quoteDateTime

    property basicprofile nullAuthor
    property generatorview nullFeed
    property listview nullList
    property tenorgif nullGif

    readonly property string userDid: skywalker.getUserDid()
    property bool requireAltText: skywalker.getUserSettings().getRequireAltText(userDid)
    property bool autoLinkCard: skywalker.getUserSettings().getAutoLinkCard()
    property bool threadAutoNumber: skywalker.getUserSettings().getThreadAutoNumber()
    property string threadPrefix: skywalker.getUserSettings().getThreadPrefix()
    property bool threadAutoSplit: skywalker.getUserSettings().getThreadAutoSplit()

    property int currentPostIndex: 0

    // Current post of a thread being sent
    property int sendingThreadPost: -1
    property string threadRootUri
    property string threadRootCid

    // Can be different from threadRoot if the thread is a reply to an existing post
    property string threadFirstPostUri
    property string threadFirstPostCid
    property bool threadGateCreated: false
    property list<string> postedUris: []

    property bool isAnniversary: skywalker.getAnniversary().isAnniversary()

    // Cache
    property list<string> tmpImages: []
    property list<string> tmpVideos: []

    signal closed

    id: page
    width: parent.width
    height: parent.height
    contentHeight: flick.height
    topPadding: 0
    bottomPadding: 10

    onCurrentPostIndexChanged: languageIdentificationTimer.reset()

    onAllowListUrisFromDraftChanged: getListNamesFromDraft()

    header: Rectangle {
        width: parent.width
        height: guiSettings.headerHeight
        z: guiSettings.headerZLevel
        color: guiSettings.headerColor

        SvgPlainButton {
            id: cancelButton
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            svg: SvgOutline.cancel
            accessibleName: qsTr("cancel posting")
            onClicked: page.cancel()
        }

        Avatar {
            anchors.centerIn: parent
            height: parent.height - 10
            width: height
            author: skywalker.user
            onClicked: skywalker.showStatusMessage(qsTr("Yes, you're gorgeous!"), QEnums.STATUS_LEVEL_INFO)
            onPressAndHold: skywalker.showStatusMessage(qsTr("Yes, you're really gorgeous!"), QEnums.STATUS_LEVEL_INFO)

            Accessible.role: Accessible.Button
            Accessible.name: qsTr("your avatar")
            Accessible.onPressAction: clicked()
        }

        SkyButton {
            property bool isPosting: false

            id: postButton
            anchors.right: moreOptions.left
            anchors.verticalCenter: parent.verticalCenter
            text: replyToPostUri ? qsTr("Reply", "verb on post composition") : qsTr("Post", "verb on post composition")
            enabled: !isPosting && postsAreValid() && hasFullContent() && checkAltText()
            onClicked: sendPost()
            Accessible.name: replyToPostUri ? qsTr("send reply") : qsTr("send post")

            function sendPost() {
                if (!checkMisleadingEmbeddedLinks())
                    return

                postButton.isPosting = true
                threadPosts.copyPostItemsToPostList()

                if (threadPosts.count === 1) {
                    sendSinglePost(threadPosts.postList[0],
                                   replyToPostUri, replyToPostCid,
                                   replyRootPostUri, replyRootPostCid, 0, 1)
                }
                else {
                    sendThreadPosts(0, replyToPostUri, replyToPostCid,
                                    replyRootPostUri, replyRootPostCid)
                }
            }
        }

        SvgPlainButton {
            id: moreOptions
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            svg: SvgOutline.moreVert
            accessibleName: qsTr("post options")
            onClicked: moreMenu.open()

            Menu {
                id: moreMenu
                width: Math.max(altItem.width, numberPrefixItem.width)
                modal: true

                onAboutToShow: root.enablePopupShield(true)
                onAboutToHide: root.enablePopupShield(false)

                CloseMenuItem {
                    text: qsTr("<b>Options</b>")
                    Accessible.name: qsTr("close options menu")
                }
                AccessibleMenuItem {
                    id: altItem
                    text: qsTr("Require ALT text")
                    checkable: true
                    checked: skywalker.getUserSettings().getRequireAltText(userDid)

                    onToggled: {
                        requireAltText = checked
                        skywalker.getUserSettings().setRequireAltText(userDid, checked)
                    }
                }
                AccessibleMenuItem {
                    text: qsTr("Auto link card")
                    checkable: true
                    checked: skywalker.getUserSettings().getAutoLinkCard()

                    onToggled: {
                        autoLinkCard = checked
                        skywalker.getUserSettings().setAutoLinkCard(checked)
                    }
                }
                AccessibleMenuItem {
                    id: autoNumberItem
                    text: qsTr("Auto number")
                    checkable: true
                    checked: skywalker.getUserSettings().getThreadAutoNumber()

                    onToggled: {
                        threadAutoNumber = checked
                        skywalker.getUserSettings().setThreadAutoNumber(checked)
                    }
                }
                AccessibleMenuItem {
                    id: numberPrefixItem
                    contentItem: AccessibleText {
                        textFormat: Text.RichText
                        text: qsTr(`Number prefix: ${(threadPrefix ? UnicodeFonts.toCleanedHtml(threadPrefix) : qsTr("<i>&lt;none&gt;</i>"))}`)
                    }
                    enabled: autoNumberItem.checked
                    onTriggered: editThreadPrefix()
                }
                AccessibleMenuItem {
                    text: qsTr("Auto split")
                    checkable: true
                    checked: skywalker.getUserSettings().getThreadAutoSplit()

                    onToggled: {
                        threadAutoSplit = checked
                        skywalker.getUserSettings().setThreadAutoSplit(checked)
                    }
                }
                AccessibleMenuItem {
                    text: qsTr("Merge posts")
                    enabled: threadPosts.count > 1
                    onTriggered: threadPosts.mergePosts()
                }
                AccessibleMenuItem {
                    text: qsTr("Video limits")
                    onTriggered: {
                        busyIndicator.running = true
                        postUtils.getVideoUploadLimits()
                    }
                }
            }
        }
    }

    Flickable {
        id: flick
        anchors.fill: parent
        clip: true
        contentWidth: page.width
        contentHeight: threadColumn.y + threadColumn.height
        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds

        onHeightChanged: {
            let postItem = currentPostItem()

            if (!postItem)
                return

            let postText = postItem.getPostText()
            postText.ensureVisible(postText.cursorRectangle)
        }

        // Reply-to
        Rectangle {
            radius: 10
            anchors.fill: replyToColumn
            border.width: 1
            border.color: guiSettings.borderHighLightColor
            color: guiSettings.postHighLightColor
            visible: replyToColumn.visible
        }
        QuotePost {
            id: replyToColumn
            y: 10
            width: parent.width - 2 * page.margin
            anchors.horizontalCenter: parent.horizontalCenter
            author: replyToAuthor
            postText: replyToPostText
            postDateTime: replyToPostDateTime
            ellipsisBackgroundColor: guiSettings.postHighLightColor
            visible: replyToPostUri
        }

        Column {
            id: threadColumn
            y: replyToColumn.y + (replyToColumn.visible ? replyToColumn.height + 5 : 0)
            width: parent.width

            Repeater {
                property list<ComposePostItem> postList: [
                    ComposePostItem {
                        text: initialText ? initialText : ""
                        embeddedLinks: []
                        images: initialImage ? [initialImage] : []
                        altTexts: initialImage ? [""] : []
                        memeTopTexts: initialImage ? [""] : []
                        memeBottomTexts: initialImage ? [""] : []
                        quoteAuthor: page.quoteAuthor
                        quoteUri: page.quoteUri
                        quoteCid: page.quoteCid
                        quoteText: page.quoteText
                        quoteDateTime: page.quoteDateTime
                        language: replyToLanguage ? replyToLanguage : languageUtils.defaultPostLanguage
                        video: ""
                        videoAltText: ""
                    }
                ]

                id: threadPosts
                width: parent.width
                model: 1

                Item {
                    required property int index
                    property string text
                    property list<weblink> embeddedLinks
                    property list<string> images
                    property list<string> altTexts
                    property list<string> memeTopTexts
                    property list<string> memeBottomTexts
                    property basicprofile quoteAuthor
                    property string quoteUri
                    property string quoteCid
                    property string quoteText
                    property date quoteDateTime
                    property bool quoteFixed: false
                    property generatorview quoteFeed
                    property listview quoteList
                    property bool cwSuggestive: false
                    property bool cwNudity: false
                    property bool cwPorn: false
                    property bool cwGore: false
                    property string language
                    property int languageSource
                    property string video
                    property string videoAltText
                    property int videoNewHeight
                    property int videoStartMs
                    property int videoEndMs
                    property bool videoRemoveAudio

                    function copyToPostList() {
                        threadPosts.postList[index].text = text
                        threadPosts.postList[index].embeddedLinks = postText.embeddedLinks
                        threadPosts.postList[index].images = images
                        threadPosts.postList[index].altTexts = altTexts
                        threadPosts.postList[index].memeTopTexts = memeTopTexts
                        threadPosts.postList[index].memeBottomTexts = memeBottomTexts
                        threadPosts.postList[index].quoteAuthor = quoteAuthor
                        threadPosts.postList[index].quoteUri = quoteUri
                        threadPosts.postList[index].quoteCid = quoteCid
                        threadPosts.postList[index].quoteText = quoteText
                        threadPosts.postList[index].quoteDateTime = quoteDateTime
                        threadPosts.postList[index].quoteFixed = quoteFixed
                        threadPosts.postList[index].quoteFeed = quoteFeed
                        threadPosts.postList[index].quoteList = quoteList
                        threadPosts.postList[index].gif = gifAttachment.gif
                        threadPosts.postList[index].card = linkCard.card
                        threadPosts.postList[index].cwSuggestive = cwSuggestive
                        threadPosts.postList[index].cwNudity = cwNudity
                        threadPosts.postList[index].cwPorn = cwPorn
                        threadPosts.postList[index].cwGore = cwGore
                        threadPosts.postList[index].language = language
                        threadPosts.postList[index].languageSource = languageSource
                        threadPosts.postList[index].video = video
                        threadPosts.postList[index].videoAltText = videoAltText
                        threadPosts.postList[index].videoNewHeight = videoNewHeight
                        threadPosts.postList[index].videoStartMs = videoStartMs
                        threadPosts.postList[index].videoEndMs = videoEndMs
                        threadPosts.postList[index].videoRemoveAudio = videoRemoveAudio
                    }

                    function copyFromPostList() {
                        images = threadPosts.postList[index].images
                        altTexts = threadPosts.postList[index].altTexts
                        memeTopTexts = threadPosts.postList[index].memeTopTexts
                        memeBottomTexts = threadPosts.postList[index].memeBottomTexts
                        quoteAuthor = threadPosts.postList[index].quoteAuthor
                        quoteUri = threadPosts.postList[index].quoteUri
                        quoteCid = threadPosts.postList[index].quoteCid
                        quoteText = threadPosts.postList[index].quoteText
                        quoteDateTime = threadPosts.postList[index].quoteDateTime
                        quoteFixed = threadPosts.postList[index].quoteFixed
                        quoteFeed = threadPosts.postList[index].quoteFeed
                        quoteList = threadPosts.postList[index].quoteList
                        linkCard.card = threadPosts.postList[index].card

                        if (!threadPosts.postList[index].gif.isNull())
                            gifAttachment.show(threadPosts.postList[index].gif)
                        else
                            gifAttachment.hide()

                        cwSuggestive = threadPosts.postList[index].cwSuggestive
                        cwNudity = threadPosts.postList[index].cwNudity
                        cwPorn = threadPosts.postList[index].cwPorn
                        cwGore = threadPosts.postList[index].cwGore
                        language = threadPosts.postList[index].language
                        languageSource = threadPosts.postList[index].languageSource
                        video = threadPosts.postList[index].video
                        videoAltText = threadPosts.postList[index].videoAltText
                        videoNewHeight = threadPosts.postList[index].videoNewHeight
                        videoStartMs = threadPosts.postList[index].videoStartMs
                        videoEndMs = threadPosts.postList[index].videoEndMs
                        videoRemoveAudio = threadPosts.postList[index].videoRemoveAudio

                        // Set text last as it will trigger link extractions which
                        // will check if a link card is already in place.
                        text = threadPosts.postList[index].text
                        postText.setEmbeddedLinks(threadPosts.postList[index].embeddedLinks)
                    }

                    function getPostText() { return postText }
                    function getImageScroller() { return imageScroller }
                    function getGifAttachment() { return gifAttachment }
                    function getLinkCard() { return linkCard }

                    function hasAttachment() {
                        return imageScroller.images.length > 0 ||
                                Boolean(videoAttachement.video) ||
                                !gifAttachment.gif.isNull() ||
                                linkCard.card ||
                                quoteFixed
                    }

                    function calcHeight() {
                        if (quoteColumn.visible)
                            return quoteColumn.y + quoteColumn.height

                        if (quoteFeedColumn.visible)
                            return quoteFeedColumn.y + quoteFeedColumn.height

                        if (quoteListColumn.visible)
                            return quoteListColumn.y + quoteListColumn.height

                        return quoteColumn.y
                    }

                    function hasContent() {
                        return postText.graphemeLength > 0 || hasAttachment()
                    }

                    function isValid() {
                        return postText.graphemeLength <= postText.maxLength
                    }

                    function fixQuoteLink(fix) {
                        quoteFixed = fix

                        if (!fix) {
                            quoteUri = ""
                            quoteList = page.nullList
                            quoteFeed = page.nullFeed
                        }
                    }

                    id: postItem
                    width: parent.width
                    height: calcHeight()
                    opacity: index === currentPostIndex ? 1.0 : 0.6

                    SeparatorLine {
                        id: separatorLine
                        visible: index > 0
                    }

                    SkyFormattedTextEdit {
                        property bool splitting: false

                        id: postText
                        anchors.top: separatorLine.bottom
                        width: parent.width
                        leftPadding: page.margin
                        rightPadding: page.margin
                        topPadding: 0
                        bottomPadding: 0
                        parentPage: page
                        parentFlick: flick
                        placeholderText: index === 0 ? qsTr("Say something nice") : qsTr(`Add post ${(index + 1)}`)
                        initialText: postItem.text
                        maxLength: page.maxPostLength - postCountText.size()
                        fontSelectorCombo: fontSelector

                        onTextUpdated: {
                            const textHasChanged = Boolean(postItem.text !== text)
                            postItem.text = text

                            if (splitting)
                                return

                            if (threadAutoSplit && graphemeLength > maxLength) {
                                console.debug("SPLIT:", index)

                                // Avoid to re-split when the post count text becomes visible or longer
                                const maxPartLength = page.maxPostLength - postCountText.maxSize()
                                const parts = textSplitter.splitText(text, embeddedLinks, maxPartLength, minPostSplitLineLength, 2)

                                if (parts.length > 1) {
                                    const moveCursor = cursorPosition > parts[0].text.length && index === currentPostIndex
                                    const oldCursorPosition = cursorPosition

                                    splitting = true
                                    text = UnicodeFonts.rtrim(parts[0].text)
                                    embeddedLinks = parts[0].embeddedLinks

                                    if (!moveCursor && index === currentPostIndex)
                                        cursorPosition = oldCursorPosition

                                    splitting = false
                                    postUtils.identifyLanguage(textWithoutLinks, index)

                                    if (index === threadPosts.count - 1 || threadPosts.itemAt(index + 1).hasAttachment()) {
                                        threadPosts.addPost(index, parts[1].text, parts[1].embeddedLinks, moveCursor)
                                    }
                                    else {
                                        // Prepend excess text to next post
                                        let nextPostText = threadPosts.itemAt(index + 1).getPostText()
                                        let joinedPart = textSplitter.joinText(parts[1].text, parts[1].embeddedLinks, nextPostText.text, nextPostText.embeddedLinks)
                                        const newText = joinedPart.text
                                        const newLinks = joinedPart.embeddedLinks
                                        const newCursorPosition = moveCursor ? oldCursorPosition - parts[0].text.length : -1

                                        setPostTextTimer.startSetText(newText, newLinks, index + 1, newCursorPosition)

                                        if (moveCursor)
                                            currentPostIndex = index + 1
                                    }
                                }
                            }
                            else if (textHasChanged) {
                                if (index === currentPostIndex)
                                    languageIdentificationTimer.start()
                                else
                                    postUtils.identifyLanguage(textWithoutLinks, index)
                            }
                        }

                        onFocusChanged: {
                            if (focus)
                                currentPostIndex = index
                        }

                        onFirstWebLinkChanged: {
                            if (!autoLinkCard)
                                return

                            if (gifAttachment.visible)
                                return

                            if (linkCard.card && linkCard.card.link === firstWebLink)
                                return

                            if (linkCard.linkFixed)
                                return

                            linkCard.hide()

                            if (firstWebLink) {
                                linkCardTimer.startForLink(index, firstWebLink)
                            } else {
                                linkCardTimer.stop()
                            }
                        }

                        onCursorInFirstWebLinkChanged: {
                            if (!cursorInFirstWebLink && linkCard.card)
                                linkCard.linkFixed = true
                        }

                        onFirstPostLinkChanged: {
                            if (page.openedAsQuotePost && index === 0)
                                return

                            if (quoteFixed)
                                return

                            quoteList = page.nullList
                            quoteFeed = page.nullFeed
                            quoteUri = ""

                            if (firstPostLink)
                                postUtils.getQuotePost(firstPostLink)
                        }

                        onCursorInFirstPostLinkChanged: {
                            if (!cursorInFirstPostLink && quoteUri)
                                fixQuoteLink(true)
                        }

                        onFirstFeedLinkChanged: {
                            if (page.openedAsQuotePost && index === 0)
                                return

                            if (quoteFixed)
                                return

                            quoteList = page.nullList
                            quoteFeed = page.nullFeed

                            if (firstPostLink)
                                return

                            if (firstFeedLink)
                                postUtils.getQuoteFeed(firstFeedLink)
                        }

                        onCursorInFirstFeedLinkChanged: {
                            if (!cursorInFirstFeedLink && !quoteFeed.isNull())
                                fixQuoteLink(true)
                        }

                        onFirstListLinkChanged: {
                            if (page.openedAsQuotePost && index === 0)
                                return

                            if (quoteFixed)
                                return

                            quoteList = page.nullList

                            if (firstPostLink || firstFeedLink)
                                return

                            if (firstListLink)
                                postUtils.getQuoteList(firstListLink)
                        }

                        onCursorInFirstListLinkChanged: {
                            if (!cursorInFirstListLink && !quoteList.isNull())
                                fixQuoteLink(true)
                        }
                    }

                    SvgButton {
                        y: postText.y - 6
                        x: parent.width - width - 10
                        z: 10
                        width: 34
                        height: width
                        svg: SvgOutline.remove
                        accessibleName: qsTr("remove post")
                        visible: !postItem.hasContent() && threadPosts.count > 1 && (index > 0 || !replyToPostUri)

                        onClicked: threadPosts.removePost(index)
                    }

                    AccessibleText {
                        id: postCountText
                        width: page.width
                        leftPadding: page.margin
                        rightPadding: page.margin
                        anchors.top: postText.bottom
                        textFormat: Text.RichText
                        text: getPostCountText(index, threadPosts.count)
                        visible: threadPosts.count > 1 && threadAutoNumber

                        function size() {
                            // +1 for newline
                            return visible ? text.length + 1 : 0
                        }

                        function maxSize() {
                            if (!threadAutoNumber)
                                return 0

                            const countText = getPostCountText(page.maxThreadPosts, page.maxThreadPosts)
                            return countText.length + 1
                        }
                    }

                    // Image attachments
                    ImageScroller {
                        property alias images: postItem.images
                        property alias altTexts: postItem.altTexts
                        property alias memeTopTexts: postItem.memeTopTexts
                        property alias memeBottomTexts: postItem.memeBottomTexts

                        id: imageScroller
                        width: page.width
                        anchors.top: postCountText.visible ? postCountText.bottom : postText.bottom
                        horizontalPadding: page.margin
                        requireAltText: page.requireAltText
                        postUtils: page.getPostUtils()
                        visible: !linkCard.visible && !gifAttachment.visible
                    }

                    VideoAttachment {
                        property alias video: postItem.video
                        property alias altText: postItem.videoAltText
                        property alias startMs: postItem.videoStartMs
                        property alias endMs: postItem.videoEndMs
                        property alias removeAudio: postItem.videoRemoveAudio
                        property alias newHeight: postItem.videoNewHeight

                        id: videoAttachement
                        x: page.margin
                        width: Math.min(height * 1.777, page.width - 2 * page.margin)
                        height: visible ? 180 : 0
                        anchors.top: imageScroller.bottom
                        anchors.topMargin: visible ? 10 : 0
                        videoSource: video
                        videoStartMs: startMs
                        videoEndMs: endMs
                        requireAltText: page.requireAltText
                        visible: Boolean(video) && !linkCard.visible && !linkCard.visible

                        onEdit: editVideo(video, startMs, endMs, removeAudio, newHeight)
                    }

                    // GIF attachment
                    AnimatedImage {
                        property tenorgif gif

                        id: gifAttachment
                        x: page.margin
                        width: Math.min(gif ? gif.smallSize.width : 1, page.width - 2 * page.margin)
                        anchors.top: videoAttachement.bottom
                        anchors.topMargin: !gif.isNull() ? 10 : 0
                        fillMode: Image.PreserveAspectFit
                        source: !gif.isNull() ? gif.smallUrl : ""
                        visible: !gif.isNull()

                        onGifChanged: threadPosts.postList[index].gif = gif

                        Accessible.role: Accessible.StaticText
                        Accessible.name: qsTr("GIF image")

                        function show(gif) {
                            gifAttachment.gif = gif
                            linkCard.hide()
                        }

                        function hide() {
                            gifAttachment.gif = nullGif
                        }

                        SvgButton {
                            x: parent.width - width
                            width: 34
                            height: width
                            svg: SvgOutline.close
                            accessibleName: qsTr("remove GIF image")
                            onClicked: gifAttachment.hide()
                        }
                    }

                    // Link card attachment
                    LinkCardView {
                        property var card: null
                        property bool linkFixed: false

                        id: linkCard
                        x: page.margin
                        width: page.width - 2 * page.margin
                        height: card ? columnHeight : 0
                        anchors.top: gifAttachment.bottom
                        anchors.topMargin: card ? 10 : 0
                        uri: card ? card.link : ""
                        title: card ? card.title : ""
                        description: card ? card.description : ""
                        thumbUrl: card ? card.thumb : ""
                        contentVisibility: QEnums.CONTENT_VISIBILITY_SHOW
                        contentWarning: ""
                        visible: card

                        onCardChanged: threadPosts.postList[index].card = card

                        Accessible.role: Accessible.StaticText
                        Accessible.name: getSpeech()

                        function getSpeech() {
                            if (!card)
                                return ""

                            const hostname = new URL(card.link).hostname
                            return qsTr("link card: ") + card.title + "\n\nfrom: " + hostname + "\n\n" + card.description
                        }

                        function show(card) {
                            linkCard.card = card
                            gifAttachment.hide()
                        }

                        function hide() {
                            linkCard.card = null
                            linkCard.linkFixed = false
                        }

                        SvgButton {
                            x: parent.width - width
                            width: 34
                            height: width
                            svg: SvgOutline.close
                            accessibleName: qsTr("remove link card")
                            onClicked: linkCard.hide()
                        }
                    }

                    // Quote post
                    Rectangle {
                        radius: 10
                        anchors.fill: quoteColumn
                        border.width: 1
                        border.color: guiSettings.borderHighLightColor
                        color: guiSettings.postHighLightColor
                        visible: quoteColumn.visible
                    }
                    QuotePost {
                        id: quoteColumn
                        width: parent.width - 2 * page.margin
                        anchors.top: linkCard.bottom
                        anchors.topMargin: visible ? 5 : 0
                        anchors.horizontalCenter: parent.horizontalCenter
                        author: postItem.quoteAuthor
                        postText: postItem.quoteText
                        postDateTime: postItem.quoteDateTime
                        ellipsisBackgroundColor: guiSettings.postHighLightColor
                        showCloseButton: postItem.quoteFixed
                        visible: postItem.quoteUri

                        onCloseClicked: {
                            postItem.fixQuoteLink(false)
                            postItem.getPostText().forceActiveFocus()
                        }
                    }

                    // Quote feed
                    Rectangle {
                        radius: 10
                        anchors.fill: quoteFeedColumn
                        border.width: 1
                        border.color: guiSettings.borderHighLightColor
                        color: guiSettings.postHighLightColor
                        visible: quoteFeedColumn.visible
                    }
                    QuoteFeed {
                        id: quoteFeedColumn
                        width: parent.width - 2 * page.margin
                        anchors.top: linkCard.bottom
                        anchors.topMargin: visible ? 5 : 0
                        anchors.horizontalCenter: parent.horizontalCenter
                        feed: postItem.quoteFeed
                        showCloseButton: postItem.quoteFixed
                        visible: !postItem.quoteFeed.isNull()

                        onCloseClicked: {
                            postItem.fixQuoteLink(false)
                            postItem.getPostText().forceActiveFocus()
                        }
                    }

                    // Quote list
                    Rectangle {
                        radius: 10
                        anchors.fill: quoteListColumn
                        border.width: 1
                        border.color: guiSettings.borderHighLightColor
                        color: guiSettings.postHighLightColor
                        visible: quoteListColumn.visible
                    }
                    QuoteList {
                        id: quoteListColumn
                        width: parent.width - 2 * page.margin
                        anchors.top: linkCard.bottom
                        anchors.topMargin: visible ? 5 : 0
                        anchors.horizontalCenter: parent.horizontalCenter
                        list: postItem.quoteList
                        showCloseButton: postItem.quoteFixed
                        visible: !postItem.quoteList.isNull()

                        onCloseClicked: {
                            postItem.fixQuoteLink(false)
                            postItem.getPostText().forceActiveFocus()
                        }
                    }
                }

                function newComposePostItem() {
                    let component = guiSettings.createComponent("ComposePostItem.qml")
                    return component.createObject(page)
                }

                function copyPostItemsToPostList() {
                    for (let i = 0; i < count; ++i) {
                        let item = itemAt(i)
                        item.copyToPostList()
                    }
                }

                function copyPostListToPostItems() {
                    for (let i = 0; i < count; ++i) {
                        let item = itemAt(i)
                        item.copyFromPostList()
                    }
                }

                function removePost(index) {
                    console.debug("REMOVE POST:", index)

                    if (count === 1) {
                        console.warn("Cannot remove last post")
                        return
                    }

                    let item = itemAt(index)
                    item.images.forEach((value, index, array) => { postUtils.dropPhoto(value); })

                    if (Boolean(item.video))
                        postUtils.dropVideo(item.video)

                    copyPostItemsToPostList()

                    if (index === 0 && openedAsQuotePost) {
                        openedAsQuotePost = false
                    }

                    if (currentPostIndex === count - 1)
                        currentPostIndex -= 1

                    model = 0
                    postList.splice(index, 1)
                    model = postList.length

                    copyPostListToPostItems()
                    moveFocusToCurrent()
                    console.debug("REMOVED POST:", index)
                }

                function addPost(index, text = "", embeddedLinks = [], focus = true) {
                    console.debug("ADD POST:", index)

                    if (count >= maxThreadPosts) {
                        console.warn("Maximum posts reached:", count)
                        return
                    }

                    const postItem = threadPosts.itemAt(currentPostIndex);
                    const oldCursorPosition = postItem.getPostText().cursorPosition
                    const lang = postItem.language

                    copyPostItemsToPostList()
                    model = 0
                    let newItem = newComposePostItem()
                    newItem.language = lang
                    postList.splice(index + 1, 0, newItem)
                    model = postList.length
                    copyPostListToPostItems()

                    if (currentPostIndex === index && focus) {
                        currentPostIndex += 1
                        focusTimer.start()
                    }
                    else {
                        setCursorTimer.startSetCursor(currentPostIndex, oldCursorPosition)
                    }

                    if (text)
                        setPostTextTimer.startSetText(text, embeddedLinks, index + 1)

                    console.debug("ADDED POST:", index)
                }

                function mergePosts() {
                    for (let i = 0; i < count; ++i) {
                        let item = itemAt(i)

                        if (item.hasAttachment())
                            continue

                        let postText = item.getPostText()
                        if (postText.text.length === postText.maxLength)
                            continue

                        i = mergePostsAt(i)
                    }
                }

                // TODO: merge embeddedLinks
                function mergePostsAt(index) {
                    if (index === count - 1)
                        return index

                    let postText = itemAt(index).getPostText()
                    let text = postText.text
                    let links = postText.embeddedLinks
                    let endIndex = index + 1

                    while (endIndex < count) {
                        let nextPost = itemAt(endIndex)

                        if (nextPost.hasAttachment())
                            break

                        let nextPostText = nextPost.getPostText()
                        let part = textSplitter.joinText(text, links, nextPostText.text, nextPostText.embeddedLinks)
                        text = part.text
                        links = part.embeddedLinks
                        ++endIndex
                    }

                    if (endIndex === index + 1)
                        return index

                    const maxLength = itemAt(endIndex - 1).getPostText().maxLength

                    for (let i = index + 1; i < endIndex; ++i)
                        threadPosts.removePost(index + 1)

                    const parts = textSplitter.splitText(text, links, maxLength, page.minPostSplitLineLengths)
                    let firstPost = threadPosts.itemAt(index).getPostText()
                    firstPost.text = UnicodeFonts.rtrim(parts[0].text)
                    firstPost.setEmbeddedLinks(parts[0].embeddedLinks)

                    for (let j = 1; j < parts.length; ++j) {
                        threadPosts.addPost(index + j - 1, "", [], false)
                        let nextPost = threadPosts.itemAt(index + j).getPostText()
                        nextPost.text = UnicodeFonts.rtrim(parts[j].text)
                        nextPost.setEmbeddedLinks(parts[j].embeddedLinks)
                    }

                    return index + parts.length - 1
                }

                function moveFocusToCurrent() {
                    let postText = currentPostItem().getPostText()
                    postText.cursorPosition = postText.text.length
                    focusTimer.start()
                }
            }
        }

        Rectangle {
            anchors.left: threadColumn.left
            anchors.top: parent.top
            anchors.bottom: threadColumn.bottom
            width: page.margin - 5
            opacity: 0.9
            visible: threadPosts.count > 1

            gradient: Gradient {
                GradientStop {
                    position: 0.0
                    color: guiSettings.threadStartColor(skywalker.getUserSettings().threadColor)
                }
                GradientStop {
                    position: 1.0
                    color: guiSettings.threadMidColor(skywalker.getUserSettings().threadColor)
                }
            }
        }
    }

    Text {
        id: draftsLink
        anchors.centerIn: parent
        font.pointSize: guiSettings.scaledFont(9/8)
        textFormat: Text.RichText
        text: qsTr(`<a href=\"drafts\" style=\"color: ${guiSettings.linkColor}\">Drafts</a>`)
        visible: threadPosts.count === 1 && !hasFullContent() && !replyToPostUri && !openedAsQuotePost && draftPosts.hasDrafts
        onLinkActivated: showDraftPosts()

        Accessible.role: Accessible.Link
        Accessible.name: UnicodeFonts.toPlainText(text)
        Accessible.onPressAction: showDraftPosts()
    }

    Text {
        anchors.top: draftsLink.bottom
        anchors.topMargin: 10
        anchors.horizontalCenter: parent.horizontalCenter
        font.pointSize: guiSettings.scaledFont(9/8)
        textFormat: Text.RichText
        text: qsTr(`<a href=\"card\" style=\"color: ${guiSettings.linkColor}\">Add anniversary card</a>`)
        visible: isAnniversary && threadPosts.count === 1 && !hasFullContent() && !replyToPostUri && !openedAsQuotePost
        onLinkActivated: addAnniversaryCard()

        Accessible.role: Accessible.Link
        Accessible.name: UnicodeFonts.toPlainText(text)
        Accessible.onPressAction: addAnniversaryCard()
    }

    footer: Rectangle {
        id: textFooter
        width: page.width
        height: getFooterHeight() + keyboardHandler.keyboardHeight
        z: guiSettings.footerZLevel
        color: guiSettings.footerColor

        function getFooterHeight() {
            return guiSettings.footerHeight + (replyToPostUri ? 0 : restrictionRow.height + footerSeparator.height)
        }

        Rectangle {
            id: footerSeparator
            width: parent.width
            height: replyToPostUri ? 0 : 1
            color: guiSettings.separatorColor
            visible: !replyToPostUri
        }

        Rectangle {
            id: restrictionRow
            anchors.top: footerSeparator.top
            width: parent.width
            height: replyToPostUri ? 0 : restrictionText.height + 10
            color: "transparent"
            visible: !replyToPostUri

            Accessible.role: Accessible.Link
            Accessible.name: getRestrictionsSpeech()
            Accessible.onPressAction: page.addReplyRestrictions()

            function getRestrictionsSpeech() {
                const speech = UnicodeFonts.toPlainText(restrictionText.text)
                return qsTr(`${speech}, press to change reply restrictions`)
            }

            SkySvg {
                id: restrictionIcon
                x: 10
                y: height + 3
                width: 20
                height: 20
                color: guiSettings.linkColor
                svg: restrictReply ? SvgOutline.replyRestrictions : SvgOutline.noReplyRestrictions

                Accessible.ignored: true
            }
            Text {
                id: restrictionText
                y: 5
                anchors.left: restrictionIcon.right
                anchors.right: parent.right
                leftPadding: 5
                color: guiSettings.linkColor
                font.italic: true
                font.pointSize: guiSettings.scaledFont(7/8)
                wrapMode: Text.Wrap
                text: getRestrictionText()

                Accessible.ignored: true

                function getRestrictionText() {
                    const replyRestricionText = getReplyRestrictionText()

                    if (allowQuoting)
                        return replyRestricionText + " " + qsTr("Quoting allowed.")

                    return replyRestricionText + " " + qsTr("Quoting disabled.")
                }

                function getReplyRestrictionText() {
                    if (!restrictReply)
                        return qsTr("Everyone can reply.")

                    let restrictionList = []
                    let allowNames = []

                    if (allowReplyMentioned)
                        allowNames.push(qsTr("mentioned"))
                    if (allowReplyFollower)
                        allowNames.push(qsTr("following"))
                    if (allowReplyFollowing)
                        allowNames.push(qsTr("followed"))

                    if (allowNames.length > 0) {
                        const allowText = guiSettings.toWordSequence(allowNames) + qsTr(" users")
                        restrictionList.push(allowText)
                    }

                    let listNames = []

                    for (let i = 0; i < allowLists.length; ++i) {
                        if (allowLists[i]) {
                            let model = skywalker.getListListModel(restrictionsListModelId)
                            const listView = model.getEntry(allowListIndexes[i])
                            const listName = UnicodeFonts.toCleanedHtml(listView.name)
                            listNames.push(`<b>${listName}</b>`)
                        }
                    }

                    if (listNames.length === 0)
                        allowListNamesFromDraft.forEach((listName) => listNames.push(`<b>${listName}</b>`))

                    if (listNames.length > 0) {
                        const names = guiSettings.toWordSequence(listNames)
                        restrictionList.push(qsTr(`members of ${names}`))
                    }

                    if (restrictionList.length === 0)
                        return qsTr("Replies disabled.")

                    const restrictedListText = guiSettings.toWordSequence(restrictionList)
                    return qsTr(`Only ${restrictedListText} can reply.`)
                }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: page.addReplyRestrictions()
            }
        }

        TextLengthBar {
            anchors.top: restrictionRow.bottom
            textField: currentPostItem() ? currentPostItem().getPostText() : null
        }

        SvgTransparentButton {
            id: addImage
            x: 10
            y: height + 5 + restrictionRow.height + footerSeparator.height
            accessibleName: qsTr("add picture")
            svg: SvgOutline.addImage
            enabled: page.canAddImage()

            onClicked: {
                const pickVideo = page.canAddVideo()

                if (Qt.platform.os === "android") {
                    pickingImage = postUtils.pickPhoto(pickVideo)
                } else {
                    fileDialog.pick(pickVideo)
                }
            }
        }

        AddGifButton {
            id: addGif
            x: addImage.x + addImage.width + 3
            y: height + 5 + restrictionRow.height + footerSeparator.height
            enabled: page.canAddGif()

            onSelectedGif: (gif) => currentPostItem().getGifAttachment().show(gif)
        }

        FontComboBox {
            id: fontSelector
            x: addGif.x + addGif.width + 8
            y: 5 + restrictionRow.height + footerSeparator.height + 6
            popup.height: Math.min(page.height - 20, popup.contentHeight)
            focusPolicy: Qt.NoFocus
        }

        LanguageComboBox {
            id: languageSelector
            allLanguages: languageUtils.languages
            usedLanguages: languageUtils.usedLanguages
            anchors.left: fontSelector.right
            anchors.leftMargin: 8
            y: 5 + restrictionRow.height + footerSeparator.height + 6
            popup.x: Math.max(-x, Math.min(0, page.width - popup.width - x))
            popup.height: Math.min(page.height - 20, popup.contentHeight)
            currentIndex: find(currentPostLanguage())
            reversedColors: languageUtils.isDefaultPostLanguageSet && currentValue === languageUtils.defaultPostLanguage
            autoDetectColor: currentPostLanguageSource() === QEnums.LANGUAGE_SOURCE_AUTO
            focusPolicy: Qt.NoFocus

            onActivated: (index) => {
                const value = valueAt(index)

                if (currentPostLanguage() === value)
                    return

                currentPostItem().language = value
                currentPostItem().languageSource = QEnums.LANGUAGE_SOURCE_USER
                console.debug("ACTIVATED LANG:", value)

                if (!languageUtils.getDefaultLanguageNoticeSeen()) {
                    guiSettings.notice(page,
                        qsTr("To set this language as default for your posts, you can press and hold the language button for a second."))
                    languageUtils.setDefaultLanguageNoticeSeen(true)
                }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: languageSelector.popup.open()
                onPressAndHold: {
                    if (languageUtils.isDefaultPostLanguageSet && languageSelector.currentValue === languageUtils.defaultPostLanguage)
                        languageUtils.defaultPostLanguage = ""
                    else
                        languageUtils.defaultPostLanguage = languageSelector.currentValue
                }
            }
        }

        SvgTransparentButton {
            id: linkButton
            anchors.left: languageSelector.right
            anchors.leftMargin: visible ? 8 : 0
            y: height + 5 + restrictionRow.height + footerSeparator.height
            width: visible ? height: 0
            accessibleName: qsTr("embed web link")
            svg: SvgOutline.link
            visible: getCursorInWebLink() >= 0 || getCursorInEmbeddedLink() >= 0
            onClicked: {
                if (getCursorInWebLink() >= 0)
                    page.addEmbeddedLink()
                else if (getCursorInEmbeddedLink() >= 0)
                    page.updateEmbeddedLink()
            }
        }

        SvgTransparentButton {
            id: contentWarningIcon
            anchors.left: linkButton.right
            anchors.leftMargin: 8
            y: height + 5 + restrictionRow.height + footerSeparator.height
            accessibleName: qsTr("add content warning")
            svg: hasContentWarning() ? SvgOutline.hideVisibility : SvgOutline.visibility
            visible: hasImageContent()
            onClicked: page.addContentWarning()
        }

        SvgButton {
            id: addPost
            y: 5 + restrictionRow.height + footerSeparator.height
            width: 34
            height: 34
            anchors.rightMargin: 10
            anchors.right: parent.right
            svg: SvgOutline.add
            accessibleName: qsTr("add post")
            enabled: hasFullContent() && threadPosts.count < maxThreadPosts
            focusPolicy: Qt.NoFocus
            onClicked: {
                Qt.inputMethod.commit()
                threadPosts.addPost(currentPostIndex)
            }
        }
    }

    StatusPopup {
        id: statusPopup
    }

    BusyIndicator {
        id: busyIndicator
        anchors.centerIn: parent
        running: false
    }

    ImageFileDialog {
        id: fileDialog
        onImageSelected: (fileUri) => photoPicked(fileUri)
        onVideoSelected: (fileUri) => videoPicked(fileUri)
    }

    TextSplitter {
        id: textSplitter
    }

    LinkCardReader {
        property list<var> linksToGet: []

        id: linkCardReader

        onLinkCard: (card) => {
            busyIndicator.running = false
            console.debug("Got card:", card.link, card.title, card.thumb)
            console.debug(card.description)
            linkCardTimer.immediateGetInProgress = false

            let postItem = threadPosts.itemAt(linkCardTimer.postIndex)

            if (postItem) {
                postItem.getLinkCard().show(card)
                let postText = postItem.getPostText()

                if (!postText.cursorInFirstWebLink)
                    postItem.getLinkCard().linkFixed = true
                else
                    postText.cutLinkIfJustAdded(postText.firstWebLink, () => postItem.getLinkCard().linkFixed = true)
            }

            getNextLink()
        }

        onLinkCardFailed: {
            busyIndicator.running = false
            console.debug("Failed to get link card")
            linkCardTimer.immediateGetInProgress = false
            getNextLink()
        }

        function getLink(postIndex, webLink) {
            if (!linkCardTimer.immediateGetInProgress) {
                linkCardTimer.getLinkImmediate(postIndex, webLink)
            }
            else {
                linksToGet.push([postIndex, webLink])
            }
        }

        function getNextLink() {
            if (linksToGet.length > 0) {
                const link = linksToGet.pop()
                getLink(link[0], link[1])
            }
        }
    }

    Timer {
        property int postIndex
        property string webLink
        property bool immediateGetInProgress: false

        id: linkCardTimer
        interval: 1000
        onTriggered: {
            busyIndicator.running = true
            linkCardReader.getLinkCard(webLink)
        }

        function startForLink(postIndex, webLink) {
            if (immediateGetInProgress) {
                console.debug("Immedate get in progress:", webLink)
                return
            }

            linkCardTimer.postIndex = postIndex
            linkCardTimer.webLink = webLink
            start()
        }

        function getLinkImmediate(postIndex, webLink) {
            stop()
            linkCardTimer.postIndex = postIndex
            linkCardTimer.webLink = webLink
            immediateGetInProgress = true
            linkCardReader.getLinkCard(webLink)
        }
    }

    PostUtils {
        property var callbackCanUploadVideo: () => {}
        property var callbackCannotUploadVideo: (error) => {}

        id: postUtils
        skywalker: page.skywalker // qmllint disable missing-type

        onPostOk: (uri, cid) => {
            postedUris.push(uri)

            if (!page.allowQuoting)
                postUtils.addPostgate(uri, true, [])

            if (page.sendingThreadPost > -1) {
                sendNextThreadPost(uri, cid)
            }
            else if (page.restrictReply) {
                const restrictionListUris = page.getReplyRestrictionListUris()
                const allowNobody = !page.allowReplyMentioned && !page.allowReplyFollower && !page.allowReplyFollowing && restrictionListUris.length === 0
                postUtils.addThreadgate(uri, cid, page.allowReplyMentioned, page.allowReplyFollower, page.allowReplyFollowing, restrictionListUris, allowNobody, [])
            }
            else {
                postDone()
            }
        }

        onPostFailed: (error) => page.postFailed(error)

        onThreadgateOk: {
            threadGateCreated = true

            if (page.sendingThreadPost > -1)
                sendNextThreadPost(threadFirstPostUri, threadFirstPostCid)
            else
                postDone()
        }

        onThreadgateFailed: (error) => page.postFailed(error)

        onPostgateFailed: (error) => statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)

        onPostProgress: (msg) => page.postProgress(msg)

        onPhotoPicked: (imgSource, gifTempFileName) => {
            pickingImage = false
            page.photoPicked(imgSource, gifTempFileName)
            currentPostItem().getPostText().forceActiveFocus()
        }

        onVideoPicked: (videoUrl) => {
            pickingImage = false
            editVideo(videoUrl)
        }

        onVideoPickedFailed: (error) => {
            pickingImage = false
            statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
            currentPostItem().getPostText().forceActiveFocus()
        }

        onPhotoPickFailed: (error) => {
            pickingImage = false
            statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
            currentPostItem().getPostText().forceActiveFocus()
        }

        onPhotoPickCanceled: {
            pickingImage = false
            currentPostItem().getPostText().forceActiveFocus()
        }

        onQuotePost: (uri, cid, text, author, timestamp) => { // qmllint disable signal-handler-parameters
            let postItem = currentPostItem()
            let postText = postItem.getPostText()

            if (!postText.firstPostLink)
                return

            postItem.quoteList = page.nullList
            postItem.quoteFeed = page.nullFeed
            postItem.quoteUri = uri
            postItem.quoteCid = cid
            postItem.quoteText = text
            postItem.quoteAuthor = author
            postItem.quoteDateTime = timestamp

            if (!postText.cursorInFirstPostLink)
                postItem.fixQuoteLink(true)
            else
                postText.cutLinkIfJustAdded(postText.firstPostLink, () => postItem.fixQuoteLink(true))
        }

        onQuoteFeed: (feed) => { // qmllint disable signal-handler-parameters
            let postItem = currentPostItem()
            let postText = postItem.getPostText()

            if (!postText.firstFeedLink)
                return

            if (postText.firstPostLink)
                return

            postItem.quoteList = page.nullList
            postItem.quoteFeed = feed

            if (!postText.cursorInFirstFeedLink)
                postItem.fixQuoteLink(true)
            else
                postText.cutLinkIfJustAdded(postText.firstFeedLink, () => postItem.fixQuoteLink(true))
        }

        onQuoteList: (list) => { // qmllint disable signal-handler-parameters
            let postItem = currentPostItem()
            let postText = postItem.getPostText()

            if (!postText.firstListLink)
                return

            if (postText.firstPostLink || postText.firstFeedLink)
                return

            postItem.quoteList = list

            if (!postText.cursorInFirstListLink)
                postItem.fixQuoteLink(true)
            else
                postText.cutLinkIfJustAdded(postText.firstListLink, () => postItem.fixQuoteLink(true))
        }

        onVideoUploadLimits: (limits) => { // qmllint disable signal-handler-parameters
            busyIndicator.running = false
            showVideoUploadLimits(limits)
        }

        function checkVideoLimits(cbOk, cbFailed) {
            callbackCanUploadVideo = cbOk
            callbackCannotUploadVideo = cbFailed
            checkVideoUploadLimits()
        }

        onCheckVideoLimitsOk: { // qmllint disable signal-handler-parameters
            callbackCanUploadVideo() // qmllint disable use-proper-function
            callbackCanUploadVideo = () => {}
            callbackCannotUploadVideo = (error) => {}
        }

        onCheckVideoLimitsFailed: (error) => {
            callbackCannotUploadVideo(error) // qmllint disable use-proper-function
            callbackCanUploadVideo = () => {}
            callbackCannotUploadVideo = (error) => {}
        }

        onLanguageIdentified: (languageCode, postIndex) => {
            let postItem = threadPosts.itemAt(postIndex)

            if (postItem && postItem.languageSource !== QEnums.LANGUAGE_SOURCE_USER && postItem.language !== languageCode) {
                postItem.language = languageCode

                // Trigger language source state change.
                if (postItem.languageSource === QEnums.LANGUAGE_SOURCE_AUTO)
                    postItem.languageSource = QEnums.LANGUAGE_SOURCE_NONE

                postItem.languageSource = QEnums.LANGUAGE_SOURCE_AUTO
            }
        }
    }

    Timer {
        property string prevText: ""

        id: languageIdentificationTimer
        interval: 500

        onTriggered: {
            let postItem = currentPostItem()

            if (!postItem)
                return

            if (postItem.langaugeSource === QEnums.LANGUAGE_SOURCE_USER)
                return

            let postText = postItem.getPostText()
            const text = postText.textWithoutLinks.trim().slice(0, maxLanguageIdentificationLength)

            if (text !== prevText) {
                prevText = text
                postUtils.identifyLanguage(prevText, currentPostIndex)
            }
        }

        function reset() {
            stop()
            prevText = ""
        }
    }

    GraphUtils {
        id: graphUtils
        skywalker: page.skywalker // qmllint disable missing-type

        onCachedList: getListNamesFromDraft()
    }

    VideoUtils {
        property var callbackOk: (videoSource) => {}
        property var callbackFailed: (error) => {}

        id: videoUtils
        skywalker: page.skywalker // qmllint disable missing-property

        onTranscodingOk: (inputFileName, outputFileName) => {
            const source = "file://" + outputFileName
            page.tmpVideos.push(source)
            callbackOk(source) // qmllint disable use-proper-function
            callbackOk = (videoSource) => {}
            callbackFailed = (error) => {}
        }

        onTranscodingFailed: (inputFileName, error) => {
            callbackFailed(error) // qmllint disable use-proper-function
            callbackOk = (videoSource) => {}
            callbackFailed = (error) => {}
        }

        function transcode(videoSource, newHeight, startMs, endMs, removeAudio, cbOk, cbFailed) {
            callbackOk = cbOk
            callbackFailed = cbFailed
            const fileName = videoSource.slice(7)
            transcodeVideo(fileName, newHeight, startMs, endMs, removeAudio)
            postProgress(qsTr("Transcoding video"))
        }
    }

    GifToVideoConverter {
        property var progressDialog

        id: gifToVideoConverter

        onConversionOk: (videoFileName) => {
            progressDialog.destroy()
            page.tmpVideos.push("file://" + videoFileName)
            editVideo(`file://${videoFileName}`)
        }

        onConversionFailed: (error) => {
            progressDialog.destroy()
            statusPopup.show(qsTr(`GIF conversion failed: ${error}`), QEnums.STATUS_LEVEL_ERROR)
        }

        onConversionProgress: (progress) => {
            if (progressDialog)
                progressDialog.setProgress(progress)
        }

        function start(gifFileName) {
            progressDialog = guiSettings.showProgress(page, qsTr("Converting GIF to Video"), () => doCancel())
            gifToVideoConverter.convert(gifFileName)
        }

        function doCancel() {
            gifToVideoConverter.cancel()
            let postItem = currentPostItem()

            if (postItem)
                postItem.getPostText().forceActiveFocus()
        }
    }

    DraftPosts {
        id: draftPosts
        skywalker: page.skywalker
        storageType: DraftPosts.STORAGE_FILE

        onSaveDraftPostOk: {
            statusPopup.show(qsTr("Saved post as draft"), QEnums.STATUS_LEVEL_INFO)
            page.closed()
        }

        onSaveDraftPostFailed: (error) => statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
        onUploadingImage: (seq) => statusPopup.show(qsTr(`Uploading image #${seq}`), QEnums.STATUS_LEVEL_INFO)
        onLoadDraftPostsFailed: (error) => statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
    }


    LanguageUtils {
        id: languageUtils
        skywalker: page.skywalker
    }

    Tenor {
        id: tenor
        skywalker: page.skywalker
    }

    MemeMaker {
        id: memeMaker
    }

    Timer {
        property string text
        property list<weblink> embeddedLinks
        property int index
        property int cursorPosition

        id: setPostTextTimer
        interval: 0
        onTriggered: {
            let postText = threadPosts.itemAt(index).getPostText()

            if (cursorPosition > -1)
                setCursorTimer.startSetCursor(index, cursorPosition)

            postText.text = text
            postText.setEmbeddedLinks(embeddedLinks)
        }

        function startSetText(text, embeddedLinks, index, cursorPosition = -1) {
            setPostTextTimer.text = text
            setPostTextTimer.embeddedLinks = embeddedLinks
            setPostTextTimer.index = index
            setPostTextTimer.cursorPosition = cursorPosition
            start()
        }
    }

    Timer {
        property int index
        property int cursorPosition

        id: setCursorTimer
        interval: 0
        onTriggered: {
            let postText = threadPosts.itemAt(index).getPostText()
            postText.cursorPosition = cursorPosition
            postText.forceActiveFocus()
        }

        function startSetCursor(index, cursorPosition) {
            setCursorTimer.index = index
            setCursorTimer.cursorPosition = cursorPosition
            start()
        }
    }

    Timer {
        id: focusTimer
        interval: 200
        onTriggered: {
            let postText = currentPostItem().getPostText()

            if (!postText.text.startsWith("\n#")) // hashtag post
                postText.cursorPosition = postText.text.length

            if (Boolean(page.initialVideo)) {
                editVideo(page.initialVideo)
            }
            else {
                postText.ensureVisible(Qt.rect(0, 0, postText.width, postText.height))
                postText.forceActiveFocus()
            }
        }
    }


    function getPostUtils() {
        return postUtils
    }

    function currentPostItem() {
        // Checking threadPosts.count here makes this function re-evaluate when count changes
        if (threadPosts.count === 0)
            console.debug("No thread posts available yet")

        return threadPosts.itemAt(currentPostIndex)
    }

    function postFailed(error) {
        busyIndicator.running = false
        statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)

        // Delete posts already posted (in a thread, or on failed thread gate creation)
        postUtils.batchDeletePosts(postedUris)

        // Clear all state so user can try to post again
        sendingThreadPost = -1
        threadRootUri = ""
        threadRootCid = ""
        threadFirstPostUri = ""
        threadFirstPostCid = ""
        threadGateCreated = false
        postedUris = []

        postButton.isPosting = false
    }

    function postProgress(msg) {
        busyIndicator.running = true

        if (sendingThreadPost < 0)
            statusPopup.show(msg, QEnums.STATUS_LEVEL_INFO, 300)
        else
            statusPopup.show(qsTr(`Post ${(sendingThreadPost + 1)}: ${msg}`), QEnums.STATUS_LEVEL_INFO, 300)
    }

    function checkAltText() {
        if (!requireAltText)
            return true

        for (let postIndex = 0; postIndex < threadPosts.count; ++postIndex) {
            const postItem = threadPosts.itemAt(postIndex)

            if (Boolean(postItem.video) && !Boolean(postItem.videoAltText))
                return false

            for (let i = 0; i < postItem.images.length; ++i) {
                if (i >= postItem.altTexts.length || postItem.altTexts[i].length === 0)
                    return false
            }
        }

        return true
    }

    // "file://" or "image://" source
    function photoPicked(source, gifTempFileName = "", altText = "") {
        if (gifTempFileName)
            page.tmpVideos.push("file://" + gifTempFileName)

        if (!canAddVideo()) {
            photoPickedContinued(source, altText)
            return
        }

        let gifFileName = gifTempFileName

        if (source.startsWith("file://") && source.endsWith(".gif"))
            gifFileName = source.slice(7)

        if (!gifFileName) {
            photoPickedContinued(source, altText)
            return
        }

        guiSettings.askConvertGif(
            page,
            "file://" + gifFileName,
            () => gifToVideoConverter.start(gifFileName),
            () => photoPickedContinued(source, altText))
    }

    function photoPickedContinued(source, altText = "") {
        console.debug("IMAGE:", source)
        let postItem = currentPostItem()

        if (!postItem)
            return

        postItem.altTexts.push(altText)
        postItem.images.push(source)
        postItem.memeTopTexts.push("")
        postItem.memeBottomTexts.push("")
        let scrollBar = postItem.getImageScroller().ScrollBar.horizontal
        scrollBar.position = 1.0 - scrollBar.size
        postItem.getPostText().forceActiveFocus()
    }

    function videoPicked(source, altText = "") {
        console.debug("VIDEO:", source)
        let postItem = currentPostItem()

        if (!postItem)
            return

        postItem.video = source
        postItem.videoAltText = altText
    }

    function addSharedText(text) {
        if (text) {
            let postItem = currentPostItem()

            if (!postItem)
                return

            postItem.getPostText().append(text)
        }
    }

    function addSharedPhoto(source, text) {
        let postItem = currentPostItem()

        if (!postItem)
            return

        if (!canAddImage()) {
            statusPopup.show(qsTr("Cannot add an image to this post."), QEnums.STATUS_LEVEL_INFO, 30)
            postUtils.dropPhoto(source)
            return
        }

        photoPicked(source)
        addSharedText(text)
    }

    function addSharedVideo(source, text) {
        let postItem = currentPostItem()

        if (!postItem)
            return

        if (!canAddVideo()) {
            statusPopup.show(qsTr("Cannot add video to this post."), QEnums.STATUS_LEVEL_INFO, 30)
            postUtils.dropVideo(source)
            return
        }

        addSharedText(text)
        editVideo(source)
    }

    function postDone() {
        busyIndicator.running = false
        page.closed()
    }

    function checkMisleadingEmbeddedLinks()
    {
        for (let i = 0; i < threadPosts.count; ++ i) {
            const postItem = threadPosts.itemAt(i)

            if (!postItem.getPostText().checkMisleadingEmbeddedLinks())
                return false
        }

        return true
    }

    function postsAreValid() {
        for (let i = 0; i < threadPosts.count; ++ i) {
            const postItem = threadPosts.itemAt(i)

            if (!postItem.isValid())
                return false
        }

        return true
    }

    function hasFullContent() {
        for (let i = 0; i < threadPosts.count; ++ i) {
            const postItem = threadPosts.itemAt(i)

            if (!postItem.hasContent())
                return false
        }

        return threadPosts.count > 0
    }

    function hasPartialContent() {
        for (let i = 0; i < threadPosts.count; ++ i) {
            const postItem = threadPosts.itemAt(i)

            if (postItem.hasContent())
                return true
        }

        return false
    }

    function cancel() {
        if (!hasPartialContent()) {
            page.closed()
            return
        }

        if (draftPosts.canSaveDraft()) {
            guiSettings.askDiscardSaveQuestion(
                    page,
                    qsTr("Do you want to discard your post or save it as draft?"),
                    () => page.closed(),
                    () => page.saveDraftPost())
        }
        else {
            guiSettings.askYesNoQuestion(
                    page,
                    qsTr("Do you want to to discard your post?<br>You cannot save more drafts."),
                    () => page.closed())
        }
    }

    function getPostCountText(postIndex, postCount) {
        return `${UnicodeFonts.toCleanedHtml(threadPrefix)}${(postIndex + 1)}/${postCount}`
    }

    function editThreadPrefix() {
        let component = guiSettings.createComponent("EditThreadPrefix.qml")
        let dialog = component.createObject(page, { prefix: threadPrefix })

        dialog.onAccepted.connect(() => {
            threadPrefix = dialog.getPrefix()
            skywalker.getUserSettings().setThreadPrefix(threadPrefix)
            dialog.destroy()
        })

        dialog.onRejected.connect(() => dialog.destroy())
        dialog.open()
    }

    function getImagesToSend(postItem) {
        let images = []

        for (let i = 0; i < postItem.images.length; ++i) {
            if (postItem.imageHasMeme(i)) {
                if (!memeMaker.setOrigImage(postItem.images[i])) {
                    console.warn("Cannot load image:", postItem.images[i])
                    images.push(postItem.images[i])
                    continue
                }

                memeMaker.topText = postItem.memeTopTexts[i]
                memeMaker.bottomText = postItem.memeBottomTexts[i]
                images.push(memeMaker.memeImgSource)
                page.tmpImages.push(memeMaker.memeImgSource)
                memeMaker.releaseMemeOwnership()
            }
            else {
                images.push(postItem.images[i])
            }
        }

        return images
    }

    function sendSinglePost(postItem, parentUri, parentCid, rootUri, rootCid, postIndex, postCount) {
        const qUri = postItem.getQuoteUri()
        const qCid = postItem.getQuoteCid()
        const labels = postItem.getContentLabels()

        let postText = postItem.text

        if (threadAutoNumber && postCount > 1)
            postText += `\n${getPostCountText(postIndex, postCount)}`

        if (postItem.card) {
            postUtils.post(postText,
                           postItem.card,
                           parentUri, parentCid,
                           rootUri, rootCid,
                           qUri, qCid,
                           postItem.embeddedLinks,
                           labels, postItem.language)
        } else if (!postItem.gif.isNull()) {
            tenor.registerShare(postItem.gif)

            let gifCard = linkCardReader.makeLinkCard(
                    postItem.gif.getUrlForPosting(),
                    `${postItem.gif.description} (via Tenor)\nPosted from Skywalker ${guiSettings.skywalkerHandle}`,
                    qsTr("This GIF has been posted from Skywalker for Android. " +
                         "Get Skywalker from Google Play.") +
                         (`<br>Bluesky: ${guiSettings.skywalkerHandle}`),
                    postItem.gif.imageUrl)

            postUtils.post(postText, gifCard,
                           parentUri, parentCid,
                           rootUri, rootCid,
                           qUri, qCid,
                           postItem.embeddedLinks,
                           labels, postItem.language)
        } else if (Boolean(postItem.video)) {
            postUtils.checkVideoLimits(
                () => videoUtils.transcode(postItem.video, postItem.videoNewHeight,
                        postItem.videoStartMs, postItem.videoEndMs, postItem.videoRemoveAudio,
                        (transcodedVideo) => {
                            postUtils.postVideo(postText, transcodedVideo, postItem.videoAltText,
                            parentUri, parentCid,
                            rootUri, rootCid,
                            qUri, qCid,
                            postItem.embeddedLinks,
                            labels, postItem.language) },
                        (error) => postFailed(error)),
                (error) => postFailed(error))
        } else {
            const images = getImagesToSend(postItem)
            postUtils.post(postText, images, postItem.altTexts,
                           parentUri, parentCid,
                           rootUri, rootCid,
                           qUri, qCid,
                           postItem.embeddedLinks,
                           labels, postItem.language);
        }

        postUtils.cacheTags(postItem.text)
        languageUtils.addUsedPostLanguage(postItem.language)
    }

    function sendThreadPosts(postIndex, parentUri, parentCid, rootUri, rootCid) {
        if (postIndex >= threadPosts.postList.length) {
            console.debug("Done posting thread")
            postDone()
            return
        }

        console.debug(`Send thread post ${postIndex}`)
        sendingThreadPost = postIndex
        let postItem = threadPosts.postList[postIndex]
        sendSinglePost(postItem, parentUri, parentCid, rootUri, rootCid, postIndex, threadPosts.count)
    }

    function sendNextThreadPost(prevUri, prevCid) {
        if (sendingThreadPost === 0) {
            threadFirstPostUri = prevUri
            threadFirstPostCid = prevCid

            if (restrictReply && !threadGateCreated) {
                const restrictionListUris = getReplyRestrictionListUris()
                const allowNobody = !allowReplyMentioned && !allowReplyFollower && !allowReplyFollowing && restrictionListUris.length === 0
                postUtils.addThreadgate(prevUri, prevCid, allowReplyMentioned, allowReplyFollower, allowReplyFollowing,
                                        restrictionListUris, allowNobody, [])
                return
            }

            if (replyRootPostUri) {
                threadRootUri = replyRootPostUri
                threadRootCid = replyRootPostCid
            }
            else if (replyToPostUri) {
                threadRootUri = replyToPostUri
                threadRootCid = replyToPostCid
            }
            else {
                threadRootUri = prevUri
                threadRootCid = prevCid
            }
        }

        sendThreadPosts(sendingThreadPost + 1, prevUri, prevCid, threadRootUri, threadRootCid)
    }

    function saveDraftPost() {
        threadPosts.copyPostItemsToPostList()
        const postItem = threadPosts.postList[0]
        const qUri = postItem.getQuoteUri()
        const qCid = postItem.getQuoteCid()
        const labels = postItem.getContentLabels()

        const draft = draftPosts.createDraft(
                        postItem.text,
                        postItem.embeddedLinks,
                        postItem.images, postItem.altTexts,
                        postItem.memeTopTexts, postItem.memeBottomTexts,
                        postItem.video, postItem.videoAltText,
                        postItem.videoStartMs, postItem.videoEndMs, postItem.videoNewHeight,
                        postItem.videoRemoveAudio,
                        replyToPostUri, replyToPostCid,
                        replyRootPostUri, replyRootPostCid,
                        replyToAuthor, UnicodeFonts.toPlainText(replyToPostText),
                        replyToPostDateTime,
                        qUri, qCid, postItem.quoteAuthor, UnicodeFonts.toPlainText(postItem.quoteText),
                        postItem.quoteDateTime, postItem.quoteFixed,
                        postItem.quoteFeed, postItem.quoteList,
                        postItem.gif, postItem.card, labels, postItem.language,
                        restrictReply, allowReplyMentioned, allowReplyFollower, allowReplyFollowing,
                        getReplyRestrictionListUris(), !allowQuoting)

        let draftItemList = []

        for (let i = 1; i < threadPosts.postList.length; ++i) {
            const threadItem = threadPosts.postList[i]
            const qUriItem = threadItem.getQuoteUri()
            const qCidItem = threadItem.getQuoteCid()
            const labelsItem = threadItem.getContentLabels()

            const draftItem = draftPosts.createDraft(
                                threadItem.text,
                                threadItem.embeddedLinks,
                                threadItem.images, threadItem.altTexts,
                                threadItem.memeTopTexts, threadItem.memeBottomTexts,
                                threadItem.video, threadItem.videoAltText,
                                threadItem.videoStartMs, threadItem.videoEndMs, threadItem.videoNewHeight,
                                threadItem.videoRemoveAudio,
                                "", "",
                                "", "",
                                nullAuthor, "",
                                new Date(),
                                qUriItem, qCidItem, threadItem.quoteAuthor, UnicodeFonts.toPlainText(threadItem.quoteText),
                                threadItem.quoteDateTime, threadItem.quoteFixed,
                                threadItem.quoteFeed, threadItem.quoteList,
                                threadItem.gif, threadItem.card, labelsItem, threadItem.language,
                                false, false, false, false,
                                [], false)

            draftItemList.push(draftItem)
            postUtils.cacheTags(threadItem.text)
            languageUtils.addUsedPostLanguage(threadItem.language)
        }

        draftPosts.saveDraftPost(draft, draftItemList)
        postUtils.cacheTags(postItem.text)
        languageUtils.addUsedPostLanguage(postItem.language)
    }

    function showDraftPosts() {
        let component = guiSettings.createComponent("DraftPostsView.qml")
        let draftsPage = component.createObject(page, { model: draftPosts.getDraftPostsModel() })
        draftsPage.onClosed.connect(() => root.popStack())
        draftsPage.onSelected.connect((index) => {
            const draftDataList = draftPosts.getDraftPostData(index)
            setDraftPost(draftDataList)
            draftPosts.removeDraftPost(index)
            root.popStack()
        })
        draftsPage.onDeleted.connect((index) => draftPosts.removeDraftPost(index))

        root.pushStack(draftsPage)
    }

    function addAnniversaryCard() {
        let component = guiSettings.createComponent("AnniversaryCardMaker.qml")
        let cardPage = component.createObject(page)
        cardPage.onCanceled.connect(() => root.popStack())
        cardPage.onAddCard.connect((source, years) => {
            page.photoPicked(source, "", qsTr(`Bluesky anniversary card sent with ${guiSettings.skywalkerHandle}`))
            page.addSharedText(qsTr(`Today is my ${years} year Bluesky anniversary 🥳`))
            root.popStack()
        })
        root.pushStack(cardPage)
    }

    function setDraftPost(draftDataList) {
        for (let j = 0; j < draftDataList.length; ++j) {
            const draftData = draftDataList[j]
            let postItem = threadPosts.newComposePostItem()
            postItem.text = draftData.text
            postItem.embeddedLinks = draftData.embeddedLinks

            for (let i = 0; i < draftData.images.length; ++i) {
                postItem.images.push(draftData.images[i].fullSizeUrl)
                postItem.altTexts.push(draftData.images[i].alt)
                postItem.memeTopTexts.push(draftData.images[i].memeTopText)
                postItem.memeBottomTexts.push(draftData.images[i].memeBottomText)
            }

            if (!draftData.video.isNull())
            {
                postItem.video = draftData.video.playlistUrl
                postItem.videoAltText = draftData.video.alt
                postItem.videoStartMs = draftData.video.startMs
                postItem.videoEndMs = draftData.video.endMs
                postItem.videoNewHeight = draftData.video.newHeight
                postItem.videoRemoveAudio = draftData.video.removeAudio
            }

            if (j === 0) {
                replyToAuthor = draftData.replyToAuthor
                replyToPostUri = draftData.replyToUri
                replyToPostCid = draftData.replyToCid
                replyRootPostUri = draftData.replyRootUri
                replyRootPostCid = draftData.replyRootCid
                replyToPostText = draftData.replyToText
                replyToPostDateTime = draftData.replyToDateTime

                openedAsQuotePost = draftData.openAsQuotePost

                restrictReply = draftData.restrictReplies
                allowReplyMentioned = draftData.allowMention
                allowReplyFollower = draftData.allowFollower
                allowReplyFollowing = draftData.allowFollowing
                allowListUrisFromDraft = draftData.allowLists
                allowListIndexes = [0, 1, 2]
                allowLists = [false, false, false]
                allowQuoting = !draftData.embeddingDisabled
            }

            postItem.quoteUri = draftData.quoteUri
            postItem.quoteCid = draftData.quoteCid
            postItem.quoteAuthor = draftData.quoteAuthor
            postItem.quoteText = draftData.quoteText
            postItem.quoteDateTime = draftData.quoteDateTime
            postItem.quoteFixed = draftData.quoteFixed
            postItem.quoteFeed = draftData.quoteFeed
            postItem.quoteList = draftData.quoteList

            postItem.gif = draftData.gif

            if (draftData.externalLink)
                linkCardReader.getLink(j, draftData.externalLink)

            postItem.setContentWarnings(draftData.labels)
            postItem.language = draftData.language

            console.debug("ADD DRAFT:", j)
            if (j > 0)
                threadPosts.postList.push(postItem)
            else
                threadPosts.postList[0] = postItem
            console.debug("ADDED DRAFT:", j)
        }

        threadPosts.model = draftDataList.length
        threadPosts.copyPostListToPostItems()
    }

    function createRestrictionListModel() {
        if (restrictionsListModelId < 0) {
            restrictionsListModelId = skywalker.createListListModel(QEnums.LIST_TYPE_ALL, QEnums.LIST_PURPOSE_CURATE, userDid)
            skywalker.getListList(restrictionsListModelId, 100)
        }
    }

    function addReplyRestrictions() {
        createRestrictionListModel()

        let component = guiSettings.createComponent("AddReplyRestrictions.qml")
        let restrictionsPage = component.createObject(page, {
                rootUri: "",
                postUri: "",
                restrictReply: page.restrictReply,
                allowMentioned: page.allowReplyMentioned,
                allowFollower: page.allowReplyFollower,
                allowFollowing: page.allowReplyFollowing,
                allowLists: page.allowLists,
                allowListIndexes: page.allowListIndexes,
                allowListUrisFromDraft: page.allowListUrisFromDraft,
                listModelId: page.restrictionsListModelId,
                allowQuoting: page.allowQuoting,
                allowSaveAsDefault: true
        })
        restrictionsPage.onAccepted.connect(() => {
                page.restrictReply = restrictionsPage.restrictReply
                page.allowReplyMentioned = restrictionsPage.allowMentioned
                page.allowReplyFollower = restrictionsPage.allowFollower
                page.allowReplyFollowing = restrictionsPage.allowFollowing
                page.allowLists = restrictionsPage.allowLists
                page.allowListIndexes = restrictionsPage.allowListIndexes
                page.allowQuoting = restrictionsPage.allowQuoting
                allowListUrisFromDraft = []
                restrictionsPage.destroy()
        })
        restrictionsPage.onRejected.connect(() => restrictionsPage.destroy())
        restrictionsPage.open()
    }

    function getReplyRestrictionListUris() {
        if (allowListUrisFromDraft.length > 0)
            return allowListUrisFromDraft

        return root.getReplyRestrictionListUris(restrictionsListModelId, allowLists, allowListIndexes)
    }

    function getListNamesFromDraft() {
        console.debug("Get list names, uris:", allowListUrisFromDraft)

        if (allowListUrisFromDraft.length == 0) {
            allowListNamesFromDraft = []
            return
        }

        let names = []
        let listModel = skywalker.getListListModel(restrictionsListModelId)

        for (let i = 0; i < allowListUrisFromDraft.length; ++i) {
            const uri = allowListUrisFromDraft[i]
            const listView = graphUtils.getCachedListView(uri)

            if (listView.isNull())
                names.push(uri)
            else
                names.push(listView.name)
        }

        console.debug("Get list names:", names)
        allowListNamesFromDraft = names
    }

    function addContentWarning() {
        let postItem = currentPostItem()

        if (!postItem)
            return

        let component = guiSettings.createComponent("AddContentWarning.qml")
        let cwPage = component.createObject(page, {
                suggestive: postItem.cwSuggestive,
                nudity: postItem.cwNudity,
                porn: postItem.cwPorn,
                gore: postItem.cwGore
        })
        cwPage.onAccepted.connect(() => {
                postItem.cwSuggestive = cwPage.suggestive
                postItem.cwNudity = cwPage.nudity
                postItem.cwPorn = cwPage.porn
                postItem.cwGore = cwPage.gore
                cwPage.destroy()
        })
        cwPage.onRejected.connect(() => cwPage.destroy())
        cwPage.open()
    }

    function addEmbeddedLink() {
        let postItem = currentPostItem()

        if (!postItem)
            return -1

        const postText = postItem.getPostText()
        const webLinkIndex = postText.cursorInWebLink

        if (webLinkIndex < 0)
            return

        console.debug("Web link index:", webLinkIndex, "size:", postText.webLinks.length)
        const webLink = postText.webLinks[webLinkIndex]
        console.debug("Web link:", webLink.link)
        let component = guiSettings.createComponent("EditEmbeddedLink.qml")
        let linkPage = component.createObject(page, {
                link: webLink.link,
                canAddLinkCard: !Boolean(postItem.getLinkCard().card)
        })
        linkPage.onAccepted.connect(() => {
                const name = linkPage.getName()
                const addLinkCard = linkPage.addLinkCard
                linkPage.destroy()

                if (name.length > 0)
                    postText.addEmbeddedLink(webLinkIndex, name)

                if (addLinkCard)
                    linkCardTimer.getLinkImmediate(currentPostIndex, webLink.link)

                postText.forceActiveFocus()
        })
        linkPage.onRejected.connect(() => {
                linkPage.destroy()
                postText.forceActiveFocus()
        })
        linkPage.open()
    }

    function updateEmbeddedLink() {
        let postItem = currentPostItem()

        if (!postItem)
            return -1

        const postText = postItem.getPostText()
        const linkIndex = postText.cursorInEmbeddedLink

        if (linkIndex < 0)
            return

        console.debug("Embedded link index:", linkIndex, "size:", postText.embeddedLinks.length)
        const link = postText.embeddedLinks[linkIndex]
        const error = link.hasMisleadingName() ? link.getMisleadingNameError() :
                            (link.isTouchingOtherLink() ? qsTr("Connected to previous link") : "")
        console.debug("Embedded link:", link.link, "name:", link.name, "error:", error)

        let component = guiSettings.createComponent("EditEmbeddedLink.qml")
        let linkPage = component.createObject(page, {
                link: link.link,
                name: link.name,
                error: error,
                canAddLinkCard: !Boolean(postItem.getLinkCard().card)
        })
        linkPage.onAccepted.connect(() => {
                const name = linkPage.getName()
                const addLinkCard = linkPage.addLinkCard
                linkPage.destroy()

                if (name.length <= 0)
                    postText.removeEmbeddedLink(linkIndex)
                else
                    postText.updateEmbeddedLink(linkIndex, name)

                if (addLinkCard)
                    linkCardTimer.getLinkImmediate(currentPostIndex, link.link)

                postText.forceActiveFocus()
        })
        linkPage.onRejected.connect(() => {
                linkPage.destroy()
                postText.forceActiveFocus()
        })
        linkPage.open()
    }

    function getCursorInWebLink() {
        let postItem = currentPostItem()

        if (!postItem)
            return -1

        return postItem.getPostText().cursorInWebLink
    }

    function getCursorInEmbeddedLink() {
        let postItem = currentPostItem()

        if (!postItem)
            return -1

        return postItem.getPostText().cursorInEmbeddedLink
    }

    function hasImageContent() {
        let postItem = currentPostItem()

        if (!postItem)
            return false

        return !postItem.getGifAttachment().gif.isNull() ||
                (postItem.getLinkCard().card && postItem.getLinkCard().card.thumb) ||
                postItem.images.length > 0 ||
                Boolean(postItem.video)
    }

    function canAddImage() {
        const postItem = currentPostItem()

        if (!postItem)
            return false

        return postItem.images.length < maxImages &&
                postItem.video.length === 0 &&
                threadPosts.postList[currentPostIndex].gif.isNull() &&
                !threadPosts.postList[currentPostIndex].card &&
                !pickingImage
    }

    function canAddVideo() {
        const postItem = currentPostItem()

        if (!postItem)
            return false

        return canAddImage() && postItem.images.length === 0
    }

    function canAddGif() {
        const postItem = currentPostItem()

        if (!postItem)
            return false

        return threadPosts.postList[currentPostIndex].gif.isNull() &&
                !threadPosts.postList[currentPostIndex].card &&
                postItem.images.length === 0 &&
                postItem.video.length === 0
    }

    function currentPostLanguage() {
        const postItem = currentPostItem()

        if (!postItem)
            return languageUtils.defaultPostLanguage

        return postItem.language ? postItem.language : languageUtils.defaultPostLanguage
    }

    function currentPostLanguageSource() {
        const postItem = currentPostItem()

        if (!postItem)
            return QEnums.LANGUAGE_SOURCE_NONE

        return postItem.languageSource
    }

    function hasContentWarning() {
        const postItem = currentPostItem()

        if (!postItem)
            return false

        return hasImageContent() && (postItem.cwSuggestive || postItem.cwNudity || postItem.cwPorn || postItem.cwGore)
    }

    function editVideo(videoSource, startMs = 0, endMs = 0, removeAudio = false, newHeight = 0) {
        console.debug("Edit video, start:", startMs, "end:", endMs, "height:", newHeight)
        let component = guiSettings.createComponent("VideoEditor.qml")
        let videoPage = component.createObject(page, {
                videoSource: videoSource,
                startMs: startMs,
                endMs: endMs,
                newHeight: newHeight,
                removeAudio: removeAudio
        })
        videoPage.onVideoEdited.connect((newHeight, startMs, endMs, removeAudio) => {
            const postItem = currentPostItem()
            let altText = ""

            if (postItem) {
                postItem.videoNewHeight = newHeight
                postItem.videoStartMs = startMs
                postItem.videoEndMs = endMs
                postItem.videoRemoveAudio = removeAudio
                altText = postItem.videoAltText
            }

            page.videoPicked(videoSource, altText)
            root.popStack()
            currentPostItem().getPostText().forceActiveFocus()
        })
        videoPage.onCancel.connect(() => {
            root.popStack()
            currentPostItem().getPostText().forceActiveFocus()
        })
        root.pushStack(videoPage)
    }

    function showVideoUploadLimits(limits) {
        let component = guiSettings.createComponent("VideoUploadLimits.qml")
        let limitsPage = component.createObject(page, { limits: limits })
        limitsPage.onAccepted.connect(() => limitsPage.destroy())
        limitsPage.open()
    }

    VirtualKeyboardHandler {
        id: keyboardHandler
    }

    Component.onDestruction: {
        for (let i = 0; i < threadPosts.count; ++i) {
            let postItem = threadPosts.itemAt(i)
            postItem.images.forEach((value, index, array) => { postUtils.dropPhoto(value); })

            if (Boolean(postItem.video))
                postUtils.dropVideo(postItem.video)

        if (initialVideo)
            postUtils.dropVideo(initialVideo)

        }

        page.tmpImages.forEach((value, index, array) => { postUtils.dropPhoto(value); })
        page.tmpVideos.forEach((value, index, array) => { postUtils.dropVideo(value); })

        if (initialImage)
            getPostUtils().dropPhoto(initialImage)
        if (restrictionsListModelId >= 0)
            skywalker.removeListListModel(restrictionsListModelId)

        draftPosts.removeDraftPostsModel()
    }

    Component.onCompleted: {
        // Wait a bit for the window to render.
        // Then make sue the text field is in the visible area.
        focusTimer.start()

        const postInteractionSettings = postUtils.getPostInteractionSettings()
        allowReplyMentioned = postInteractionSettings.allowMention
        allowReplyFollower = postInteractionSettings.allowFollower
        allowReplyFollowing = postInteractionSettings.allowFollowing
        allowListUrisFromDraft = postInteractionSettings.allowListUris
        restrictReply = postInteractionSettings.allowNobody || allowReplyMentioned || allowReplyFollower || allowReplyFollowing || allowListUrisFromDraft.length > 0
        allowQuoting = !postInteractionSettings.disableEmbedding

        threadPosts.copyPostListToPostItems()
        draftPosts.loadDraftPosts()
    }
}
