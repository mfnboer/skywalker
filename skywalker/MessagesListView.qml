import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Page {
    required property var chat
    required property convoview convo
    property bool isSending: false
    property int maxInputTextHeight
    readonly property int maxMessageLength: 1000
    readonly property int margin: 10
    property var skywalker: root.getSkywalker()
    property int textInputToolbarHeight: 0

    signal closed

    id: page

    header: MessagesListHeader {
        convo: page.convo
        onBack: page.closed()
    }

    // Needed for SkyFormattedTextEdit
    footer: Rectangle {
        height: 0
        color: "transparent"
    }

    onHeightChanged: {
        // When the Android keyboard is visible the height sometimes shrinks to
        // the size of the visible window. I don't why that happens. Would be nice
        // if could do so from the start. But shrinking the height on the keyboard event
        // cause the whole screen to move up into the void????
        page.header.y = 0
    }

    ListView {
        id: messagesView
        width: parent.width
        height: parent.height - y - flick.height - newMessageText.padding - newMessageText.bottomPadding
        spacing: 0
        clip: true
        model: chat.getMessageListModel(convo.id)
        boundsMovement: Flickable.StopAtBounds
        flickDeceleration: guiSettings.flickDeceleration
        ScrollIndicator.vertical: ScrollIndicator {}

        onHeightChanged: moveToEnd()

        delegate: MessageViewDelegate {
            viewWidth: messagesView.width
            convo: page.convo
            onDeleteMessage: (messageId) => page.deleteMessage(messageId)
            onReportMessage: (msg) => page.reportDirectMessage(msg)
        }

        FlickableRefresher {
            inProgress: chat.getMessagesInProgress
            topOvershootFun: () => chat.getMessagesNextPage(convo.id)
        }

        EmptyListIndication {
            y: parent.headerItem ? parent.headerItem.height : 0
            svg: svgOutline.noDirectMessages
            text: qsTr("None")
            list: messagesView
        }

        BusyIndicator {
            anchors.centerIn: parent
            running: chat.getMessagesInProgress
        }

        function moveToEnd() {
            positionViewAtEnd()

            // positionViewAtEnd not always gets completely to the end...
            // This seems to fix it...
            contentY = Math.max(originY, originY + contentHeight - height)
        }
    }

    Flickable {
        property bool contentYUpdating: false

        id: flick
        width: parent.width
        height: Math.min(newMessageText.height, maxInputTextHeight) - newMessageText.padding - newMessageText.bottomPadding
        anchors.bottom: parent.bottom
        anchors.bottomMargin: newMessageText.bottomPadding
        clip: true
        contentWidth: parent.width
        contentHeight: newMessageText.contentHeight + page.margin
        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds

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
            id: newMessageText
            x: page.margin
            width: parent.width - sendButton.width - 3 * page.margin
            padding: page.margin
            bottomPadding: 2 * page.margin + textInputToolbarHeight
            parentPage: page
            parentFlick: flick
            color: guiSettings.messageNewTextColor
            placeholderText: qsTr("Say something nice")
            maxLength: page.maxMessageLength
            enableLinkShortening: false
            fontSelectorCombo: fontSelector
        }
    }

    SvgTransparentButton {
        id: sendButton
        anchors.right: parent.right
        anchors.rightMargin: page.margin
        y: flick.y + flick.height + 5
        svg: svgFilled.send
        accessibleName: qsTr("send message")
        enabled: !page.isSending && newMessageText.graphemeLength > 0 && newMessageText.graphemeLength <= page.maxMessageLength
        onClicked: sendMessage()
    }

    Rectangle {
        x: page.margin
        y: flick.y - newMessageText.padding
        z: -1
        width: newMessageText.width
        height: flick.height + 2 * newMessageText.padding + textInputToolbarHeight
        radius: 10
        color: guiSettings.messageNewBackgroundColor

        FontComboBox {
            id: fontSelector
            x: page.margin
            anchors.bottom: parent.bottom
            popup.height: page.height - (page.header.y + page.header.height)
            focusPolicy: Qt.NoFocus
            visible: textInputToolbarHeight > 0
        }
    }

    BusyIndicator {
        id: busyIndicator
        anchors.centerIn: parent
        running: false
    }

    GuiSettings {
        id: guiSettings
    }

    Connections {
        property bool keyboardVisible: false

        target: Qt.inputMethod

        function onKeyboardRectangleChanged() {
            const keyboardY = Qt.inputMethod.keyboardRectangle.y  / Screen.devicePixelRatio

            // NOTE: This function seems to be called many times when the keyboard
            // pops up, with increasing Y values. The first Y value seems to be the
            // position of the top of the keyboard, not sure where the others calls
            // come from.
            if (keyboardY > 0) {
                if (!keyboardVisible) {
                    const keyboardHeight = page.height - keyboardY

                    // Shrink empty space between message list and text input. The
                    // text input shifted up to make place for the keyboard.
                    messagesView.y = Math.min(keyboardHeight, Math.max(messagesView.height - messagesView.contentHeight, 0))

                    // Qt seems to scroll the whole window up!. Pull the header
                    // down to make it visible
                    header.y = keyboardHeight

                    newMessageText.ensureVisible(newMessageText.cursorRectangle)
                    textInputToolbarHeight = 24
                    keyboardVisible = true
                }
            }
            else {
                messagesView.y = 0
                header.y = 0
                textInputToolbarHeight = 0
                keyboardVisible = false
            }
        }
    }

    function sendMessage() {
        Qt.inputMethod.commit()
        isSending = true
        const qUri = ""
        const qCid = ""
        let msgText = newMessageText.text
        console.debug("Send message:", convo.id, msgText)
        chat.sendMessage(convo.id, msgText, qUri, qCid)
    }

    function deleteMessage(messageId) {
        guiSettings.askYesNoQuestion(page,
            qsTr("Do you want to delete the message? The message will be deleted for you, the other participant will still see it."),
            () => chat.deleteMessage(convo.id, messageId))
    }

    function reportDirectMessage(msg) {
        root.reportDirectMessage(msg, convo.id, convo.getMember(msg.senderDid).basicProfile)
    }

    function sendMessageOkHandler() {
        isSending = false
        busyIndicator.running = false
        newMessageText.clear()
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

    function rowsInsertedHandler(parent, start, end) {
        if (end === messagesView.count - 1)
            messagesView.moveToEnd()
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
        maxInputTextHeight = 5 * inputTextHeight + newMessageText.padding + newMessageText.bottomPadding
        messagesView.moveToEnd()
        initHandlers()
        sendButton.forceActiveFocus()
    }
}
