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
    property int maxTextWidth: viewWidth - 80
    property int maxTextLines: 1000
    readonly property int margin: 10

    signal deleteMessage(string messageId)
    signal reportMessage(messageview message)
    signal openingEmbed

    id: view
    width: viewWidth
    color: guiSettings.backgroundColor
    height: messageTimeText.y + messageTimeText.height

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

    Rectangle {
        id: messageRect
        x: senderIsUser ? viewWidth - margin - width : margin
        anchors.top: messageDateText.bottom;
        anchors.topMargin: 5
        width: Math.max(messageText.width, embed.visible ? embed.width + 20 : 0)
        height: messageText.height + (embed.visible ? embed.height + 10 : 0)
        radius: 10
        color: senderIsUser ? guiSettings.messageUserBackgroundColor :
                              guiSettings.messageOtherBackgroundColor

        // TODO: show ava/name for multi-person chat
        SkyCleanedText {
            // Note: textMetrics.boundingRect.width gave a wrong width when the text
            // ended with an emoji VARIATION SELECTOR-16 (U+FE0F)
            readonly property alias textWidth: textMetrics.advanceWidth
            readonly property string deletedText: qsTr("message deleted")

            id: messageText
            width: Math.min(textWidth + 5 + leftPadding + rightPadding, maxTextWidth)
            leftPadding: 10
            rightPadding: 10
            topPadding: 5
            bottomPadding: 5
            wrapMode: Text.Wrap
            initialShowMaxLineCount: Math.min(maxTextLines, 25)
            maximumLineCount: maxTextLines
            ellipsisBackgroundColor: messageRect.color
            elide: Text.ElideRight
            textFormat: Text.RichText
            font.italic: message.deleted
            font.pointSize: getMessageFontSize()
            color: senderIsUser ? guiSettings.messageUserTextColor : guiSettings.messageOtherTextColor
            plainText: getMessageDisplayText()

            onLinkActivated: (link) => root.openLink(link)

            function getMessageDisplayText() {
                if (message.deleted)
                    return deletedText

                // Something fishy with this profile, make links not clickable
                if (convo.getMember(message.senderDid).basicProfile.hasInvalidHandle())
                    return message.text

                return message.formattedText
            }

            TextMetrics {
                id: textMetrics
                font: messageText.font
                text: !message.deleted ? message.text : messageText.deletedText
            }
        }

        Rectangle {
            x: senderIsUser ? parent.width - width : 0
            y: parent.height - height
            width: parent.radius
            height: width
            color: parent.color
            visible: !sameSenderAsNext
        }

        RecordView {
            id: embed
            x: 10
            width: maxTextWidth - 20
            anchors.top: messageText.bottom
            backgroundColor: guiSettings.backgroundColor
            record: message.embed
            visible: !message.embed.isNull()

            onOpening: openingEmbed()
        }

        MouseArea {
            anchors.fill: parent
            z: -2
            onPressAndHold: moreMenu.open()

            Menu {
                id: moreMenu
                modal: true

                onAboutToShow: root.enablePopupShield(true)
                onAboutToHide: root.enablePopupShield(false)

                CloseMenuItem {
                    text: qsTr("<b>Message</b>")
                    Accessible.name: qsTr("close messages menu")
                }
                AccessibleMenuItem {
                    text: qsTr("Translate")
                    onTriggered: root.translateText(message.text)

                    MenuItemSvg { svg: SvgOutline.googleTranslate }
                }
                AccessibleMenuItem {
                    text: qsTr("Copy message")
                    onTriggered: skywalker.copyToClipboard(message.text)

                    MenuItemSvg { svg: SvgOutline.copy }
                }
                AccessibleMenuItem {
                    text: qsTr("Delete")
                    onTriggered: deleteMessage(message.id)

                    MenuItemSvg { svg: SvgOutline.delete }
                }
                AccessibleMenuItem {
                    text: qsTr("Report message")
                    visible: !senderIsUser
                    onTriggered: reportMessage(message)

                    MenuItemSvg { svg: SvgOutline.report }
                }
            }
        }
    }

    AccessibleText {
        id: messageTimeText
        anchors.left: messageRect.left
        anchors.top: messageRect.bottom
        anchors.topMargin: visible ? 5 : 0
        width: messageRect.width
        height: visible ? contentHeight : 0
        horizontalAlignment: senderIsUser ? Text.AlignRight : Text.AlignLeft
        color: guiSettings.messageTimeColor
        font.pointSize: guiSettings.scaledFont(6/8)
        text: Qt.locale().toString(message.sentAt, Qt.locale().timeFormat(Locale.ShortFormat))
        visible: !sameSenderAsNext || !sameTimeAsNext
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
                    guiSettings.scaledFont(unicodeFonts.graphemeLength(message.text) === 1 ? 9 : 3) :
                    guiSettings.scaledFont(1)
    }

    function onlyEmojisMessage() {
        if (!message.text)
            return false

        if (unicodeFonts.graphemeLength(message.text) > 5)
            return false

        return unicodeFonts.onlyEmojis(message.text)
    }

    UnicodeFonts {
        id: unicodeFonts
    }

    GuiSettings {
        id: guiSettings
    }
}
