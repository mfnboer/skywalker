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
    required property bool sameDateAsPrevious;
    required property bool endOfList
    property int maxTextWidth: viewWidth - 80
    property int maxTextLines: 1000
    readonly property int margin: 10

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
        text: Qt.locale().toString(message.sentAt, Qt.locale().dateFormat(Locale.ShortFormat))
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
            readonly property string deletedText: qsTr("deleted")

            id: messageText
            width: Math.min(textWidth + 5 + padding * 2, maxTextWidth)
            padding: 10
            wrapMode: Text.Wrap
            intialShowMaxLineCount: Math.min(maxTextLines, 25)
            maximumLineCount: maxTextLines
            ellipsisBackgroundColor: messageRect.color
            elide: Text.ElideRight
            textFormat: Text.RichText
            font.italic: message.deleted
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

    GuiSettings {
        id: guiSettings
    }
}
