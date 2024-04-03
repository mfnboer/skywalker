import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window 2.2
import skywalker

Page {
    required property var skywalker
    property string initialText
    property string initialImage
    property int fullPageHeight

    property int maxThreadPosts: 99
    property int maxImages: 4
    property bool pickingImage: false

    // Reply restrictions
    property bool restrictReply: false
    property bool allowReplyMentioned: false
    property bool allowReplyFollowing: false
    property list<int> allowListIndexes: [0, 1, 2]
    property list<bool> allowLists: [false, false, false]
    property list<string> allowListUrisFromDraft: []
    property int restrictionsListModelId: -1

    // Content warnings // TODO
    property bool cwSuggestive: false
    property bool cwNudity: false
    property bool cwPorn: false
    property bool cwGore: false

    // Reply-to
    property basicprofile replyToAuthor
    property string replyToPostUri: ""
    property string replyToPostCid: ""
    property string replyRootPostUri: ""
    property string replyRootPostCid: ""
    property string replyToPostText
    property date replyToPostDateTime

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

    readonly property string userDid: skywalker.getUserDid()
    readonly property bool requireAltText: skywalker.getUserSettings().getRequireAltText(userDid)

    property tenorgif attachedGif
    property int currentPostIndex: 0

    signal closed

    id: page
    width: parent.width
    height: parent.height
    contentHeight: flick.height
    topPadding: 10
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
            onClicked: page.cancel()

            Accessible.role: Accessible.Button
            Accessible.name: qsTr("cancel posting")
            Accessible.onPressAction: clicked()
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
            id: postButton
            anchors.rightMargin: 10
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            text: replyToPostUri ? qsTr("Reply", "verb on post composition") : qsTr("Post", "verb on post composition")

            enabled: postsAreValid() && hasContent() && checkAltText()
            onClicked: sendPost()

            Accessible.role: Accessible.Button
            Accessible.name: replyToPostUri ? qsTr("send reply") : qsTr("send post")
            Accessible.onPressAction: if (enabled) clicked()

            function sendPost() {
                postButton.enabled = false

                const postItem = currentPostItem()
                const qUri = postItem.getQuoteUri()
                const qCid = postItem.getQuoteCid()
                const labels = getContentLabels()

                if (postItem.getLinkCard().card) {
                    postUtils.post(postItem.getPostText().text,
                                   postItem.getLinkCard().card,
                                   replyToPostUri, replyToPostCid,
                                   replyRootPostUri, replyRootPostCid,
                                   qUri, qCid, labels)
                } else if (postItem.getGifAttachment().gif) {
                    tenor.registerShare(attachedGif)

                    let gifCard = linkCardReader.makeLinkCard(
                            attachedGif.url,
                            `${attachedGif.description} (via Tenor)\nPosted from Skywalker ${guiSettings.skywalkerHandle}`,
                            qsTr("This GIF has been posted from Skywalker for Android. " +
                                 "Get Skywalker from Google Play.") +
                                 (`<br>Bluesky: ${guiSettings.skywalkerHandle}`),
                            attachedGif.imageUrl)

                    postUtils.post(postItem.getPostText().text, gifCard,
                                   replyToPostUri, replyToPostCid,
                                   replyRootPostUri, replyRootPostCid,
                                   qUri, qCid, labels)
                } else {
                    postUtils.post(postItem.getPostText().text, postItem.images, postItem.altTexts,
                                   replyToPostUri, replyToPostCid,
                                   replyRootPostUri, replyRootPostCid,
                                   qUri, qCid, labels);
                }

                postUtils.cacheTags(postItem.getPostText().text)
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
            width: parent.width - 20
            anchors.horizontalCenter: parent.horizontalCenter
            author: replyToAuthor
            postText: replyToPostText
            postDateTime: replyToPostDateTime
            ellipsisBackgroundColor: guiSettings.postHighLightColor
            visible: replyToPostUri
        }

        Column {
            id: threadColumn
            y: replyToColumn.visible ? replyToColumn.height + 5 : 0
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
                    }
                ]

                id: threadPosts
                width: parent.width
                model: 1

                Item {
                    required property int index
                    property list<string> images: threadPosts.postList[index].images
                    property list<string> altTexts: threadPosts.postList[index].altTexts
                    property basicprofile quoteAuthor: threadPosts.postList[index].quoteAuthor
                    property string quoteUri: threadPosts.postList[index].quoteUri
                    property string quoteCid: threadPosts.postList[index].quoteCid
                    property string quoteText: threadPosts.postList[index].quoteText
                    property date quoteDateTime: threadPosts.postList[index].quoteDateTime
                    property generatorview quoteFeed: threadPosts.postList[index].quoteFeed
                    property listview quoteList: threadPosts.postList[index].quoteList

                    onImagesChanged: threadPosts.postList[index].images = images
                    onAltTextsChanged: threadPosts.postList[index].altTexts = altTexts
                    //onQuoteAuthorChanged: threadPosts.postList[index].quoteAuthor = quoteAuthor
                    onQuoteUriChanged: threadPosts.postList[index].quoteUri = quoteUri
                    onQuoteCidChanged: threadPosts.postList[index].quoteCid = quoteCid
                    onQuoteTextChanged: threadPosts.postList[index].quoteText = quoteText
                    onQuoteDateTimeChanged: threadPosts.postList[index].quoteDateTime = quoteDateTime
                    //onQuoteFeedChanged: threadPosts.postList[index].quoteFeed = quoteFeed
                    //onQuoteListChanged: threadPosts.postList[index].quoteList = quoteList

                    function getPostText() { return postText }
                    function getImageScroller() { return imageScroller }
                    function getGifAttachment() { return gifAttachment }
                    function getLinkCard() { return linkCard }

                    function getQuoteUri() {
                        if (quoteUri)
                            return quoteUri

                        if (quoteFeed.uri)
                            return quoteFeed.uri

                        return quoteList.uri
                    }

                    function getQuoteCid() {
                        if (quoteCid)
                            return quoteCid

                        if (quoteFeed.cid)
                            return quoteFeed.cid

                        return quoteList.cid
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
                        return postText.graphemeLength > 0 ||
                                images.length > 0 ||
                                linkCard.card ||
                                gifAttachment.gif
                    }

                    function isValid() {
                        return postText.graphemeLength <= postText.maxLength
                    }

                    id: postItem
                    width: parent.width
                    height: calcHeight()

                    Rectangle {
                        id: postSeparatorFrontSpace
                        width: parent.width
                        height: index > 0 ? 10 : 0
                        color: "transparent"
                        visible: index > 0
                    }

                    Rectangle {
                        id: postSeparatorLine
                        anchors.top: postSeparatorFrontSpace.bottom
                        width: parent.width
                        height: index > 0 ? 1 : 0
                        color: guiSettings.separatorColor
                        visible: index > 0
                    }

                    Rectangle {
                        id: postSeparatorSpace
                        anchors.top: postSeparatorLine.bottom
                        width: parent.width
                        height: index > 0 ? 10 : 0
                        color: "transparent"
                        visible: index > 0
                    }

                    SkyFormattedTextEdit {
                        id: postText
                        anchors.top: postSeparatorSpace.bottom
                        width: parent.width
                        parentPage: page
                        parentFlick: flick
                        placeholderText: qsTr("Say something nice")
                        initialText: threadPosts.postList[index].text
                        maxLength: 300
                        fontSelectorCombo: fontSelector

                        onTextChanged: threadPosts.postList[index].text = text

                        onFocusChanged: {
                            if (focus)
                                currentPostIndex = index
                        }

                        onFirstWebLinkChanged: {
                            if (gifAttachment.visible)
                                return

                            linkCard.hide()

                            if (firstWebLink) {
                                linkCardTimer.startForLink(firstWebLink)
                            } else {
                                linkCardTimer.stop()
                            }
                        }

                        onFirstPostLinkChanged: {
                            if (page.openedAsQuotePost)
                                return

                            quoteList = page.nullList
                            quoteFeed = page.nullFeed
                            quoteUri = ""

                            if (firstPostLink)
                                postUtils.getQuotePost(firstPostLink)
                        }

                        onFirstFeedLinkChanged: {
                            if (page.openedAsQuotePost)
                                return

                            quoteList = page.nullList
                            quoteFeed = page.nullFeed

                            if (firstPostLink)
                                return

                            if (firstFeedLink)
                                postUtils.getQuoteFeed(firstFeedLink)
                        }

                        onFirstListLinkChanged: {
                            if (page.openedAsQuotePost)
                                return

                            page.quoteList = page.nullList

                            if (firstPostLink || firstFeedLink)
                                return

                            if (firstListLink)
                                postUtils.getQuoteList(firstListLink)
                        }
                    }

                    SvgButton {
                        y: postText.y - 5
                        x: parent.width - width - 10
                        z: 10
                        width: 34
                        height: width
                        svg: svgOutline.remove
                        visible: !postItem.hasContent() && threadPosts.count > 1 && (index > 0 || !replyToPostUri)

                        onClicked: threadPosts.removePost(index)
                    }

                    // Image attachments
                    ImageScroller {
                        property alias images: postItem.images
                        property alias altTexts: postItem.altTexts

                        id: imageScroller
                        width: page.width
                        anchors.top: postText.bottom
                        requireAltText: page.requireAltText
                        postUtils: page.getPostUtils()
                        visible: !linkCard.visible && !gifAttachment.visible
                    }

                    // GIF attachment
                    AnimatedImage {
                        property var gif: null
                        property tenorgif nullGif

                        id: gifAttachment
                        x: 10
                        width: Math.min(gif ? gif.smallSize.width : 1, page.width - 20)
                        anchors.top: imageScroller.bottom
                        anchors.topMargin: gif ? 10 : 0
                        fillMode: Image.PreserveAspectFit
                        source: gif ? gif.smallUrl : ""
                        visible: gif

                        Accessible.role: Accessible.StaticText
                        Accessible.name: qsTr("GIF image")

                        function show(gif) {
                            // For some reasong the gif property of the AnimatedImage gets destroyed
                            // after setting it from a draft after the AnimatedImage gets displayed.
                            // The copy in attached gif can be used for sending/saving.
                            attachedGif = gif
                            gifAttachment.gif = gif
                            linkCard.hide()
                        }

                        function hide() {
                            gifAttachment.gif = null
                        }

                        SvgButton {
                            x: parent.width - width
                            height: width
                            svg: svgOutline.close
                            onClicked: gifAttachment.hide()

                            Accessible.role: Accessible.Button
                            Accessible.name: qsTr("remove GIF image")
                            Accessible.onPressAction: clicked()
                        }
                    }

                    // Link card attachment
                    LinkCardView {
                        property var card: null

                        id: linkCard
                        x: 10
                        width: page.width - 20
                        height: card ? columnHeight : 0
                        anchors.top: gifAttachment.bottom
                        uri: card ? card.link : ""
                        title: card ? card.title : ""
                        description: card ? card.description : ""
                        thumbUrl: card ? card.thumb : ""
                        visible: card

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
                        }

                        SvgButton {
                            x: parent.width - width
                            height: width
                            svg: svgOutline.close
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
                        width: parent.width - 20
                        anchors.top: linkCard.bottom
                        anchors.topMargin: 5
                        anchors.horizontalCenter: parent.horizontalCenter
                        author: postItem.quoteAuthor
                        postText: postItem.quoteText
                        postDateTime: postItem.quoteDateTime
                        ellipsisBackgroundColor: guiSettings.postHighLightColor
                        visible: postItem.quoteUri
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
                        width: parent.width - 20
                        anchors.top: linkCard.bottom
                        anchors.topMargin: 5
                        anchors.horizontalCenter: parent.horizontalCenter
                        feed: postItem.quoteFeed
                        visible: !postItem.quoteFeed.isNull()
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
                        width: parent.width - 20
                        anchors.top: linkCard.bottom
                        anchors.topMargin: 5
                        anchors.horizontalCenter: parent.horizontalCenter
                        list: postItem.quoteList
                        visible: !postItem.quoteList.isNull()
                    }
                }

                function newComposePostItem() {
                    let component = Qt.createComponent("ComposePostItem.qml")
                    return component.createObject(page)
                }

                function removePost(index) {
                    console.debug("REMOVE POST:", index)
                    if (count === 1) {
                        console.warn("Cannot remove last post")
                        return
                    }

                    if (currentPostIndex === count - 1)
                        currentPostIndex -= 1

                    model = 0
                    postList.splice(index, 1)
                    model = postList.length

                    moveFocusToCurrent()
                }

                function addPost(index) {
                    console.debug("ADD POST:", index)
                    if (count >= maxThreadPosts) {
                        console.warn("Maximum posts reached:", count)
                        return
                    }

                    postList.splice(index + 1, 0, newComposePostItem())
                    model = postList.length
                    console.debug("SPLICED")

                    if (currentPostIndex === index)
                        currentPostIndex += 1

                    console.debug("CURRENT SET")
                    moveFocusToCurrent()
                }

                function moveFocusToCurrent() {
                    let postText = currentPostItem().getPostText()
                    postText.cursorPosition = postText.text.length
                    postText.forceActiveFocus()
                }
            }
        }
    }

    Text {
        id: draftsLink
        anchors.centerIn: parent
        font.pointSize: guiSettings.scaledFont(9/8)
        text: qsTr("<a href=\"drafts\">Drafts</a>")
        visible: !hasContent() && !replyToPostUri && !quoteUri && draftPosts.hasDrafts // TODO
        onLinkActivated: showDraftPosts()

        Accessible.role: Accessible.Link
        Accessible.name: unicodeFonts.toPlainText(text)
        Accessible.onPressAction: showDraftPosts()
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
            enabled: mustEnable()

            onClicked: {
                if (Qt.platform.os === "android") {
                    pickingImage = postUtils.pickPhoto()
                } else {
                    fileDialog.open()
                }
            }

            function mustEnable() {
                const postItem = threadPosts.itemAt(currentPostIndex)

                if (!postItem)
                    return false

                return postItem.images.length < maxImages && !postItem.getGifAttachment().visible && !postItem.getLinkCard().visible && !pickingImage
            }
        }

        AddGifButton {
            id: addGif
            x: addImage.x + addImage.width + 10
            y: height + 5 + restrictionRow.height + footerSeparator.height
            enabled: mustEnable()

            onSelectedGif: (gif) => currentPostItem().getGifAttachment().show(gif)

            function mustEnable() {
                const postItem = threadPosts.itemAt(currentPostIndex)

                if (!postItem)
                    return false

                return !postItem.getGifAttachment().visible && !postItem.getLinkCard().visible && postItem.images.length === 0
            }
        }

        FontComboBox {
            id: fontSelector
            x: addGif.x + addGif.width + 15
            y: 5 + restrictionRow.height + footerSeparator.height

            popup.onClosed: currentPostItem().getPostText().forceActiveFocus()
        }

        SvgTransparentButton {
            id: contentWarningIcon
            anchors.left: fontSelector.right
            anchors.leftMargin: 15
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
            enabled: hasContent() && threadPosts.count < maxThreadPosts

            onClicked: threadPosts.addPost(currentPostIndex)

            Accessible.role: Accessible.Button
            Accessible.name: qsTr("add post")
            Accessible.onPressAction: clicked()
        }

        // TODO: remove?
        // TextLengthCounter {
        //     y: 10 + restrictionRow.height + footerSeparator.height
        //     anchors.rightMargin: 10
        //     anchors.right: parent.right
        //     textField: currentPostItem().getPostText()
        // }
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
                        currentPostItem().getLinkCard().show(card)
                    }
    }

    Timer {
        property string webLink

        id: linkCardTimer
        interval: 1000
        onTriggered: linkCardReader.getLinkCard(webLink)

        function startForLink(webLink) {
            linkCardTimer.webLink = webLink
            start()
        }
    }

    PostUtils {
        id: postUtils
        skywalker: page.skywalker

        onPostOk: (uri, cid) => {
            if (page.restrictReply)
                postUtils.addThreadgate(uri, page.allowReplyMentioned, page.allowReplyFollowing, page.getReplyRestrictionListUris())
            else
                postDone()
        }

        onPostFailed: (error) => page.postFailed(error)

        onThreadgateOk: postDone()
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

                if (!postItem.getPostText().firstPostLink)
                    return

                postItem.quoteList = page.nullList
                postItem.quoteFeed = page.nullFeed
                postItem.quoteUri = uri
                postItem.quoteCid = cid
                postItem.quoteText = text
                postItem.quoteAuthor = author
                postItem.quoteDateTime = timestamp
            }

        onQuoteFeed: (feed) => {
                let postItem = currentPostItem()

                if (!postItem.getPostText().firstFeedLink)
                    return

                if (postItem.getPostText().firstPostLink)
                    return

                postItem.quoteList = page.nullList
                postItem.quoteFeed = feed
            }

        onQuoteList: (list) => {
                let postItem = currentPostItem()

                if (!postItem.getPostText().firstListLink)
                    return

                if (postItem.getPostText().firstPostLink || postItem.getPostText().firstFeedLink)
                    return

                postItem.quoteList = list
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

    Tenor {
        id: tenor
    }

    Timer {
        id: focusTimer
        interval: 200
        onTriggered: {
            let postText = currentPostItem().getPostText()

            if (!initialText.startsWith("\n#")) // hashtag post
                postText.cursorPosition = initialText.length

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
        postButton.enabled = true
    }

    function postProgress(msg) {
        busyIndicator.running = true
        statusPopup.show(msg, QEnums.STATUS_LEVEL_INFO)
    }

    function checkAltText() {
        if (!requireAltText)
            return true

        for (let postIndex = 0; postIndex < threadPosts.count; ++postIndex) {
            const postItem = threadPosts.itemAt(postIndex)
            const imgScroller = postItem.getImagesScroller()

            for (let i = 0; i < postItem.images.length; ++i) {
                if (!imgScroller.hasAltText(i))
                    return false
            }
        }

        return true
    }

    // "file://" or "image://" source
    function photoPicked(source) {
        console.debug("IMAGE:", source)
        let postItem = currentPostItem()

        if (!postItem)
            return

        postItem.altTexts.push("")
        postItem.images.push(source)
        let scrollBar = postItem.getImageScroller().ScrollBar.horizontal
        scrollBar.position = 1.0 - scrollBar.size
    }

    function addSharedText(text) {
        if (text) {
            let postItem = currentPostItem()

            if (!postItem)
                return

            postItem.postText.append(text)
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

    function hasContent() {
        for (let i = 0; i < threadPosts.count; ++ i) {
            const postItem = threadPosts.itemAt(i)

            if (!postItem.hasContent())
                return false
        }

        return threadPosts.count > 0
    }

    function cancel() {
        if (!hasContent()) {
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

    function saveDraftPost() {
        const postItem = currentPostItem()

        if (!postItem)
            return

        const postText = postItem.getPostText()
        const qUri = postItem.getQuoteUri()
        const qCid = postItem.getQuoteCid()
        const labels = getContentLabels()
        const gif = gifAttachment.gif ? attachedGif : gifAttachment.nullGif

        draftPosts.saveDraftPost(postText.text, postItem.images, postItem.altTexts,
                                 replyToPostUri, replyToPostCid,
                                 replyRootPostUri, replyRootPostCid,
                                 replyToAuthor, unicodeFonts.toPlainText(replyToPostText),
                                 replyToPostDateTime,
                                 qUri, qCid, quoteAuthor, unicodeFonts.toPlainText(quoteText),
                                 quoteDateTime, postItem.quoteFeed, postItem.quoteList,
                                 gif, labels,
                                 restrictReply, allowReplyMentioned, allowReplyFollowing,
                                 getReplyRestrictionListUris())

        postUtils.cacheTags(postText.text)
    }

    function showDraftPosts() {
        let component = Qt.createComponent("DraftPostsView.qml")
        let draftsPage = component.createObject(page, { model: draftPosts.getDraftPostsModel() })
        draftsPage.onClosed.connect(() => root.popStack())
        draftsPage.onSelected.connect((index) => {
            const draftData = draftPosts.getDraftPostData(index)
            setDraftPost(draftData)
            draftPosts.removeDraftPost(index)
            draftData.destroy()
            root.popStack()
        })
        draftsPage.onDeleted.connect((index) => draftPosts.removeDraftPost(index))

        root.pushStack(draftsPage)
    }

    function setDraftPost(draftData) {
        let postItem = currentPostItem()

        if (!postItem)
            return

        postItem.getLinkCard.hide()
        postItem.getGifAttachment.hide()
        postItem.images = []
        postItem.altTexts = []

        postItem.getPostText().text = draftData.text

        for (let i = 0; i < draftData.images.length; ++i) {
            postItem.images.push(draftData.images[i].fullSizeUrl)
            postItem.altTexts.push(draftData.images[i].alt)
        }

        replyToAuthor = draftData.replyToAuthor
        replyToPostUri = draftData.replyToUri
        replyToPostCid = draftData.replyToCid
        replyRootPostUri = draftData.replyRootUri
        replyRootPostCid = draftData.replyRootCid
        replyToPostText = draftData.replyToText
        replyToPostDateTime = draftData.replyToDateTime

        openedAsQuotePost = draftData.openAsQuotePost
        quoteUri = draftData.quoteUri
        quoteCid = draftData.quoteCid
        quoteAuthor = draftData.quoteAuthor
        quoteText = draftData.quoteText
        quoteDateTime = draftData.quoteDateTime

        // TODO
        quoteFeed = draftData.quoteFeed
        quoteList = draftData.quoteList

        if (draftData.gif.isNull())
            postItem.getGifAttachment().hide()
        else
            postItem.getGifAttachment().show(draftData.gif)

        setContentWarnings(draftData.labels)
        restrictReply = draftData.restrictReplies
        allowReplyMentioned = draftData.allowMention
        allowReplyFollowing = draftData.allowFollowing
        allowListUrisFromDraft = draftData.allowLists
        allowListIndexes = [0, 1, 2]
        allowLists = [false, false, false]
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

        let uris = []

        for (let i = 0; i < allowLists.length; ++i) {
            if (allowLists[i]) {
                let model = skywalker.getListListModel(restrictionsListModelId)
                const listView = model.getEntry(allowListIndexes[i])
                uris.push(listView.uri)
            }
        }

        console.debug("Restriction lists:", uris)
        return uris
    }

    function addContentWarning() {
        let component = Qt.createComponent("AddContentWarning.qml")
        let cwPage = component.createObject(page, {
                suggestive: cwSuggestive,
                nudity: cwNudity,
                porn: cwPorn,
                gore: cwGore
        })
        cwPage.onAccepted.connect(() => {
                cwSuggestive = cwPage.suggestive
                cwNudity = cwPage.nudity
                cwPorn = cwPage.porn
                cwGore = cwPage.gore
                cwPage.destroy()
        })
        cwPage.onRejected.connect(() => cwPage.destroy())
        cwPage.open()
    }

    function hasImageContent() {
        let postItem = currentPostItem()

        if (!postItem)
            return false

        return postItem.getGifAttachment().gif ||
                (postItem.getLinkCard().card && postItem.getLinkCard().card.thumb) ||
                postItem.images.length > 0
    }

    function hasContentWarning() {
        return hasImageContent() && (cwSuggestive || cwNudity || cwPorn || cwGore)
    }

    function getContentLabels() {
        let labels = []

        if (!hasImageContent())
            return labels

        if (cwSuggestive)
            labels.push("sexual")
        if (cwNudity)
            labels.push("nudity")
        if (cwPorn)
            labels.push("porn")
        if (cwGore)
            labels.push("gore")

        return labels
    }

    function setContentWarnings(labels) {
        cwSuggestive = false
        cwNudity = false
        cwPorn = false
        cwGore = false

        labels.forEach((label) => {
            if (label === "sexual")
                cwSuggestive = true
            else if (label === "nudity")
                cwNudity = true
            else if (label === "porn")
                cwPorn = true
            else if (label === "gore")
                cwGore = true
        })
    }

    Connections {
        target: Qt.inputMethod

        // Resize the page when the Android virtual keyboard is shown
        function onKeyboardRectangleChanged() {
            if (Qt.inputMethod.keyboardRectangle.y > 0) {
                // Sometimes the page height gets changed automatically but most times not...
                // Setting to to keyboard-y seems reliable.
                const keyboardY = Qt.inputMethod.keyboardRectangle.y  / Screen.devicePixelRatio
                parent.height = keyboardY
            }
            else {
                console.debug("HIDE KEYBOARD, PARENT:", parent.height, "CONTENT:", contentHeight)
                parent.height = fullPageHeight
                fontSelector.virtualKeyboardClosed()
            }
        }
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
        fullPageHeight = parent.height

        // Wait a bit for the window to render.
        // Then make sue the text field is in the visible area.
        focusTimer.start()

        // TODO: remove?
        // postItem = threadPosts.itemAt(0)
        // postUtils.setHighlightDocument(postItem.getPostText().textDocument,
        //                                guiSettings.linkColor,
        //                                postItem.getPostText().maxLength,
        //                                guiSettings.textLengthExceededColor)

        draftPosts.loadDraftPosts()
    }
}
