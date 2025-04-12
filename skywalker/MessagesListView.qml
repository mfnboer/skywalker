import QtQuick
import QtQuick.Controls
import skywalker

SkyPage {
    required property var chat
    required property convoview convo
    readonly property bool convoAccepted: convo.status === QEnums.CONVO_STATUS_ACCEPTED
    property basicprofile firstMember: convo.members.length > 0 ? convo.members[0].basicProfile : skywalker.getUserProfile()
    property bool isSending: false
    property int maxInputTextHeight
    readonly property int maxMessageLength: 1000
    readonly property int margin: 10
    property var skywalker: root.getSkywalker()
    property int textInputToolbarHeight: (keyboardHandler.keyboardVisible || !guiSettings.isAndroid) ? 24 : 0
    property int quotedContentHeight: quoteColumn.visible ? quoteColumn.height : 0
    property int lastIndex: -1

    signal closed
    signal acceptConvo(convoview convo)
    signal deleteConvo(convoview convo)
    signal blockAndDeleteConvo(convoview convo, basicprofile author)

    id: page

    header: MessagesListHeader {
        convo: page.convo
        onBack: page.closed()
    }

    footer: Rectangle {
        height: keyboardHandler.keyboardHeight
        color: "transparent"
    }

    SkyListView {
        id: messagesView
        width: parent.width
        height: parent.height - y - (convoAccepted ? flick.height : requestButtons.height) - newMessageText.padding - newMessageText.bottomPadding
        model: chat.getMessageListModel(convo.id)
        boundsMovement: Flickable.StopAtBounds
        clip: true

        onHeightChanged: moveToEnd()

        delegate: MessageViewDelegate {
            required property int index

            viewWidth: messagesView.width
            convo: page.convo
            onDeleteMessage: (messageId) => page.deleteMessage(messageId)
            onReportMessage: (msg) => page.reportDirectMessage(msg)
            onOpeningEmbed: page.lastIndex = index
            onAddEmoji: (messageId, emoji) => page.addReaction(messageId, emoji)
            onPickEmoji: (messageId) => utils.pickEmoji(messageId)
            onShowReactions: (msg) => page.showReactionList(msg)
        }

        FlickableRefresher {
            inProgress: chat.getMessagesInProgress
            topOvershootFun: () => chat.getMessagesNextPage(convo.id)
        }

        EmptyListIndication {
            y: parent.headerItem ? parent.headerItem.height : 0
            svg: SvgOutline.noDirectMessages
            text: qsTr("None")
            list: messagesView
        }

        BusyIndicator {
            anchors.centerIn: parent
            running: chat.getMessagesInProgress
        }

        function moveToEnd() {
            positionViewAtEnd()

            // HACK
            // positionViewAtEnd not always gets completely to the end...
            // This seems to fix it...
            contentY = Math.max(originY, originY + contentHeight - height)
        }
    }

    ConvoRequestButtonRow {
        id: requestButtons
        x: page.margin
        width: parent.width - 2 * page.margin
        anchors.bottom: parent.bottom
        anchors.bottomMargin: page.margin
        author: firstMember
        visible: !convoAccepted

        onAcceptConvo: page.acceptConvo(convo)
        onDeleteConvo: page.deleteConvo(convo)
        onBlockAndDeleteConvo: page.blockAndDeleteConvo(convo, firstMember)
    }

    Flickable {
        property bool contentYUpdating: false

        id: flick
        width: parent.width
        height: Math.min(newMessageText.height - newMessageText.padding - newMessageText.bottomPadding, maxInputTextHeight)
        anchors.bottom: parent.bottom
        anchors.bottomMargin: newMessageText.bottomPadding
        clip: true
        contentWidth: parent.width
        contentHeight: newMessageText.contentHeight + page.margin
        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds
        visible: convoAccepted

        onHeightChanged: newMessageText.ensureVisible(newMessageText.cursorRectangle)

        onContentYChanged: {
            if (contentYUpdating)
                return

            contentYUpdating = true
            newMessageText.ensureVisible(newMessageText.cursorRectangle)
            contentYUpdating = false
        }

        // After dragging the cursor may have moved outside the visible flick area.
        onDragEnded: newMessageText.resetCursorPosition()

        // HACK: to keep margin between the top of the Android keyboard and the bottom
        // of the text input we add extra padding.
        SkyFormattedTextEdit {
            property basicprofile quoteAuthor
            property string quoteUri: ""
            property string quoteCid: ""
            property string quoteText
            property date quoteDateTime
            property bool quoteFixed: false
            property generatorview nullFeed
            property generatorview quoteFeed

            id: newMessageText
            x: page.margin
            width: parent.width - sendButton.width - 3 * page.margin
            padding: page.margin
            bottomPadding: 2 * page.margin + textInputToolbarHeight + quotedContentHeight
            parentPage: page
            parentFlick: flick
            color: guiSettings.messageNewTextColor
            placeholderText: qsTr("Say something nice")
            maxLength: page.maxMessageLength
            enableLinkShortening: false
            fontSelectorCombo: fontSelector

            onFirstPostLinkChanged: {
                if (quoteFixed)
                    return

                quoteUri = ""
                quoteFeed = nullFeed

                if (firstPostLink)
                    postUtils.getQuotePost(firstPostLink)
            }

            onCursorInFirstPostLinkChanged: {
                if (!cursorInFirstPostLink && quoteUri)
                    fixQuoteLink(true)
            }

            onFirstFeedLinkChanged: {
                quoteFeed = nullFeed

                if (firstPostLink)
                    return

                if (firstFeedLink)
                    postUtils.getQuoteFeed(firstFeedLink)
            }

            function getQuoteUri() {
                if (quoteUri)
                    return quoteUri

                return quoteFeed.uri
            }

            function getQuoteCid() {
                if (quoteCid)
                    return quoteCid

                return quoteFeed.cid
            }

            function fixQuoteLink(fix) {
                quoteFixed = fix

                if (!fix)
                    quoteUri = ""
            }

            function reset() {
                clear()
                fixQuoteLink(false)
            }
        }
    }

    SvgTransparentButton {
        id: sendButton
        anchors.right: parent.right
        anchors.rightMargin: page.margin
        y: flick.y + flick.height + 5
        svg: SvgFilled.send
        accessibleName: qsTr("send message")
        enabled: !page.isSending && newMessageText.graphemeLength > 0 && newMessageText.graphemeLength <= page.maxMessageLength
        visible: convoAccepted
        onClicked: sendMessage()
    }

    Rectangle {
        x: page.margin
        y: flick.y - newMessageText.padding
        z: -1
        width: newMessageText.width
        height: flick.height + newMessageText.bottomPadding
        radius: 10
        color: guiSettings.messageNewBackgroundColor
        border.width: newMessageText.activeFocus ? 1 : 0
        border.color: guiSettings.buttonColor
        visible: convoAccepted

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
            x: page.margin
            width: parent.width - 2 * page.margin
            anchors.bottom: fontSelector.visible ? fontSelector.top : parent.bottom
            author: newMessageText.quoteAuthor
            postText: newMessageText.quoteText
            postDateTime: newMessageText.quoteDateTime
            ellipsisBackgroundColor: guiSettings.postHighLightColor
            showCloseButton: newMessageText.quoteFixed
            visible: newMessageText.quoteUri

            onCloseClicked: {
                newMessageText.fixQuoteLink(false)
                newMessageText.forceActiveFocus()
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
            x: page.margin
            width: parent.width - 2 * page.margin
            anchors.bottom: fontSelector.visible ? fontSelector.top : parent.bottom
            feed: newMessageText.quoteFeed
            visible: !newMessageText.quoteFeed.isNull()
        }

        FontComboBox {
            id: fontSelector
            x: page.margin
            anchors.bottomMargin: 5
            anchors.bottom: parent.bottom
            popup.height: page.height - (page.header.y + page.header.height)
            focusPolicy: Qt.NoFocus
            visible: textInputToolbarHeight > 0
        }
        SvgTransparentButton {
            id: linkButton
            anchors.left: fontSelector.right
            anchors.leftMargin: visible ? 8 : 0
            anchors.bottomMargin: -height
            anchors.bottom: parent.bottom
            width: visible ? height: 0
            accessibleName: qsTr("embed web link")
            svg: SvgOutline.link
            visible: textInputToolbarHeight > 0 && (newMessageText.cursorInWebLink >= 0 || newMessageText.cursorInEmbeddedLink >= 0)
            onClicked: {
                if (newMessageText.cursorInWebLink >= 0)
                    page.addEmbeddedLink()
                else if (newMessageText.cursorInEmbeddedLink >= 0)
                    page.updateEmbeddedLink()
            }
        }

        TextLengthCounter {
            anchors.rightMargin: page.margin
            anchors.right: parent.right
            anchors.bottomMargin: 5
            anchors.bottom: parent.bottom
            textField: newMessageText
            visible: textInputToolbarHeight > 0
        }
    }

    BusyIndicator {
        id: busyIndicator
        anchors.centerIn: parent
        running: false
    }

    PostUtils {
        id: postUtils
        skywalker: page.skywalker // qmllint disable missing-type

        onQuotePost: (uri, cid, text, author, timestamp) => { // qmllint disable signal-handler-parameters
            if (!newMessageText.firstPostLink)
                return

            newMessageText.quoteFeed = newMessageText.nullFeed
            newMessageText.quoteUri = uri
            newMessageText.quoteCid = cid
            newMessageText.quoteText = text
            newMessageText.quoteAuthor = author
            newMessageText.quoteDateTime = timestamp

            if (!newMessageText.cursorInFirstPostLink)
                newMessageText.fixQuoteLink(true)
            else
                newMessageText.cutLinkIfJustAdded(newMessageText.firstPostLink, () => newMessageText.fixQuoteLink(true))
        }

        // Embedded feeds are not supported
        // onQuoteFeed: (feed) => {
        //     if (!newMessageText.firstFeedLink)
        //         return

        //     if (newMessageText.firstPostLink)
        //         return

        //     newMessageText.quoteFeed = feed
        // }
    }

    Utils {
        property string msgId

        id: utils
        skywalker: page.skywalker // qmllint disable missing-type

        onEmojiPicked: (emoji) => page.addReaction(msgId, emoji)

        function pickEmoji(messageId) {
            msgId = messageId
            showEmojiPicker()
        }
    }

    VirtualKeyboardHandler {
        id: keyboardHandler
    }

    function addMessage(msgText) {
        Qt.inputMethod.commit() // qmllint disable missing-property
        newMessageText.insert(newMessageText.length, msgText)
        newMessageText.cursorPosition = newMessageText.length
        newMessageText.forceActiveFocus()
    }

    function sendMessage() {
        Qt.inputMethod.commit() // qmllint disable missing-property

        if (!newMessageText.checkMisleadingEmbeddedLinks())
            return

        isSending = true
        const qUri = newMessageText.getQuoteUri()
        const qCid = newMessageText.getQuoteCid()
        let msgText = newMessageText.text
        console.debug("Send message:", convo.id, msgText)
        chat.sendMessage(convo.id, msgText, qUri, qCid, newMessageText.embeddedLinks)
    }

    function addReaction(messageId, emoji) {
        chat.addReaction(convo.id, messageId, emoji)
    }

    function deleteReaction(messageId, emoji) {
        chat.deleteReaction(convo.id, messageId, emoji)
    }

    function deleteMessage(messageId) {
        guiSettings.askYesNoQuestion(page,
            qsTr("Do you want to delete the message? The message will be deleted for you, the other participant will still see it."),
            () => chat.deleteMessage(convo.id, messageId))
    }

    function reportDirectMessage(msg) {
        const lastVisible = messagesView.getLastVisibleIndex()
        const lastOffset = messagesView.calcVisibleOffsetY(lastVisible)

        let component = guiSettings.createComponent("Report.qml")
        let form = component.createObject(page, {
                skywalker: skywalker,
                message: msg,
                convoId: convo.id,
                author: convo.getMember(msg.senderDid).basicProfile })

        form.onClosed.connect(() => {
            root.popStack()
            moveToMessageTimer.moveTo(lastVisible, lastOffset)
        })

        root.pushStack(form)
    }

    function addEmbeddedLink() {
        const webLinkIndex = newMessageText.cursorInWebLink

        if (webLinkIndex < 0)
            return

        console.debug("Web link index:", webLinkIndex, "size:", newMessageText.webLinks.length)
        const webLink = newMessageText.webLinks[webLinkIndex]
        console.debug("Web link:", newMessageText.link)
        let component = guiSettings.createComponent("EditEmbeddedLink.qml")
        let linkPage = component.createObject(page, {
                link: webLink.link,
                canAddLinkCard: false
        })
        linkPage.onAccepted.connect(() => {
                const name = linkPage.getName()
                linkPage.destroy()

                if (name.length > 0)
                    newMessageText.addEmbeddedLink(webLinkIndex, name)

                newMessageText.forceActiveFocus()
        })
        linkPage.onRejected.connect(() => {
                linkPage.destroy()
                newMessageText.forceActiveFocus()
        })
        linkPage.open()
    }

    function updateEmbeddedLink() {
        const linkIndex = newMessageText.cursorInEmbeddedLink

        if (linkIndex < 0)
            return

        console.debug("Embedded link index:", linkIndex, "size:", newMessageText.embeddedLinks.length)
        const link = newMessageText.embeddedLinks[linkIndex]
        const error = link.hasMisleadingName() ? link.getMisleadingNameError() :
                            (link.isTouchingOtherLink() ? qsTr("Connected to previous link") : "")
        console.debug("Embedded link:", link.link, "name:", link.name, "error:", error)

        let component = guiSettings.createComponent("EditEmbeddedLink.qml")
        let linkPage = component.createObject(page, {
                link: link.link,
                name: link.name,
                error: error,
                canAddLinkCard: false
        })
        linkPage.onAccepted.connect(() => {
                const name = linkPage.getName()
                linkPage.destroy()

                if (name.length <= 0)
                    newMessageText.removeEmbeddedLink(linkIndex)
                else
                    newMessageText.updateEmbeddedLink(linkIndex, name)

                newMessageText.forceActiveFocus()
        })
        linkPage.onRejected.connect(() => {
                linkPage.destroy()
                newMessageText.forceActiveFocus()
        })
        linkPage.open()
    }

    function showReactionList(msg) {
        const lastVisible = messagesView.getLastVisibleIndex()
        const lastOffset = messagesView.calcVisibleOffsetY(lastVisible)

        let component = guiSettings.createComponent("ReactionListView.qml")
        let view = component.createObject(page, { convo: page.convo, model: msg.reactions })

        view.onDeleteEmoji.connect((emoji) => {
            page.deleteReaction(msg.id, emoji)
            root.popStack()
            moveToMessageTimer.moveTo(lastVisible, lastOffset)
        })

        view.onClosed.connect(() => {
            root.popStack();
            moveToMessageTimer.moveTo(lastVisible, lastOffset)
        })

        root.pushStack(view)
    }

    function sendMessageOkHandler() {
        isSending = false
        busyIndicator.running = false
        newMessageText.reset()
        chat.updateMessages(convo.id)
    }

    function sendMessageFailedHandler(error) {
        skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
        isSending = false
        busyIndicator.running = false
    }

    function sendMessageProgressHandler() {
        busyIndicator.running = true
    }

    function deleteMessageOkHandler() {
        skywalker.showStatusMessage(qsTr("Message deleted"), QEnums.STATUS_LEVEL_INFO)
        chat.getMessages(convo.id)
    }

    function deleteMessageFailedHandler(error) {
        skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
    }

    function getMessagesOkHandler(cursor) {
        if (!cursor)
            messagesView.moveToEnd()
    }

    function getMessagesFailedHandler(error) {
        skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
    }

    function doMoveToMessage(index) {
        const firstVisibleIndex = messagesView.getFirstVisibleIndex()
        const lastVisibleIndex = messagesView.getLastVisibleIndex()
        console.debug("Move to:", index, "first:", firstVisibleIndex, "last:", lastVisibleIndex, "count:", messagesView.count)
        messagesView.positionViewAtIndex(Math.max(index, 0), ListView.End)

        return (lastVisibleIndex >= index && firstVisibleIndex <= index)
    }

    // HACK
    // When another page opens over this page, then the list position changes??
    // Immediately changing it back to the previous position does not work.
    // With a delay of 200ms it seems to work??
    Timer
    {
        property int msgIndex: 0
        property int msgOffset: 0

        id: moveToMessageTimer
        interval: 200

        onTriggered: page.moveToMessage(msgIndex, () => { messagesView.contentY -= msgOffset })

        function moveTo(index, offset) {
            msgIndex = index
            msgOffset = offset
            start()
        }
    }

    function moveToMessage(index, afterMoveCb = () => {}) {
        const destination = index >= 0 ? index : messagesView.count - 1
        messagesView.moveToIndex(destination, doMoveToMessage, afterMoveCb)
    }

    function rowsInsertedHandler(parent, start, end) {
        if (end === messagesView.count - 1)
            moveToMessage(-1)
    }

    function initHandlers() {
        messagesView.model.onRowsInserted.connect(rowsInsertedHandler)

        chat.onSendMessageOk.connect(sendMessageOkHandler)
        chat.onSendMessageFailed.connect(sendMessageFailedHandler)
        chat.onSendMessageProgress.connect(sendMessageProgressHandler)
        chat.onDeleteMessageOk.connect(deleteMessageOkHandler)
        chat.onDeleteMessageFailed.connect(deleteMessageFailedHandler)
        chat.onGetMessagesOk.connect(getMessagesOkHandler)
        chat.onGetMessagesFailed.connect(getMessagesFailedHandler)
    }

    function destroyHandlers() {
        messagesView.model.onRowsInserted.disconnect(rowsInsertedHandler)

        chat.onSendMessageOk.disconnect(sendMessageOkHandler)
        chat.onSendMessageFailed.disconnect(sendMessageFailedHandler)
        chat.onSendMessageProgress.disconnect(sendMessageProgressHandler)
        chat.onDeleteMessageOk.disconnect(deleteMessageOkHandler)
        chat.onDeleteMessageFailed.disconnect(deleteMessageFailedHandler)
        chat.onGetMessagesOk.disconnect(getMessagesOkHandler)
        chat.onGetMessagesFailed.disconnect(getMessagesFailedHandler)

        chat.removeMessageListModel(convo.id)
    }

    Component.onDestruction: {
        chat.updateRead(convo.id)
        destroyHandlers()
    }

    Component.onCompleted: {
        const inputTextHeight = newMessageText.height - newMessageText.padding - newMessageText.bottomPadding
        maxInputTextHeight = 5 * inputTextHeight
        moveToMessage(-1)
        initHandlers()
        sendButton.forceActiveFocus()
    }
}
