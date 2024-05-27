import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Rectangle {
    required property int viewWidth
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

    id: view
    width: viewWidth
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
        height: visible ? implicitHeight : 0
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
        width: messageText.width
        height: messageText.height
        radius: 10
        color: senderIsUser ? guiSettings.messageUserBackgroundColor : guiSettings.messageOtherBackgroundColor

        // TODO: show ava/name for multi-person chat
        SkyCleanedText {
            readonly property alias textWidth: textMetrics.boundingRect.width
            readonly property string deletedText: qsTr("message deleted")

            id: messageText
            width: Math.min(textWidth + 5 + leftPadding + rightPadding, maxTextWidth)
            leftPadding: 10
            rightPadding: 10
            topPadding: 5
            bottomPadding: 5
            wrapMode: Text.Wrap
            intialShowMaxLineCount: Math.min(maxTextLines, 25)
            maximumLineCount: maxTextLines
            ellipsisBackgroundColor: messageRect.color
            elide: Text.ElideRight
            textFormat: Text.RichText
            font.italic: message.deleted
            font.pointSize: getMessageFontSize()
            color: senderIsUser ? guiSettings.messageUserTextColor : guiSettings.messageOtherTextColor
            plainText: !message.deleted ? message.formattedText : deletedText

            onLinkActivated: (link) => root.openLink(link)

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

                    MenuItemSvg { svg: svgOutline.googleTranslate }
                }
                AccessibleMenuItem {
                    text: qsTr("Copy message")
                    onTriggered: skywalker.copyToClipboard(message.text)

                    MenuItemSvg { svg: svgOutline.copy }
                }
                AccessibleMenuItem {
                    text: qsTr("Delete")
                    onTriggered: deleteMessage(message.id)

                    MenuItemSvg { svg: svgOutline.delete }
                }
                AccessibleMenuItem {
                    text: qsTr("Report message")
                    visible: !senderIsUser
                    onTriggered: reportMessage(message)

                    MenuItemSvg { svg: svgOutline.report }
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
        height: visible ? implicitHeight : 0
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
