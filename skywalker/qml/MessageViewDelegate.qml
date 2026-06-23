import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Rectangle {
    required property int viewWidth
    required property convoview convo
    required property messageview message
    required property bool senderIsUser
    required property bool sameSenderAsNext
    required property bool sameSenderAsPrevious
    required property bool sameTimeAsNext
    required property bool sameDateAsPrevious
    required property bool endOfList
    property Skywalker skywalker: root.getSkywalker()
    property basicprofile author: senderIsUser ? skywalker.user : convo.getMember(message.senderDid).basicProfile
    readonly property bool isGroupConvo: convo.kind === QEnums.CONVO_KIND_GROUP
    readonly property int avatarWidth: 30
    readonly property int messageIndent: isGroupConvo ? avatarWidth + 10 : 0
    property int maxTextWidth: viewWidth - 80
    property int maxTextLines: 1000
    readonly property int margin: 10

    signal replyToMessage(messageview message, basicprofile author)
    signal deleteMessage(string messageId)
    signal reportMessage(messageview message)
    signal addEmoji(string messageId, string emoji)
    signal pickEmoji(string messageId)
    signal showReactions(messageview message)
    signal replyClicked(string messageId)

    id: view
    width: viewWidth
    color: guiSettings.backgroundColor
    height: endMarker.y

    AccessibleText {
        id: conversationStartText
        width: viewWidth
        topPadding: 10
        horizontalAlignment: Text.AlignHCenter
        font.italic: true
        text: qsTr("Start of conversation")
        visible: endOfList
    }

    AccessibleText {
        id: messageDateText
        y: conversationStartText.y + (conversationStartText.visible ? conversationStartText.height : 0)
        width: viewWidth
        height: visible ? contentHeight + padding : 0
        padding: 10
        horizontalAlignment: Text.AlignHCenter
        font.bold: true
        text: getMessageDateIndication()
        visible: !sameDateAsPrevious
    }

    Loader {
        x: margin
        anchors.bottom: messageRect.bottom
        width: avatarWidth
        height: width
        active: isGroupConvo && !senderIsUser && !sameSenderAsNext && !message.isSystemMessage

        sourceComponent: Avatar {
            author: view.author

            onClicked: {
                if (!view.author.isNull())
                    skywalker.getDetailedProfile(view.author.did)
            }
        }
    }

    Loader {
        id: senderName
        x: margin + messageIndent
        anchors.top: messageDateText.bottom
        active: isGroupConvo && !senderIsUser && !sameSenderAsPrevious && !message.isSystemMessage

        sourceComponent: SkyCleanedTextLine {
            topPadding: 10
            width: maxTextWidth
            elide: Text.ElideRight
            color: Material.color(Material.Grey)
            font.bold: true
            font.pointSize: guiSettings.scaledFont(7/8)
            plainText: view.author.name
        }
    }

    Rectangle {
        readonly property color backgroundColor: senderIsUser ?
                                                     guiSettings.messageUserBackgroundColor :
                                                     guiSettings.messageOtherBackgroundColor


        id: messageRect
        x: senderIsUser ? viewWidth - margin - width : margin + messageIndent
        anchors.top: senderName.bottom
        anchors.topMargin: 5
        width: Math.max(messageText.width, embed.item ? embed.item.width + 20 : 0, replyToLoader.item ? replyToLoader.item.width + 20 : 0)
        height: (replyToLoader.item ? replyToLoader.item.height + 10 : 0) + messageText.height + (embed.item ? embed.item.height + 10 : 0)
        radius: guiSettings.radius
        color: backgroundColor
        visible: !message.isSystemMessage

        Loader {
            id: replyToLoader
            x: 10
            y: 5
            active: message.isReply

            sourceComponent: MessageViewReply {
                maxWidth: maxTextWidth - 20
                minWidth: messageText.width
                convo: view.convo
                replyTo: message.replyTo

                onClicked: view.replyClicked(message.replyTo.id)
            }
        }

        MessageText {
            id: messageText
            y: replyToLoader.item ? replyToLoader.item.height + replyToLoader.y : 0
            maxWidth: maxTextWidth - messageIndent
            convo: view.convo
            message: view.message
            author: view.author
            topPadding: 5
            bottomPadding: 5
            initialShowMaxLineCount: Math.min(maxTextLines, 25)
            maximumLineCount: maxTextLines
            ellipsisBackgroundColor: messageRect.color
            color: senderIsUser ? guiSettings.messageUserTextColor : guiSettings.messageOtherTextColor

            onLongPress: moreMenu.open()
        }

        Rectangle {
            x: senderIsUser ? parent.width - width : 0
            y: parent.height - height
            width: parent.radius
            height: width
            color: parent.color
            visible: !sameSenderAsNext
        }

        Loader {
            id: embed
            x: 10
            anchors.top: messageText.bottom
            active: !message.embed.isNull()

            sourceComponent: RecordView {
                width: maxTextWidth - 20 - messageIndent
                record: message.embed
            }
        }

        SkyMouseArea {
            anchors.fill: parent
            z: -2
            onPressAndHold: moreMenu.open()
        }

        SkyMenu {
            id: moreMenu
            modal: false

            SkyMenuButton {
                text: qsTr("Reply to")
                svg: SvgOutline.reply
                popup: moreMenu
                onClicked: replyToMessage(message, author)
            }
            SkyMenuButton {
                text: qsTr("Translate")
                svg: SvgOutline.googleTranslate
                popup: moreMenu
                onClicked: root.translateText(message.text)
            }
            SkyMenuButton {
                text: qsTr("Copy message")
                svg: SvgOutline.copy
                popup: moreMenu
                onClicked: skywalker.getShareUtils().copyToClipboard(message.text)
            }
            SkyMenuButton {
                text: qsTr("Delete")
                svg: SvgOutline.delete
                popup: moreMenu
                onClicked: deleteMessage(message.id)
            }
            SkyMenuButton {
                text: qsTr("Report message")
                svg: SvgOutline.report
                popup: moreMenu
                visible: !senderIsUser
                onClicked: reportMessage(message)
            }
        }

        ReactionsMenu {
            parent: Overlay.overlay
            x: messageRect.x + moreMenu.x + moreMenu.width + 10
            y: view.y - view.ListView.view.contentY + view.ListView.view.parent.y + messageRect.y + moreMenu.y
            z: 1
            color: moreMenu.background.color
            visible: moreMenu.opened && !senderIsUser

            onEmojiSelected: (emoji) => {
                moreMenu.close()
                addEmoji(message.id, emoji)
            }

            onMoreEmoji: {
                moreMenu.close()
                pickEmoji(message.id)
            }
        }

        Loader {
            id: blinkLoader
            active: false

            sourceComponent: SequentialAnimation {
                loops: 1
                running: true

                ColorAnimation { target: messageRect; property: "color"; from: messageRect.backgroundColor; to: guiSettings.accentColor; duration: 500 }
                ColorAnimation { target: messageRect; property: "color"; from: guiSettings.accentColor; to: messageRect.backgroundColor; duration: 500 }

                onStopped: blinkLoader.active = false
            }
        }
    }

    Loader {
        id: reactionsLoader
        active: message.reactions.length > 0
        anchors.top: messageRect.bottom
        anchors.topMargin: -3

        sourceComponent: Rectangle {
            id: reactionsRect
            x: messageRect.x + (senderIsUser ? messageRect.width - 5 - width : 5);
            width: reactionsRow.width + 6
            height: reactionsRow.height
            radius: height / 2
            color: guiSettings.backgroundColor

            Row {
                property var uniqueReactions: message.getUniqueReactions(5)

                id: reactionsRow
                anchors.centerIn: parent

                Repeater {
                    model: parent.uniqueReactions

                    AccessibleText {
                        font.family: UnicodeFonts.getEmojiFontFamily()
                        text: modelData.emoji
                    }
                }

                AccessibleText {
                    text: ` ${message.reactions.length} `
                    visible: parent.uniqueReactions.length < message.reactions.length
                }
            }

            SkyMouseArea {
                anchors.fill: parent
                onClicked: showReactions(message)
            }
        }
    }

    Loader {
        id: systemMessage
        x: margin
        anchors.top: messageRect.top
        anchors.topMargin: 5
        active: message.isSystemMessage

        sourceComponent: Column {
            width: view.width - 2 * margin

            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 10

                SkySvg {
                    id: systemIcon
                    width: guiSettings.appFontHeight * 7/8
                    height: width
                    color: guiSettings.messageTimeColor
                    svg: SvgOutline.chat
                }

                AccessibleText {
                    color: guiSettings.messageTimeColor
                    font.pointSize: guiSettings.scaledFont(7/8)
                    text: Qt.locale().toString(message.sentAt, Qt.locale().timeFormat(Locale.ShortFormat))
                }
            }

            AccessibleText {
                id: systemText
                horizontalAlignment: Text.AlignHCenter
                width: parent.width
                elide: Text.ElideRight
                wrapMode: Text.Wrap
            }

            function setMessage(icon, text) {
                systemIcon.svg = icon
                systemText.text = text
            }
        }

        function setMessage(icon, text) {
            console.debug("SYSTEM:", text)

            if (item)
                item.setMessage(icon, text)
        }
    }

    AccessibleText {
        id: messageTimeText
        anchors.left: messageRect.left
        anchors.top: reactionsLoader.item ? reactionsLoader.bottom : messageRect.bottom
        anchors.topMargin: visible ? 5 : 0
        width: messageRect.width
        height: visible ? contentHeight : 0
        horizontalAlignment: senderIsUser ? Text.AlignRight : Text.AlignLeft
        color: guiSettings.messageTimeColor
        font.pointSize: guiSettings.scaledFont(6/8)
        text: Qt.locale().toString(message.sentAt, Qt.locale().timeFormat(Locale.ShortFormat))
        visible: (!sameSenderAsNext || !sameTimeAsNext) && !message.isSystemMessage
    }

    Item {
        id: endMarker
        anchors.top: messageTimeText.visible ?
                messageTimeText.bottom :
                (reactionsLoader.item ?
                     reactionsLoader.bottom :
                     messageRect.visible ? messageRect.bottom : systemMessage.bottom)
    }

    Loader {
        id: profileLoader
        active: author.isNull()

        sourceComponent: ProfileUtils {
            skywalker: view.skywalker

            onBasicProfileOk: (profile) => view.author = profile
        }

        onStatusChanged: {
            if (status == Loader.Ready)
                item.getBasicProfile(message.senderDid)
        }
    }

    Loader {
        id: systemMessageUtils
        active: message.isSystemMessage

        sourceComponent: SystemMessageUtils {
            onMessage: (icon, text) => systemMessage.setMessage(icon, text)
        }

        function getMessage(msg) {
            if (item)
                item.getMessage(msg)
        }
    }

    function getMessageDateIndication() {
        if (guiSettings.isToday(message.sentAt))
            return qsTr("Today")
        else if (guiSettings.isYesterday(message.sentAt))
            return qsTr("Yesterday")
        else
            return Qt.locale().toString(message.sentAt, Qt.locale().dateFormat(Locale.ShortFormat))
    }

    function getMessageFontSize() {
        if (!root.getSkywalker().getUserSettings().giantEmojis)
            return guiSettings.scaledFont(1)

        return onlyEmojisMessage() ?
                    guiSettings.scaledFont(UnicodeFonts.graphemeLength(message.text) === 1 ? 9 : 3) :
                    guiSettings.scaledFont(1)
    }

    function onlyEmojisMessage() {
        if (!message.text)
            return false

        if (UnicodeFonts.graphemeLength(message.text) > 5)
            return false

        return UnicodeFonts.onlyEmojis(message.text)
    }

    function flash() {
        blinkLoader.active = true
    }

    Component.onCompleted: {
        if (message.isSystemMessage)
            systemMessageUtils.getMessage(message)
    }
}
