import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window 2.2
import skywalker

Page {
    required property var skywalker
    property string initialText
    property string initialImage
    property int margin: 15

    readonly property int maxPostLength: 300
    readonly property int maxThreadPosts: 99
    readonly property int minPostSplitLineLength: 30
    readonly property int maxImages: 4 // per post
    property bool pickingImage: false

    // Reply restrictions (on post thread)
    property bool restrictReply: false
    property bool allowReplyMentioned: false
    property bool allowReplyFollowing: false
    property list<int> allowListIndexes: [0, 1, 2]
    property list<bool> allowLists: [false, false, false]
    property list<string> allowListUrisFromDraft: []
    property int restrictionsListModelId: -1

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
    property bool threadAutoNumber: skywalker.getUserSettings().getThreadAutoNumber()
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

    signal closed

    id: page
    width: parent.width
    height: parent.height
    contentHeight: flick.height
    topPadding: 0
    bottomPadding: 10

    header: Rectangle {
        width: parent.width
        height: guiSettings.headerHeight
        z: guiSettings.headerZLevel
        color: guiSettings.headerColor

        SvgButton {
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            svg: svgOutline.cancel
            accessibleName: qsTr("cancel posting")
            onClicked: page.cancel()
        }

        Avatar {
            anchors.centerIn: parent
            height: parent.height - 10
            width: height
            avatarUrl: skywalker.avatarUrl
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

        SvgButton {
            id: moreOptions
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            svg: svgOutline.moreVert
            accessibleName: qsTr("more options")
            onClicked: moreMenu.open()

            Menu {
                id: moreMenu
                modal: true

                onAboutToShow: root.enablePopupShield(true)
                onAboutToHide: root.enablePopupShield(false)

                CloseMenuItem {
                    text: qsTr("<b>Options</b>")
                    Accessible.name: qsTr("close more options menu")
                }
                AccessibleMenuItem {
                    text: qsTr("Require ALT text")
                    checkable: true
                    checked: skywalker.getUserSettings().getRequireAltText(userDid)

                    onToggled:{
                        requireAltText = checked
                        skywalker.getUserSettings().setRequireAltText(userDid, checked)
                    }
                }
                AccessibleMenuItem {
                    text: qsTr("Auto number")
                    checkable: true
                    checked: skywalker.getUserSettings().getThreadAutoNumber()

                    onToggled: {
                        threadAutoNumber = checked
                        skywalker.getUserSettings().setThreadAutoNumber(checked)
                    }
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
            anchors.fill: replyToColumn
            border.width: 2
            border.color: guiSettings.borderColor
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
                        images: initialImage ? [initialImage] : []
                        altTexts: initialImage ? [""] : []
                        quoteAuthor: page.quoteAuthor
                        quoteUri: page.quoteUri
                        quoteCid: page.quoteCid
                        quoteText: page.quoteText
                        quoteDateTime: page.quoteDateTime
                        language: replyToLanguage ? replyToLanguage : languageUtils.defaultPostLanguage
                    }
                ]

                id: threadPosts
                width: parent.width
                model: 1

                Item {
                    required property int index
                    property string text
                    property list<string> images
                    property list<string> altTexts
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

                    function copyToPostList() {
                        threadPosts.postList[index].text = text
                        threadPosts.postList[index].images = images
                        threadPosts.postList[index].altTexts = altTexts
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
                    }

                    function copyFromPostList() {
                        images = threadPosts.postList[index].images
                        altTexts = threadPosts.postList[index].altTexts
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

                        // Set text last as it will trigger link extractions which
                        // will check if a link card is already in place.
                        text = threadPosts.postList[index].text
                    }

                    function getPostText() { return postText }
                    function getImageScroller() { return imageScroller }
                    function getGifAttachment() { return gifAttachment }
                    function getLinkCard() { return linkCard }

                    function hasAttachment() {
                        return imageScroller.images.length > 0 ||
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

                        onTextChanged: {
                            postItem.text = text

                            if (threadAutoSplit && graphemeLength > maxLength && !splitting) {
                                console.debug("SPLIT:", index)

                                // Avoid to re-split when the post count text becomes visible or longer
                                const maxPartLength = page.maxPostLength - postCountText.maxSize()
                                const parts = unicodeFonts.splitText(text, maxPartLength, minPostSplitLineLength, 2)

                                if (parts.length > 1) {
                                    const moveCursor = cursorPosition > parts[0].length && index === currentPostIndex
                                    const oldCursorPosition = cursorPosition

                                    splitting = true
                                    text = parts[0].trim()

                                    if (!moveCursor && index === currentPostIndex)
                                        cursorPosition = oldCursorPosition

                                    splitting = false

                                    if (index === threadPosts.count - 1 || threadPosts.itemAt(index + 1).hasAttachment()) {
                                        threadPosts.addPost(index, parts[1], moveCursor)
                                    }
                                    else {
                                        // Prepend excess text to next post
                                        let nextPostText = threadPosts.itemAt(index + 1).getPostText()
                                        const newText = joinPosts(parts[1], nextPostText.text)
                                        const newCursorPosition = moveCursor ? oldCursorPosition - parts[0].length : -1

                                        setPostTextTimer.startSetText(newText, index + 1, newCursorPosition)

                                        if (moveCursor)
                                            currentPostIndex = index + 1
                                    }
                                }
                            }
                        }

                        onFocusChanged: {
                            if (focus)
                                currentPostIndex = index
                        }

                        onFirstWebLinkChanged: {
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
                        svg: svgOutline.remove
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

                        id: imageScroller
                        width: page.width
                        anchors.top: postCountText.visible ? postCountText.bottom : postText.bottom
                        horizontalPadding: page.margin
                        requireAltText: page.requireAltText
                        postUtils: page.getPostUtils()
                        visible: !linkCard.visible && !gifAttachment.visible
                    }

                    // GIF attachment
                    AnimatedImage {
                        property tenorgif gif

                        id: gifAttachment
                        x: page.margin
                        width: Math.min(gif ? gif.smallSize.width : 1, page.width - 2 * page.margin)
                        anchors.top: imageScroller.bottom
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
                            svg: svgOutline.close
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
                            svg: svgOutline.close
                            accessibleName: qsTr("remove link card")
                            onClicked: linkCard.hide()
                        }
                    }

                    // Quote post
                    Rectangle {
                        anchors.fill: quoteColumn
                        border.width: 2
                        border.color: guiSettings.borderColor
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
                        anchors.fill: quoteFeedColumn
                        border.width: 2
                        border.color: guiSettings.borderColor
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
                        anchors.fill: quoteListColumn
                        border.width: 2
                        border.color: guiSettings.borderColor
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
                    let component = Qt.createComponent("ComposePostItem.qml")
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

                function addPost(index, text = "", focus = true) {
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
                        setPostTextTimer.startSetText(text, index + 1)

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

                function mergePostsAt(index) {
                    if (index === count - 1)
                        return index

                    let text = itemAt(index).getPostText().text
                    let endIndex = index + 1

                    while (endIndex < count) {
                        let nextPost = itemAt(endIndex)

                        if (nextPost.hasAttachment())
                            break

                        text = joinPosts(text, nextPost.getPostText().text)
                        ++endIndex
                    }

                    if (endIndex === index + 1)
                        return index

                    const maxLength = itemAt(endIndex - 1).getPostText().maxLength

                    for (let i = index + 1; i < endIndex; ++i)
                        threadPosts.removePost(index + 1)

                    const parts = unicodeFonts.splitText(text, maxLength, page.minPostSplitLineLengths)
                    threadPosts.itemAt(index).getPostText().text = parts[0].trim()

                    for (let j = 1; j < parts.length; ++j) {
                        threadPosts.addPost(index + j - 1, "", false)
                        threadPosts.itemAt(index + j).getPostText().text = parts[j].trim()
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
        Accessible.name: unicodeFonts.toPlainText(text)
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
        Accessible.name: unicodeFonts.toPlainText(text)
        Accessible.onPressAction: addAnniversaryCard()
    }

    footer: Rectangle {
        id: textFooter
        width: page.width
        height: getFooterHeight()
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
                const speech = unicodeFonts.toPlainText(restrictionText.text)
                return qsTr(`${speech}, press to change reply restrictions`)
            }

            SvgImage {
                id: restrictionIcon
                x: 10
                y: height + 3
                width: 20
                height: 20
                color: guiSettings.linkColor
                svg: restrictReply ? svgOutline.replyRestrictions : svgOutline.noReplyRestrictions

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
                    if (!restrictReply)
                        return qsTr("Everyone can reply")

                    let restrictionList = []

                    if (allowReplyMentioned && allowReplyFollowing)
                        restrictionList.push(qsTr("mentioned and followed users"))
                    else if (allowReplyMentioned)
                        restrictionList.push(qsTr("mentioned users"))
                    else if (allowReplyFollowing)
                        restrictionList.push(qsTr("followed users"))

                    let listNames = []

                    for (let i = 0; i < allowLists.length; ++i) {
                        if (allowLists[i]) {
                            let model = skywalker.getListListModel(restrictionsListModelId)
                            const listView = model.getEntry(allowListIndexes[i])
                            const listName = unicodeFonts.toCleanedHtml(listView.name)
                            listNames.push(`<b>${listName}</b>`)
                        }
                    }

                    if (listNames.length === 0)
                        listNames = allowListUrisFromDraft

                    if (listNames.length > 0) {
                        const names = guiSettings.toWordSequence(listNames)
                        restrictionList.push(qsTr(`members of ${names}`))
                    }

                    if (restrictionList.length === 0)
                        return qsTr("Replies disabled")

                    const restrictedListText = guiSettings.toWordSequence(restrictionList)
                    return qsTr(`Only ${restrictedListText} can reply`)
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
            svg: svgOutline.addImage
            enabled: page.canAddImage()

            onClicked: {
                if (Qt.platform.os === "android") {
                    pickingImage = postUtils.pickPhoto()
                } else {
                    fileDialog.open()
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
            focusPolicy: Qt.NoFocus

            onActivated: (index) => {
                currentPostItem().language = valueAt(index)
                console.debug("ACTIVATED LANG:", valueAt(index))

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
            id: contentWarningIcon
            anchors.left: languageSelector.right
            anchors.leftMargin: 8
            y: height + 5 + restrictionRow.height + footerSeparator.height
            accessibleName: qsTr("add content warning")
            svg: hasContentWarning() ? svgOutline.hideVisibility : svgOutline.visibility
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
            svg: svgOutline.add
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
        onFileSelected: (fileName) => { photoPicked(fileName) }
    }

    LinkCardReader {
        id: linkCardReader
        onLinkCard: (card) => {
                        console.debug("Got card:", card.link, card.title, card.thumb)
                        console.debug(card.description)

                        let postItem = threadPosts.itemAt(linkCardTimer.postIndex)

                        if (!postItem)
                            return

                        postItem.getLinkCard().show(card)
                        let postText = postItem.getPostText()

                        if (!postText.cursorInFirstWebLink)
                            postItem.getLinkCard().linkFixed = true
                        else
                            postText.cutLinkIfJustAdded(postText.firstWebLink, () => postItem.getLinkCard().linkFixed = true)
                    }
    }

    Timer {
        property int postIndex
        property string webLink

        id: linkCardTimer
        interval: 1000
        onTriggered: linkCardReader.getLinkCard(webLink)

        function startForLink(postIndex, webLink) {
            linkCardTimer.postIndex = postIndex
            linkCardTimer.webLink = webLink
            start()
        }
    }

    PostUtils {
        id: postUtils
        skywalker: page.skywalker

        onPostOk: (uri, cid) => {
            postedUris.push(uri)

            if (page.sendingThreadPost > -1)
                sendNextThreadPost(uri, cid)
            else if (page.restrictReply)
                postUtils.addThreadgate(uri, cid, page.allowReplyMentioned, page.allowReplyFollowing, page.getReplyRestrictionListUris())
            else
                postDone()
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

        onPostProgress: (msg) => page.postProgress(msg)

        onPhotoPicked: (imgSource) => {
            pickingImage = false
            page.photoPicked(imgSource)
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

        onQuotePost: (uri, cid, text, author, timestamp) => {
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

        onQuoteFeed: (feed) => {
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

        onQuoteList: (list) => {
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

    UnicodeFonts {
        id: unicodeFonts
    }

    LanguageUtils {
        id: languageUtils
        skywalker: page.skywalker
    }

    Tenor {
        id: tenor
        skywalker: page.skywalker
    }

    Timer {
        property string text
        property int index
        property int cursorPosition

        id: setPostTextTimer
        interval: 0
        onTriggered: {
            let postText = threadPosts.itemAt(index).getPostText()

            if (cursorPosition > -1)
                setCursorTimer.startSetCursor(index, cursorPosition)

            postText.text = text
        }

        function startSetText(text, index, cursorPosition = -1) {
            setPostTextTimer.text = text
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

            postText.ensureVisible(Qt.rect(0, 0, postText.width, postText.height))
            postText.forceActiveFocus()
        }
    }

    GuiSettings {
        id: guiSettings
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
            statusPopup.show(msg, QEnums.STATUS_LEVEL_INFO)
        else
            statusPopup.show(qsTr(`Post ${(sendingThreadPost + 1)}: ${msg}`), QEnums.STATUS_LEVEL_INFO)
    }

    function checkAltText() {
        if (!requireAltText)
            return true

        for (let postIndex = 0; postIndex < threadPosts.count; ++postIndex) {
            const postItem = threadPosts.itemAt(postIndex)

            for (let i = 0; i < postItem.images.length; ++i) {
                if (i >= postItem.altTexts.length || postItem.altTexts[i].length === 0)
                    return false
            }
        }

        return true
    }

    // "file://" or "image://" source
    function photoPicked(source, altText = "") {
        console.debug("IMAGE:", source)
        let postItem = currentPostItem()

        if (!postItem)
            return

        postItem.altTexts.push(altText)
        postItem.images.push(source)
        let scrollBar = postItem.getImageScroller().ScrollBar.horizontal
        scrollBar.position = 1.0 - scrollBar.size
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

        if (postItem.images.length >= page.maxImages) {
            statusPopup.show(qsTr(`Post already has ${page.maxImages} images attached.`), QEnums.STATUS_LEVEL_INFO, 30)
            postUtils.dropPhoto(source)
            return
        }

        photoPicked(source)
        addSharedText(text)
    }

    function postDone() {
        busyIndicator.running = false
        page.closed()
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

    function joinPosts(text1, text2) {
        const joinStr = (/\s/.test(text1.slice(-1)) || /\s/.test(text2.charAt(0))) ? "" : " "
        const newText = text1 + joinStr + text2
        return newText
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
        return `🧵${(postIndex + 1)}/${postCount}`
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
                           qUri, qCid, labels, postItem.language)
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
                           qUri, qCid, labels, postItem.language)
        } else {
            postUtils.post(postText, postItem.images, postItem.altTexts,
                           parentUri, parentCid,
                           rootUri, rootCid,
                           qUri, qCid, labels, postItem.language);
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
                postUtils.addThreadgate(prevUri, prevCid, allowReplyMentioned, allowReplyFollowing,
                                        getReplyRestrictionListUris())
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

        const draft = draftPosts.createDraft(postItem.text, postItem.images, postItem.altTexts,
                                 replyToPostUri, replyToPostCid,
                                 replyRootPostUri, replyRootPostCid,
                                 replyToAuthor, unicodeFonts.toPlainText(replyToPostText),
                                 replyToPostDateTime,
                                 qUri, qCid, postItem.quoteAuthor, unicodeFonts.toPlainText(postItem.quoteText),
                                 postItem.quoteDateTime, postItem.quoteFixed,
                                 postItem.quoteFeed, postItem.quoteList,
                                 postItem.gif, labels, postItem.language,
                                 restrictReply, allowReplyMentioned, allowReplyFollowing,
                                 getReplyRestrictionListUris())

        let draftItemList = []

        for (let i = 1; i < threadPosts.postList.length; ++i) {
            const threadItem = threadPosts.postList[i]
            const qUriItem = threadItem.getQuoteUri()
            const qCidItem = threadItem.getQuoteCid()
            const labelsItem = threadItem.getContentLabels()

            const draftItem = draftPosts.createDraft(threadItem.text, threadItem.images, threadItem.altTexts,
                                     "", "",
                                     "", "",
                                     nullAuthor, "",
                                     new Date(),
                                     qUriItem, qCidItem, threadItem.quoteAuthor, unicodeFonts.toPlainText(threadItem.quoteText),
                                     threadItem.quoteDateTime, threadItem.quoteFixed,
                                     threadItem.quoteFeed, threadItem.quoteList,
                                     threadItem.gif, labelsItem, threadItem.language,
                                     false, false, false,
                                     [])

            draftItemList.push(draftItem)
            postUtils.cacheTags(threadItem.text)
            languageUtils.addUsedPostLanguage(threadItem.language)
        }

        draftPosts.saveDraftPost(draft, draftItemList)
        postUtils.cacheTags(postItem.text)
        languageUtils.addUsedPostLanguage(postItem.language)
    }

    function showDraftPosts() {
        let component = Qt.createComponent("DraftPostsView.qml")
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
        let component = Qt.createComponent("AnniversaryCardMaker.qml")
        let cardPage = component.createObject(page)
        cardPage.onCanceled.connect(() => root.popStack())
        cardPage.onAddCard.connect((source, years) => {
            page.photoPicked(source, qsTr(`Bluesky anniversary card sent with ${guiSettings.skywalkerHandle}`))
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

            for (let i = 0; i < draftData.images.length; ++i) {
                postItem.images.push(draftData.images[i].fullSizeUrl)
                postItem.altTexts.push(draftData.images[i].alt)
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
                allowReplyFollowing = draftData.allowFollowing
                allowListUrisFromDraft = draftData.allowLists
                allowListIndexes = [0, 1, 2]
                allowLists = [false, false, false]
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

    function addReplyRestrictions() {
        if (restrictionsListModelId < 0) {
            restrictionsListModelId = skywalker.createListListModel(QEnums.LIST_TYPE_ALL, QEnums.LIST_PURPOSE_CURATE, userDid)
            skywalker.getListList(restrictionsListModelId)
        }

        let component = Qt.createComponent("AddReplyRestrictions.qml")
        let restrictionsPage = component.createObject(page, {
                restrictReply: page.restrictReply,
                allowMentioned: page.allowReplyMentioned,
                allowFollowing: page.allowReplyFollowing,
                allowLists: page.allowLists,
                allowListIndexes: page.allowListIndexes,
                allowListUrisFromDraft: page.allowListUrisFromDraft,
                listModelId: page.restrictionsListModelId
        })
        restrictionsPage.onAccepted.connect(() => {
                page.restrictReply = restrictionsPage.restrictReply
                page.allowReplyMentioned = restrictionsPage.allowMentioned
                page.allowReplyFollowing = restrictionsPage.allowFollowing
                page.allowLists = restrictionsPage.allowLists
                page.allowListIndexes = restrictionsPage.allowListIndexes
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

    function addContentWarning() {
        let postItem = currentPostItem()

        if (!postItem)
            return

        let component = Qt.createComponent("AddContentWarning.qml")
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

    function hasImageContent() {
        let postItem = currentPostItem()

        if (!postItem)
            return false

        return !postItem.getGifAttachment().gif.isNull() ||
                (postItem.getLinkCard().card && postItem.getLinkCard().card.thumb) ||
                postItem.images.length > 0
    }

    function canAddImage() {
        const postItem = currentPostItem()

        if (!postItem)
            return false

        return postItem.images.length < maxImages &&
                threadPosts.postList[currentPostIndex].gif.isNull() &&
                !threadPosts.postList[currentPostIndex].card &&
                !pickingImage
    }

    function canAddGif() {
        const postItem = currentPostItem()

        if (!postItem)
            return false

        return threadPosts.postList[currentPostIndex].gif.isNull() &&
                !threadPosts.postList[currentPostIndex].card &&
                postItem.images.length === 0
    }

    function currentPostLanguage() {
        const postItem = currentPostItem()

        if (!postItem)
            return languageUtils.defaultPostLanguage

        return postItem.language ? postItem.language : languageUtils.defaultPostLanguage
    }

    function hasContentWarning() {
        const postItem = currentPostItem()

        if (!postItem)
            return false

        return hasImageContent() && (postItem.cwSuggestive || postItem.cwNudity || postItem.cwPorn || postItem.cwGore)
    }

    VirtualKeyboardPageResizer {
        id: virtualKeyboardPageResizer
    }

    Component.onDestruction: {
        for (let i = 0; i < threadPosts.count; ++i) {
            let postItem = threadPosts.itemAt(i)
            postItem.images.forEach((value, index, array) => { postUtils.dropPhoto(value); })
        }

        if (restrictionsListModelId >= 0)
            skywalker.removeListListModel(restrictionsListModelId)

        draftPosts.removeDraftPostsModel()
    }

    Component.onCompleted: {
        // Save the full page height now. Later when the Android keyboard pops up,
        // the page height sometimes changes by itself, but not always...
        virtualKeyboardPageResizer.fullPageHeight = parent.height

        // Wait a bit for the window to render.
        // Then make sue the text field is in the visible area.
        focusTimer.start()

        threadPosts.copyPostListToPostItems()
        draftPosts.loadDraftPosts()
    }
}
