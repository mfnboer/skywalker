import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Page {
    required property var chat
    required property convoview convo
    property bool isSending: false
    readonly property int maxMessageLength: 1000
    readonly property int margin: 10
    property var skywalker: root.getSkywalker()

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

    ListView {
        id: messagesView
        width: parent.width
        height: parent.height - flick.height
        spacing: 0
        clip: true
        model: chat.getMessageListModel(convo.id)
        flickDeceleration: guiSettings.flickDeceleration
        ScrollIndicator.vertical: ScrollIndicator {}

        onHeightChanged: positionViewAtEnd()

        delegate: MessageViewDelegate {
            viewWidth: messagesView.width
            onDeleteMessage: (messageId) => page.deleteMessage(messageId)
        }

        FlickableRefresher {
            inProgress: chat.getMessagesInProgress
            topOvershootFun: () => chat.getMessagesNextPage(convo.id)
            bottomOvershootFun: () => chat.getMessages(convo.id)
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
    }

    Flickable {
        id: flick
        width: parent.width
        height: Math.min(newMessageRect.height, parent.height)
        anchors.bottom: parent.bottom
        clip: true
        contentWidth: parent.width
        contentHeight: newMessageRect.height
        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds
        onHeightChanged: newMessageText.ensureVisible(newMessageText.cursorRectangle)

        Rectangle {
            id: newMessageRect
            x: page.margin
            width: parent.width - sendButton.width - 3 * page.margin
            height: newMessageText.height
            color: "transparent"

            // HACK: to keep margin between the top of the Android keyboard and the bottom
            // of the text input we add extra padding.
            SkyFormattedTextEdit {
                id: newMessageText
                width: parent.width
                padding: page.margin
                bottomPadding: 2 * page.margin
                parentPage: page
                parentFlick: flick
                color: guiSettings.messageNewTextColor
                placeholderText: qsTr("Say something nice")
                maxLength: page.maxMessageLength
                // TODO: font selector?

                Rectangle {
                    z: -1
                    width: parent.width
                    height: parent.height - page.margin
                    radius: 10
                    color: guiSettings.messageNewBackgroundColor
                }
            }
        }

        SvgTransparentButton {
            id: sendButton
            anchors.right: parent.right
            anchors.rightMargin: page.margin
            y: parent.height - 5 - page.margin
            svg: svgFilled.send
            accessibleName: qsTr("send message")
            enabled: !page.isSending && newMessageText.graphemeLength > 0 && newMessageText.graphemeLength <= page.maxMessageLength
            onClicked: sendMessage()
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

            if (keyboardY > 0) {
                if (!keyboardVisible) {
                    const keyboardHeight = page.height - keyboardY
                    messagesView.y = Math.min(keyboardHeight, Math.max(messagesView.height - messagesView.contentHeight, 0))
                    header.y = keyboardHeight
                    keyboardVisible = true
                }
            }
            else {
                messagesView.y = 0
                header.y = 0
                keyboardVisible = false
            }
        }
    }

    function sendMessage() {
        sendButton.forceActiveFocus() // take focus from the text input to get complete text
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

    function sendMessageOkHandler() {
        skywalker.showStatusMessage(qsTr("Message sent"), QEnums.STATUS_LEVEL_INFO)
        isSending = false
        busyIndicator.running = false
        newMessageText.clear()
        chat.updateMessages(convo.id)
        sendButton.forceActiveFocus()
    }

    function sendMessageFailedHandler(error) {
        skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
        isSending = false
        busyIndicator.running = false
    }

    function sendMessageProgressHandler(msg) {
        busyIndicator.running = true
        skywalker.showStatusMessage(msg, QEnums.STATUS_LEVEL_INFO)
    }

    function deleteMessageOkHandler() {
        skywalker.showStatusMessage(qsTr("Message deleted"), QEnums.STATUS_LEVEL_INFO)
        chat.getMessages(convo.id)
    }

    function deleteMessageFailedHandler(error) {
        skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
    }

    function getMessagesFailedHandler(error) {
        skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
    }

    function rowsInsertedHandler(parent, start, end) {
        messagesView.positionViewAtEnd()
    }

    function initHandlers() {
        messagesView.model.onRowsInserted.connect(rowsInsertedHandler)

        chat.onSendMessageOk.connect(sendMessageOkHandler)
        chat.onSendMessageFailed.connect(sendMessageFailedHandler)
        chat.onSendMessageProgress.connect(sendMessageProgressHandler)
        chat.onDeleteMessageOk.connect(deleteMessageOkHandler)
        chat.onDeleteMessageFailed.connect(deleteMessageFailedHandler)
        chat.onGetMessagesFailed.connect(getMessagesFailedHandler)
    }

    function destroyHandlers() {
        messagesView.model.onRowsInserted.disconnect(rowsInsertedHandler)

        chat.onSendMessageOk.disconnect(sendMessageOkHandler)
        chat.onSendMessageFailed.disconnect(sendMessageFailedHandler)
        chat.onSendMessageProgress.disconnect(sendMessageProgressHandler)
        chat.onDeleteMessageOk.disconnect(deleteMessageOkHandler)
        chat.onDeleteMessageFailed.disconnect(deleteMessageFailedHandler)
        chat.onGetMessagesFailed.disconnect(getMessagesFailedHandler)

        chat.removeMessageListModel(convo.id)
    }

    Component.onDestruction: {
        chat.updateRead(convo.id)
        destroyHandlers()
    }

    Component.onCompleted: {
        messagesView.positionViewAtEnd()
        initHandlers()
        sendButton.forceActiveFocus()
    }
}
