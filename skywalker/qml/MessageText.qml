import QtQuick
import skywalker

SkyCleanedText {
    property int maxWidth: 100
    required property convoview convo
    required property messageview message
    required property basicprofile author

    // Note: textMetrics.boundingRect.width gave a wrong width when the text
    // ended with an emoji VARIATION SELECTOR-16 (U+FE0F)
    readonly property alias textWidth: textMetrics.advanceWidth
    readonly property string deletedText: qsTr("message deleted")

    signal longPress

    id: messageText
    width: Math.min(textWidth + 5 + leftPadding + rightPadding, maxWidth)
    leftPadding: 10
    rightPadding: 10
    wrapMode: Text.Wrap
    elide: Text.ElideRight
    textFormat: message.textMetaInfo.isSimpleText() ? Text.StyledText : Text.RichText
    font.italic: message.deleted
    plainText: getMessageDisplayText()

    function getMessageDisplayText() {
        if (message.deleted)
            return deletedText

        // Something fishy with this profile, make links not clickable
        if (author.hasInvalidHandle())
            return message.text

        return message.formattedText
    }

    TextMetrics {
        id: textMetrics
        font: messageText.font
        text: !message.deleted ? message.text : message.deletedText
    }

    LinkCatcher {
        z: parent.z - 1
        containingText: message.text
        author: messageText.author
        onLongPress: messageText.longPress()
    }
}
