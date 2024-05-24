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
        height: 10
        color: "transparent"
    }

    ListView {
        id: messagesView
        width: parent.width
        anchors.top: parent.top
        height: parent.height - flick.height
        spacing: 0
        clip: true
        model: chat.getMessageListModel(convo.id)
        flickDeceleration: guiSettings.flickDeceleration
        ScrollIndicator.vertical: ScrollIndicator {}

        onHeightChanged: positionViewAtEnd()

        delegate: MessageViewDelegate {
            viewWidth: messagesView.width
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
            radius: 10
            color: guiSettings.messageNewBackgroundColor

            SkyFormattedTextEdit {
                id: newMessageText
                width: parent.width
                padding: page.margin
                parentPage: page
                parentFlick: flick
                color: guiSettings.messageNewTextColor
                placeholderText: qsTr("Say something nice")
                maxLength: page.maxMessageLength
                // TODO: font selector?
            }
        }

        SvgTransparentButton {
            id: sendButton
            anchors.right: parent.right
            anchors.rightMargin: page.margin
            y: parent.height - 5
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

    VirtualKeyboardPageResizer {
        id: virtualKeyboardPageResizer
    }

    GuiSettings {
        id: guiSettings
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
        chat.onGetMessagesFailed.connect(getMessagesFailedHandler)
    }

    function destroyHandlers() {
        messagesView.model.onRowsInserted.disconnect(rowsInsertedHandler)

        chat.onSendMessageOk.disconnect(sendMessageOkHandler)
        chat.onSendMessageFailed.disconnect(sendMessageFailedHandler)
        chat.onSendMessageProgress.disconnect(sendMessageProgressHandler)
        chat.onGetMessagesFailed.disconnect(getMessagesFailedHandler)

        chat.removeMessageListModel(convo.id)
    }

    Component.onDestruction: {
        const lastMessageId = messagesView.model.getLastMessageId()
        chat.updateRead(convo.id, lastMessageId)
        destroyHandlers()
    }

    Component.onCompleted: {
        virtualKeyboardPageResizer.fullPageHeight = parent.height
        messagesView.positionViewAtEnd()
        initHandlers()
        sendButton.forceActiveFocus()
    }
}
